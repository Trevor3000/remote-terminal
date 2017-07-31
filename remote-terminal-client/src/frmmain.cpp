// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "frmmain.h"
#include "ui_frmmain.h"

frmMain::frmMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::frmMain)
{
    ui->setupUi(this);

    LoadWindowSettings();

    connect(ui->btnConnect, SIGNAL(clicked(bool)),this, SLOT(ConnectClick()));
    connect(ui->btnSend, SIGNAL(clicked(bool)),this, SLOT(SendCommands()));
    connect(ui->btnClearLog, SIGNAL(clicked(bool)),this, SLOT(ClearLog()));
    connect(ui->txtCommand, SIGNAL(textChanged(QString)),this, SLOT(OnCommandTextChange()));
    connect(ui->itemExit,SIGNAL(triggered()),this,SLOT(CloseApplication()));
    connect(ui->itemAboutRemoteTerminal,SIGNAL(triggered()),this,SLOT(ViewAboutRemoteTerminal()));
    connect(ui->itemWebsite,SIGNAL(triggered()),this,SLOT(ViewWebsite()));
    connect(ui->itemOpenProfileManager,SIGNAL(triggered()),this,SLOT(ViewProfileManager()));
    connect(ui->cboSelectedProfile, SIGNAL(currentIndexChanged(int)), SLOT(LoadSelectedProfile(int)));
    connect(&messageTimer, SIGNAL(timeout()), this, SLOT(CheckMessages()));

    ui->cboSelectedProfile->installEventFilter(this);

    this->aboutForm = 0;
    this->profileManagerForm = 0;
    this->crypto = 0;

    this->settingsCrypto = new Crypto(SettingsCrypto::GetUniqueSystemHash().toStdString());
    this->settings = new Settings(*this->settingsCrypto);
    this->settings->GetSettings();
    this->profileManager = new ProfileManager(*this->settings);

    LoadStoredProfiles();
}

void frmMain::LoadStoredProfiles()
{
    int defaultIndex = -1;
    this->storedProfiles = this->profileManager->GetProfiles();
    ui->cboSelectedProfile->clear();

    for(int i = 0; i < this->storedProfiles.count(); i++)
    {
        ui->cboSelectedProfile->addItem(this->storedProfiles.at(i)->GetProfileAsDefault() ? this->storedProfiles.at(i)->GetProfileName() + " (Default)"
                                                                                          : this->storedProfiles.at(i)->GetProfileName());
        if(this->storedProfiles.at(i)->GetProfileAsDefault())
            defaultIndex = i;
    }
    ui->cboSelectedProfile->addItem("None");
    ui->cboSelectedProfile->setCurrentIndex(defaultIndex != -1 ? defaultIndex
                                                               : this->settings->GetLastProfileIndex());

    if(ui->cboSelectedProfile->itemText(ui->cboSelectedProfile->currentIndex()) != "None")
        LoadSelectedProfile(defaultIndex);
}

// Events

void frmMain::keyPressEvent(QKeyEvent* pressedEvent)
{
    if(pressedEvent->key() == Qt::Key_Return)
    {
        SendCommands();
    }
    else if(pressedEvent->key() == Qt::Key_Up) // Show recent command history
    {
        this->commandIndex += 1;

        if(this->commands.count() > this->commandIndex)
        {
            ui->txtCommand->setText(this->commands.at(this->commandIndex));
        }
        else
        {
            this->commandIndex = this->commands.count() - 1;
        }
    }
    else if(pressedEvent->key() == Qt::Key_Down) // Show past command history
    {
        if(this->commandIndex >= 0 && this->commands.count() > this->commandIndex)
        {
            ui->txtCommand->setText(this->commands.at(this->commandIndex));
            this->commandIndex -= 1;
        }
        else
        {
            this->commandIndex = 0;
        }
    }
}

void frmMain::moveEvent(QMoveEvent*)
{
    SaveWindowSettings();
}

