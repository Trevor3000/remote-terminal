// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include <profilemanager.h>

ProfileManager::ProfileManager(Settings& settings)
{
    this->settings = &settings;
}

QVector<Profile*> ProfileManager::GetProfiles()
{
    QVector<Profile*> storedProfiles = QVector<Profile*>(0);

    QString xmlContent = this->settings->GetCrypto()->DecryptSettingsFile(this->settings->GetProfilesPath());

    if(!this->settings->GetCrypto()->IsInvalidKey())
    {
        Profile *currentProfile = 0;
        QXmlStreamReader reader(xmlContent);

        QXmlStreamReader::TokenType outerToken;
        QXmlStreamReader::TokenType innerToken;

        while(!reader.atEnd()) // Read until the end
        {
            outerToken = reader.readNext();

            if(outerToken == QXmlStreamReader::StartElement)
            {
                while(!reader.atEnd())
                {
                    innerToken = reader.readNext();

                    if(innerToken == QXmlStreamReader::StartElement && reader.name() == "Profile")
                    {
                        if(reader.attributes().hasAttribute("ProfileName"))
                        {
                            currentProfile = new Profile();
                            currentProfile->SetProfileName(this->settings->GetCrypto()->DecryptString(reader.attributes().value("ProfileName").toString()));
                        }

                        if(currentProfile)
                        {
                            if(reader.attributes().hasAttribute("ProfileIPAddress"))
                                currentProfile->SetProfileIPAddress(this->settings->GetCrypto()->DecryptString(reader.attributes().value("ProfileIPAddress").toString()));

                            if(reader.attributes().hasAttribute("ProfilePassword"))
                                currentProfile->SetProfilePassword(this->settings->GetCrypto()->DecryptString(reader.attributes().value("ProfilePassword").toString()));

                            if(reader.attributes().hasAttribute("ProfileTCPPort"))
                                currentProfile->SetProfileTCPPort(reader.attributes().value("ProfileTCPPort").toInt());

                            if(reader.attributes().hasAttribute("ProfileIsDefault"))
                            {
                                currentProfile->SetProfileAsDefault(reader.attributes().value("ProfileIsDefault").toInt());
                                storedProfiles.append(currentProfile);
                            }
                        }
                    }
                }
            }
        }
        if (reader.hasError())
            storedProfiles = QVector<Profile*>(0);
    }
    return storedProfiles;
}

bool ProfileManager::SaveProfiles(QVector<Profile*> selectedProfiles)
{
    QDomDocument profilesXMLDoc;
    QDomElement profileDomNode;
    QDomNode parentProfileNode = profilesXMLDoc.createElement("Profiles");

    for(int i = 0; i < selectedProfiles.count(); i++)
    {
        profileDomNode = profilesXMLDoc.createElement("Profile");
        profileDomNode.setAttribute("ProfileName",this->settings->GetCrypto()->EncryptString(selectedProfiles.at(i)->GetProfileName()));
        profileDomNode.setAttribute("ProfileIPAddress",this->settings->GetCrypto()->EncryptString(selectedProfiles.at(i)->GetProfileIPAddress()));
        profileDomNode.setAttribute("ProfilePassword",this->settings->GetCrypto()->EncryptString(selectedProfiles.at(i)->GetProfilePassword()));
        profileDomNode.setAttribute("ProfileTCPPort",QString::number(selectedProfiles.at(i)->GetProfileTCPPort()));
        profileDomNode.setAttribute("ProfileIsDefault",selectedProfiles.at(i)->GetProfileAsDefault() ? "1" : "0");
        parentProfileNode.appendChild(profileDomNode);
    }
    profilesXMLDoc.appendChild(parentProfileNode);
    return this->settings->GetCrypto()->EncryptSettingsFile(this->settings->GetProfilesPath(), profilesXMLDoc.toByteArray());
}
