// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

class Socket
{
public:

    struct sockaddr_in s_prop; // Listen sock properties.

    // Getters
    unsigned short int GetPort() const;
    int GetLSocket() const;

    // Setters
    void SetPort(unsigned short int &);
    void SetLSocket();
    void SetLProperties();
    void LSocketBind();
    void LSDisconnect();

private:
    int listen_sock; // Listen Socket.
    unsigned short int port;
};

#endif // SOCKET_H