void frmMain::resizeEvent(QResizeEvent*)
{
    SaveWindowSettings();
}

void frmMain::closeEvent(QCloseEvent*)
{
    SaveWindowSettings();
}

bool frmMain::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->cboSelectedProfile && event->type() == QEvent::MouseButtonPress)
        LoadStoredProfiles();
    return false;
}

void frmMain::OnCommandTextChange()
{
    ui->btnSend->setEnabled(ui->txtCommand->text().length() > 0);
}

void frmMain::CloseApplication()
{
    if(TCPClient::IsConnected())
        Disconnect();

    this->close();
}

void frmMain::ViewProfileManager()
{
    if(this->profileManagerForm)
        delete this->profileManagerForm;

    this->profileManagerForm = new frmProfileManager(*this->profileManager);
    this->profileManagerForm->move(this->rect().center() - this->profileManagerForm->rect().center());
    this->profileManagerForm->show();
}


void frmMain::ViewAboutRemoteTerminal()
{
    if(this->aboutForm)
        delete this->aboutForm;

    this->aboutForm = new frmAbout();
    this->aboutForm->move(this->rect().center() - this->aboutForm->rect().center());
    this->aboutForm->show();
}

void frmMain::ViewWebsite()
{
    QDesktopServices::openUrl(QUrl("https://github.com/mjsware/remote-terminal"));
}

void frmMain::ConnectClick()
{
    if(TCPClient::IsConnected())
    {
        Disconnect();
    }
    else
    {
        Connect();
    }
}

frmMain::~frmMain()
{
    SaveWindowSettings();

    if(ui->cboSelectedProfile->currentIndex() != -1)
    {
        this->settings->SetLastProfileIndex(ui->cboSelectedProfile->currentIndex());
        this->settings->SaveSettings();
    }

    if(this->aboutForm)
        delete this->aboutForm;

    if(this->profileManagerForm)
        delete this->profileManagerForm;

    if(this->settingsCrypto)
        delete this->settingsCrypto;

    if(this->crypto)
        delete this->crypto;

    if(this->profileManager)
        delete this->profileManager;

    if(this->settings)
        delete this->settings;

    delete ui;
}

// Methods

void frmMain::ClearSelectedProfile()
{
    ui->txtIPHost->setText("");
    ui->txtPassword->setText("");
    ui->txtPort->setText("");
}

void frmMain::LoadSelectedProfile(int index)
{
    if(index != -1)
    {
        ClearSelectedProfile();

        if(ui->cboSelectedProfile->itemText(index) != "None")
        {
            Profile *currentProfile = this->storedProfiles.at(index);

            if(currentProfile)
            {
                ui->txtIPHost->setText(currentProfile->GetProfileIPAddress());
                ui->txtPassword->setText(currentProfile->GetProfilePassword());
                ui->txtPort->setText(currentProfile->GetProfileTCPPort() != 0 ? QString::number(currentProfile->GetProfileTCPPort()) : "");
            }
        }
    }
}

void frmMain::LoadWindowSettings()
{
    QSettings qSettings("Matthew James", "Remote Terminal [Client]");

    qSettings.beginGroup("frmMain");

    restoreGeometry(qSettings.value("geometry", saveGeometry()).toByteArray());
    restoreState(qSettings.value("save_state",saveState()).toByteArray());
    move(qSettings.value("pos", pos()).toPoint());
    resize(qSettings.value("size", size()).toSize());

    if (qSettings.value("maximised", isMaximized()).toBool())
        showMaximized();

    qSettings.endGroup();
}

void frmMain::SaveWindowSettings()
{
    QSettings qSettings("Matthew James", "Remote Terminal [Client]");

    qSettings.beginGroup("frmMain");

    qSettings.setValue("geometry", saveGeometry());
    qSettings.setValue("save_state", saveState());
    qSettings.setValue("maximised", isMaximized());

    if (!isMaximized())
    {
        qSettings.setValue("pos", pos());
        qSettings.setValue("size", size());
    }
    qSettings.endGroup();
}

