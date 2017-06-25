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

    connect(ui->btnConnect, SIGNAL(clicked(bool)),this, SLOT(btnConnect_Click()));
    connect(ui->btnSend, SIGNAL(clicked(bool)),this, SLOT(SendCommands()));
    connect(ui->btnClearLog, SIGNAL(clicked(bool)),this, SLOT(ClearLog()));
    connect(ui->txtCommand, SIGNAL(textChanged(QString)),this, SLOT(txtCommand_TextChanged()));
    connect(ui->itemExit,SIGNAL(triggered()),this,SLOT(CloseApplication()));
    connect(ui->itemAboutRemoteTerminal,SIGNAL(triggered()),this,SLOT(itemAboutRemoteTerminal()));
    connect(ui->itemWebsite,SIGNAL(triggered()),this,SLOT(itemViewWebsite()));
    connect(&messageTimer, SIGNAL(timeout()), this, SLOT(CheckMessages()));

    this->aboutForm = 0;
    this->crypto = 0;
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

void frmMain::txtCommand_TextChanged()
{
    ui->btnSend->setEnabled(ui->txtCommand->text().length() > 0);
}

void frmMain::CloseApplication()
{
    if(TCPClient::IsConnected())
        Disconnect();

    this->close();
}

void frmMain::itemAboutRemoteTerminal()
{
    if(this->aboutForm)
        delete this->aboutForm;

    this->aboutForm = new frmAbout();
    this->aboutForm->move(this->rect().center() - this->aboutForm->rect().center());
    this->aboutForm->show();
}

void frmMain::itemViewWebsite()
{
    QDesktopServices::openUrl(QUrl("https://github.com/mjsware/remote-terminal"));
}

void frmMain::btnConnect_Click()
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

    if(this->crypto)
        delete this->crypto;

    if(this->aboutForm)
        delete this->aboutForm;

    delete ui;
}

// Methods

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
    if(ui->txtIPHost->text().length() > 0 && ui->txtEncryptionCode->text().length() > 0 && ui->txtPort->text().length() > 0)
    {
        if(TCPClient::Connect(ui->txtIPHost->text().toStdString(), ui->txtPort->text().toInt()))
        {
            this->commandIndex = -1;
            this->commands.clear();

            if(this->crypto)
                delete this->crypto;

            this->crypto = new Crypto(ui->txtEncryptionCode->text().toStdString());

            ui->lblStatus->setText("Connected!");
            ui->btnConnect->setText("Disconnect");
            ui->txtCommand->setEnabled(true);
            ui->txtIPHost->setEnabled(false);
            ui->txtPort->setEnabled(false);
            ui->txtEncryptionCode->setEnabled(false);
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
    ui->txtEncryptionCode->setEnabled(true);
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
