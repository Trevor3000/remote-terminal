// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include <profile.h>

Profile::Profile(){
}

Profile::Profile(QString profileName, QString profileIPAddress, QString profilePassword, int profileTCPPort, bool profileAsDefault)
{
    this->profileName = profileName;
    this->profileIPAddress = profileIPAddress;
    this->profilePassword = profilePassword;
    this->profileTCPPort = profileTCPPort;
    this->profileAsDefault = profileAsDefault;
}

QString Profile::GetProfileName()
{
    return this->profileName;
}

QString Profile::GetProfileIPAddress()
{
    return this->profileIPAddress;
}

QString Profile::GetProfilePassword()
{
    return this->profilePassword;
}

int Profile::GetProfileTCPPort()
{
    return this->profileTCPPort;
}

bool Profile::GetProfileAsDefault()
{
    return this->profileAsDefault;
}

void Profile::SetProfileName(QString profileName)
{
    this->profileName = profileName;
}

void Profile::SetProfileIPAddress(QString profileIPAddress)
{
    this->profileIPAddress = profileIPAddress;
}

void Profile::SetProfilePassword(QString profilePassword)
{
    this->profilePassword = profilePassword;
}

void Profile::SetProfileTCPPort(int profileTCPPort)
{
    this->profileTCPPort = profileTCPPort;
}

void Profile::SetProfileAsDefault(bool profileAsDefault)
{
    this->profileAsDefault = profileAsDefault;
}
