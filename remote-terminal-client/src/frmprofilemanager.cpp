// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "frmprofilemanager.h"

frmProfileManager::frmProfileManager(ProfileManager& profileManager, QWidget *parent) : QWidget(parent), ui(new Ui::frmProfileManager)
{
    ui->setupUi(this); // Set up widget ui
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint); // Set the widget to have no form buttons
    this->profileManager = &profileManager;

    QString ipOctet = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    ui->txtProfilePort->setValidator(new QIntValidator(1, 65535, this));
    ui->txtProfilePassword->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
    ui->txtProfilePassword->setEchoMode(QLineEdit::Password);
    ui->txtProfileIPAddress->setValidator(new QRegExpValidator(QRegExp("^" + ipOctet + "\\." + ipOctet + "\\." + ipOctet + "\\." + ipOctet + "$"), this));

    connect(ui->btnNewProfile, SIGNAL(clicked(bool)),this,SLOT(AddNewProfile()));
    connect(ui->btnDeleteProfile, SIGNAL(clicked(bool)),this,SLOT(DeleteProfile()));
    connect(ui->btnSaveProfile, SIGNAL(clicked(bool)),this,SLOT(SaveProfile()));
    connect(ui->lstStoredProfiles,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(OnProfileItemClick(QListWidgetItem*)));
    connect(ui->lstStoredProfiles,SIGNAL(currentRowChanged(int)),this,SLOT(OnProfileItemChange(int)));
    connect(ui->chkShowPassword, SIGNAL(stateChanged(int)), this, SLOT(CheckShowPassword()));
    this->selectedProfileIndex = -1;

    LoadProfiles();

    if(ui->lstStoredProfiles->count() > 0)
    {
        ui->lstStoredProfiles->item(0)->setSelected(true);
        LoadProfile(0);
    }
}

void frmProfileManager::CheckShowPassword()
{
    ui->txtProfilePassword->setEchoMode(ui->chkShowPassword->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
}

void frmProfileManager::LoadProfiles()
{
    Profile *currentProfile;
    ui->lstStoredProfiles->clear();
    this->storedProfiles = this->profileManager->GetProfiles();

    for(int i = 0; i < this->storedProfiles.count(); i++)
    {
        currentProfile = this->storedProfiles.at(i);
        ui->lstStoredProfiles->addItem(currentProfile->GetProfileAsDefault() ? currentProfile->GetProfileName() + " (Default)"
                                                                             : currentProfile->GetProfileName());
    }
}

void frmProfileManager::AddNewProfile()
{
    Profile *newProfile = new Profile("New Profile " + QString::number(ui->lstStoredProfiles->count() + 1),"","",0,false);
    this->storedProfiles.append(newProfile);
    this->profileManager->SaveProfiles(this->storedProfiles);
    LoadProfiles();

    if(this->selectedProfileIndex != -1)
    {
        ui->lstStoredProfiles->item(this->selectedProfileIndex)->setSelected(true);
        LoadProfile(this->selectedProfileIndex);
    }
}

void frmProfileManager::DeleteProfile()
{
    this->storedProfiles.remove(this->selectedProfileIndex);
    this->profileManager->SaveProfiles(this->storedProfiles);
    DisableProfileFields();
    LoadProfiles();

    if(ui->lstStoredProfiles->count() > 0)
    {
        this->selectedProfileIndex = this->selectedProfileIndex != 0 ? this->selectedProfileIndex - 1 : 0;
        ui->lstStoredProfiles->item(this->selectedProfileIndex)->setSelected(true);
        LoadProfile(this->selectedProfileIndex);
    }
}

void frmProfileManager::EnableProfileFields()
{
    ui->txtProfileName->setEnabled(true);
    ui->txtProfileIPAddress->setEnabled(true);
    ui->txtProfilePassword->setEnabled(true);
    ui->txtProfilePort->setEnabled(true);
    ui->chkSetDefaultProfile->setEnabled(true);
    ui->btnSaveProfile->setEnabled(true);
    ui->btnDeleteProfile->setEnabled(true);
}

void frmProfileManager::DisableProfileFields()
{
    ui->txtProfileName->setText("");
    ui->txtProfileIPAddress->setText("");
    ui->txtProfilePassword->setText("");
    ui->txtProfilePort->setText("");
    ui->chkSetDefaultProfile->setChecked(false);
    ui->txtProfileName->setEnabled(false);
    ui->txtProfileIPAddress->setEnabled(false);
    ui->txtProfilePassword->setEnabled(false);
    ui->txtProfilePort->setEnabled(false);
    ui->chkSetDefaultProfile->setEnabled(false);
    ui->btnSaveProfile->setEnabled(false);
    ui->btnDeleteProfile->setEnabled(false);
}

void frmProfileManager::LoadProfile(int index)
{
    Profile *currentProfile = this->storedProfiles.at(index);
    if(currentProfile)
    {
        EnableProfileFields();
        ui->txtProfileName->setText(currentProfile->GetProfileName());
        ui->txtProfileIPAddress->setText(currentProfile->GetProfileIPAddress());
        ui->txtProfilePassword->setText(currentProfile->GetProfilePassword());
        ui->txtProfilePort->setText(currentProfile->GetProfileTCPPort() != 0 ? QString::number(currentProfile->GetProfileTCPPort()) : "");
        ui->chkSetDefaultProfile->setChecked(currentProfile->GetProfileAsDefault());

        if(!currentProfile->GetProfileAsDefault())
            ui->chkSetDefaultProfile->setEnabled(!IsDefaultProfileSelected());

        this->selectedProfileIndex = index;
    }
}

void frmProfileManager::SaveProfile()
{
    Profile *currentProfile = this->storedProfiles.at(this->selectedProfileIndex);
    if(currentProfile)
    {
        currentProfile->SetProfileName(ui->txtProfileName->text() != "None" ? ui->txtProfileName->text() : currentProfile->GetProfileName());
        currentProfile->SetProfileIPAddress(ui->txtProfileIPAddress->text());
        currentProfile->SetProfilePassword(ui->txtProfilePassword->text());
        currentProfile->SetProfileTCPPort(ui->txtProfilePort->text().toInt());
        currentProfile->SetProfileAsDefault(ui->chkSetDefaultProfile->isChecked());
        this->storedProfiles.replace(this->selectedProfileIndex, currentProfile);
        this->profileManager->SaveProfiles(this->storedProfiles);
    }
    DisableProfileFields();
    LoadProfiles();
    LoadProfile(this->selectedProfileIndex);
    ui->lstStoredProfiles->item(this->selectedProfileIndex)->setSelected(true);
    QMessageBox::information(this, "Profile", "Your profile has been saved!");
}

bool frmProfileManager::IsDefaultProfileSelected()
{
    for(int i = 0; i < this->storedProfiles.count(); i++)
    {
        if(this->storedProfiles.at(i)->GetProfileAsDefault())
            return true;
    }
    return false;
}

void frmProfileManager::OnProfileItemChange(int index)
{
    if(index != -1)
        LoadProfile(index);
}

void frmProfileManager::OnProfileItemClick(QListWidgetItem* item)
{
    QItemSelectionModel* selectionModel = ui->lstStoredProfiles->selectionModel();
    int index = selectionModel->selectedRows().at(0).row();

    if(index != -1)
        LoadProfile(index);
}
