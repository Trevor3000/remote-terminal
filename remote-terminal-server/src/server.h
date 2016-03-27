// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
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
    int *clients = NULL; // Sockets - int type for GNU/Linux

    Server(unsigned short int, unsigned short int);
    ~Server();

    // Getters
    unsigned short int GetNumClients() const;
    unsigned short int GetMaxClients() const;
    unsigned short int GetCSocket() const;
    unsigned int GetMiliseconds();
    int CheckConnection(const unsigned short int &index, fd_set rsd);
    char* GetCurrentIP() const;

    // Setters
    void SetCSocket(unsigned short);
    void SetNumClients(const unsigned short &);
    void SetMaxClients(const unsigned short &);
    void SetCurrentIP(char *);
    void CloseSocket(unsigned short int);
    void ResetSockets();
    void SocketsDisconnect();

private:

    // Current Client
    char* current_ip;
    unsigned short int current_socket;
    unsigned short int num_clients;
    unsigned short int max_clients;
};

#endif // SERVER_H