void frmMain::ClearLog()
{
    ui->txtTerminalOutput->setText("");
}

void frmMain::Connect()
{
    if(ui->txtIPHost->text().length() > 0 && ui->txtPassword->text().length() > 0 && ui->txtPort->text().length() > 0)
    {
        if(TCPClient::Connect(ui->txtIPHost->text().toStdString(), ui->txtPort->text().toInt()))
        {
            this->commandIndex = -1;
            this->commands.clear();

            if(this->crypto)
                delete this->crypto;

            this->crypto = new Crypto(ui->txtPassword->text().toStdString());

            ui->lblStatus->setText("Connected!");
            ui->btnConnect->setText("Disconnect");
            ui->txtCommand->setEnabled(true);
            ui->txtIPHost->setEnabled(false);
            ui->txtPort->setEnabled(false);
            ui->txtPassword->setEnabled(false);
            this->messageTimer.start(1000);
        }
    }
}

void frmMain::Disconnect()
{
    TCPClient::ClearServerOutput();

    if(TCPClient::IsConnected())
    {
        TCPClient::SendMessage(this->crypto->EncryptString(TCPClient::DISCONNECT_CODE)); // Send disconnect command to server
        TCPClient::Disconnect();
    }

    ui->lblStatus->setText("Not Connected");
    ui->btnConnect->setText("Connect");
    ui->btnSend->setText("Send");
    ui->txtCommand->setEnabled(false);
    ui->btnSend->setEnabled(false);
    ui->txtIPHost->setEnabled(true);
    ui->txtPort->setEnabled(true);
    ui->txtPassword->setEnabled(true);
    this->messageTimer.stop();
}

void frmMain::CheckMessages()
{
    if(TCPClient::IsConnected())
    {
        if(TCPClient::GetServerOutput().length() > 0)
        {
            std::string serverOutput = "";
            QStringList serverOutputList = QString::fromStdString(TCPClient::GetServerOutput()).split("|");

            if(serverOutputList.length() > 0)
            {
                for(int i = 0; i < serverOutputList.length(); i++)
                {
                    string outputLine = serverOutputList[i].toStdString();

                    if(outputLine.length() > 0 && outputLine != TCPClient::END_TRANSMISSION_CODE)
                        serverOutput.append(this->crypto->DecryptString(outputLine));
                }
            }

            if(!TCPClient::IsTransmissionEnd())
            {
                if(serverOutput.length() > 0)
                    ui->txtTerminalOutput->append(QString::fromStdString(serverOutput));

                TCPClient::ClearServerOutput();
            }
            else
            {
                if(serverOutput.length() > 0)
                    ui->txtTerminalOutput->append(QString::fromStdString(serverOutput));

                ui->txtCommand->setEnabled(true);
                ui->btnSend->setText("Send");
                ui->btnSend->setEnabled(false);

                TCPClient::ClearServerOutput();
                TCPClient::SetTransmissionEnd(false);
            }
        }
    }
    else
    {
        Disconnect();
    }
}

void frmMain::SendCommands()
{
    if(ui->btnSend->text() == "Send")
    {
        if(TCPClient::IsConnected() && !TCPClient::IsTransmissionEnd())
        {
            TCPClient::ClearServerOutput();

            string command = TCPClient::MESSAGE_CODE + ui->txtCommand->text().toStdString();

            this->commands.append(ui->txtCommand->text());
            this->commandIndex = this->commands.count() - 1;
            ui->txtCommand->setText("");

            if(TCPClient::SendMessage(this->crypto->EncryptString(command)))
            {
                ui->btnSend->setText("Cancel");
                ui->btnSend->setEnabled(true);
            }
        }
    }
    else if(ui->btnSend->text() == "Cancel")
    {
        if(TCPClient::IsConnected() && !TCPClient::IsTransmissionEnd())
            TCPClient::SendMessage(this->crypto->EncryptString(TCPClient::CANCEL_CODE));
    }
}
