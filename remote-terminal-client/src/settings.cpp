// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include <settings.h>

Settings::Settings(Crypto& crypto)
{
    this->crypto = &crypto;
    this->lastProfileIndex = -1;

    SetDefaultPaths();

    if(CheckSettings())
    {
        GetSettings();

        if(this->crypto->IsInvalidKey())
            SetDefaultSettings();
    }
    else
    {
        SetDefaultSettings();
    }
}

void Settings::SetDefaultPaths()
{
    this->homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    this->topDirectory = this->homePath + QDir::separator() + ".remote-terminal-client";
    this->settingsPath = this->homePath + QDir::separator() + ".remote-terminal-client/Settings/Settings.xml";
    this->profilesPath = this->homePath + QDir::separator() + ".remote-terminal-client/Settings/Profiles.xml";
    this->settingsDirectory = this->homePath + QDir::separator() + ".remote-terminal-client/Settings";
}

void Settings::SetDefaultSettings()
{
    this->lastProfileIndex = -1;
    SaveSettings();
}

bool Settings::CheckSettings()
{
    QDir topDirectory(this->topDirectory);

    if(!topDirectory.exists())
        topDirectory.mkdir(this->topDirectory);

    QDir settingsDirectory(this->settingsDirectory);

    if(!settingsDirectory.exists())
        settingsDirectory.mkdir(this->settingsDirectory);

    QFile settingsFile(this->settingsPath);
    return settingsFile.exists() && settingsFile.size() != 0;
}

Crypto* Settings::GetCrypto()
{
    return this->crypto;
}

QString Settings::GetProfilesPath()
{
    return this->profilesPath;
}

int Settings::GetLastProfileIndex()
{
    return this->lastProfileIndex;
}

void Settings::SetLastProfileIndex(const int& lastProfileIndex)
{
    this->lastProfileIndex = lastProfileIndex;
}

bool Settings::GetSettings()
{
    bool result = false;
    QString xmlContent = this->crypto->DecryptSettingsFile(this->settingsPath);

    if(!this->crypto->IsInvalidKey())
    {
        QXmlStreamReader reader(xmlContent);

        while(!reader.atEnd()) // Read until the end
        {
            QXmlStreamReader::TokenType token = reader.readNext();
            if(token == QXmlStreamReader::StartElement)
            {
                if(reader.name() == "LastProfileIndex")
                    this->lastProfileIndex = reader.readElementText().toInt();
            }
        }
        result = true;

        if (reader.hasError())
        {
            QMessageBox::critical(NULL,"XML Reading Error", reader.errorString());
            result = false;
        }
    }
    return result;
}

bool Settings::SaveSettings()
{
    QString xmlContent = "";
    QXmlStreamWriter writer(&xmlContent);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("Settings");
    writer.writeTextElement("LastProfileIndex", QString::number(this->lastProfileIndex));
    writer.writeEndElement();
    writer.writeEndDocument();
    return this->crypto->EncryptSettingsFile(this->settingsPath, xmlContent);
}
