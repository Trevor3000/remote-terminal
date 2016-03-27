// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "frmmain.h"
#include "ui_frmmain.h"

frmMain::frmMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmMain)
{
    ui->setupUi(this);

    LoadWindowSettings();

    // Button clicks
    connect(ui->btnConnect, SIGNAL(clicked(bool)),this, SLOT(btnConnect_click()));
    connect(ui->btnSend, SIGNAL(clicked(bool)),this, SLOT(SendCommands()));
    connect(ui->btnClearLog, SIGNAL(clicked(bool)),this, SLOT(ClearLog()));

    // Text change
    connect(ui->txtCommand, SIGNAL(textChanged(QString)),this, SLOT(txtCommand_textChanged()));

    // Menu clicks
    connect(ui->itemExit,SIGNAL(triggered()),this,SLOT(CloseApplication()));
    connect(ui->itemAboutRemoteTerminal,SIGNAL(triggered()),this,SLOT(itemAboutRemoteTerminal()));
    connect(ui->itemWebsite,SIGNAL(triggered()),this,SLOT(itemViewWebsite()));

    // Message timer
    connect(&message_timer, SIGNAL(timeout()), this, SLOT(CheckMessages()));
}

// Events

void frmMain::keyPressEvent(QKeyEvent* pe)
{
    if(pe->key() == Qt::Key_Return)
    {
        SendCommands();
    }
    else if(pe->key() == Qt::Key_Up) // Show recent command history
    {
        command_index += 1;

        if(commands.count() > command_index)
        {
            ui->txtCommand->setText(commands.at(command_index));
        }
        else
        {
            command_index = commands.count() - 1;
        }
    }
    else if(pe->key() == Qt::Key_Down) // Show past command history
    {
        if(command_index >= 0 && commands.count() > command_index)
        {
            ui->txtCommand->setText(commands.at(command_index));
            command_index -= 1;
        }
        else
        {
            command_index = 0;
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

void frmMain::txtCommand_textChanged()
{
    if(ui->txtCommand->text().length() > 0)
    {
        ui->btnSend->setEnabled(true);
    }
    else
    {
        ui->btnSend->setEnabled(false);
    }
}

void frmMain::CloseApplication()
{
    this->close();
}

void frmMain::itemAboutRemoteTerminal()
{
    aboutForm = new frmAbout();
    aboutForm->move(this->rect().center() - aboutForm->rect().center());
    aboutForm->show();
}

void frmMain::itemViewWebsite()
{
    QDesktopServices::openUrl(QUrl("http://www.mjsware.co.uk"));
}

void frmMain::btnConnect_click()
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

    if(crypto){
        delete crypto;
    }

    if(crypto){
        delete aboutForm;
    }

    delete ui;
}

// Methods

void frmMain::LoadWindowSettings()
{
    QSettings qSettings("Matthew James", "Remote Terminal Client");

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
    QSettings qSettings("Matthew James", "Remote Terminal Client");

    qSettings.beginGroup("frmMain");

    qSettings.setValue("geometry", saveGeometry());
    qSettings.setValue("save_state", saveState());
    qSettings.setValue("maximised", isMaximized());

    if (!isMaximized()) {
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
    if(ui->txtIPHost->text().length() > 0)
    {
        if(ui->txtEncryptionCode->text().length() > 0)
        {
            if(ui->txtPort->text().length() > 0)
            {
                QString ip_address = ui->txtIPHost->text();
                int port = ui->txtPort->text().toInt();

                if(TCPClient::Connect(ip_address.toStdString(), port))
                {
                    command_index = -1;
                    commands.clear();

                    crypto = new Crypto(ui->txtEncryptionCode->text().toStdString());

                    ui->lblStatus->setText("Connected!");
                    ui->btnConnect->setText("Disconnect");
                    ui->txtCommand->setEnabled(true);

                    ui->txtIPHost->setEnabled(false);
                    ui->txtPort->setEnabled(false);
                    ui->txtEncryptionCode->setEnabled(false);

                    this->message_timer.start(1000);
                }
            }
        }
    }
}

void frmMain::Disconnect()
{
        TCPClient::ClearResults();

        if(TCPClient::IsConnected())
        {
            string e_string = crypto->encryptToString("C");
            TCPClient::SendMessage(e_string); // Send disconnect command to server
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

        this->message_timer.stop();
}

void frmMain::CheckMessages()
{
    if(TCPClient::IsConnected())
    {
        if(TCPClient::GetResults().length() > 0)
        {
            std::string output = "";
            QStringList results = QString::fromStdString(TCPClient::GetResults()).split("|");

            if(results.length() > 0)
            {
                for(int i = 0; i < results.length(); i++)
                {
                    QString line = results[i];
                    if(line.length() > 0)
                    {
                        output.append(crypto->decryptToString(line.toStdString()));
                    }
                }
            }

            if(!TCPClient::IsTransmissionEnd())
            {
                if(output.length() > 0)
                {
                    ui->txtTerminalOutput->append(QString::fromStdString(output));
                }

                TCPClient::ClearResults();
            }
            else
            {
                if(output.length() > 0)
                {
                    ui->txtTerminalOutput->append(QString::fromStdString(output));
                }

                ui->txtCommand->setEnabled(true);
                ui->btnSend->setText("Send");
                ui->btnSend->setEnabled(false);

                TCPClient::ClearResults();
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
            TCPClient::ClearResults();

            QString command = "M" + ui->txtCommand->text();
            commands.append(ui->txtCommand->text());
            command_index = commands.count() - 1;

            string e_string = crypto->encryptToString(command.toStdString());

            ui->txtCommand->setText("");

            if(TCPClient::SendMessage(e_string))
            {
                ui->btnSend->setText("Cancel");
                ui->btnSend->setEnabled(true);
            }
        }
    }
    else if(ui->btnSend->text() == "Cancel")
    {
        if(TCPClient::IsConnected() && !TCPClient::IsTransmissionEnd())
        {
            QString command = "X";
            string e_string = crypto->encryptToString(command.toStdString());

            TCPClient::SendMessage(e_string);
        }
    }
}
