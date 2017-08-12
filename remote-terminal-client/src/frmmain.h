// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QSettings>

#include "client.h"
#include "crypto.h"
#include "settings.h"
#include "settings_crypto.h"
#include "frmabout.h"
#include "frmprofilemanager.h"

namespace Ui
{
    class frmMain;
}

class frmMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();
    void ClearSelectedProfile();
    void LoadStoredProfiles();

public slots:
    void ClearLog();
    void keyPressEvent(QKeyEvent* pe);
    void moveEvent(QMoveEvent*);
    void resizeEvent(QResizeEvent*);
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject *, QEvent *);
    void LoadWindowSettings();
    void SaveWindowSettings();
    void OnCommandTextChange();
    void ViewAboutRemoteTerminal();
    void ViewWebsite();
    void ViewProfileManager();
    void ConnectClick();
    void CloseApplication();
    void Connect();
    void Disconnect();
    void SendCommand();
    void CancelCommand();
    void CheckMessages();
    void LoadSelectedProfile(int);

private:
    Ui::frmMain *ui;
    Crypto *crypto;
    Crypto *settingsCrypto;
    Settings *settings;
    ProfileManager *profileManager;
    frmAbout *aboutForm;
    frmProfileManager *profileManagerForm;
    QVector<Profile*> storedProfiles;
    QTimer messageTimer; // Message check timer
    QTimer connectTimer;
    QStringList commands;
    int commandIndex;
};

#endif // FRMMAIN_H
