// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <ctime>

#include "socket.h"

class Server : public Socket
{

public:
    int *clients = NULL;

    Server(unsigned short int, unsigned short int);
    ~Server();

    unsigned short int GetNumClients() const;
    unsigned short int GetMaxClients() const;
    unsigned short int GetCurrentClientSocket() const;
    unsigned int GetProcessMiliseconds();

    int CheckClientConnection(const unsigned short int &, fd_set);
    char* GetCurrentClientIP() const;

    void SetCurrentClientSocket(unsigned short);
    void SetNumClients(const unsigned short &);
    void SetMaxClients(const unsigned short &);
    void SetCurrentClientIP(char *);
    void CloseSocket(unsigned short int);
    void ResetSockets();
    void SocketsDisconnect();

private:

    char* currentClientIP;
    unsigned short int currentClientSocket;
    unsigned short int numClients;
    unsigned short int maxClients;
};

#endif // SERVER_H
