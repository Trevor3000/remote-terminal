// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "server.h"

Server::Server(unsigned short int port, unsigned short int maxClients)
{
    this->numClients = 0;

    SetMaxClients(maxClients);
    SetServerPort(port);
    SetListenSocket();
    SetListenSocketProperties();
    ListenSocketBind();
    SocketsDisconnect();
}

Server::~Server() // Stop
{
    SocketsDisconnect();
    ListenSocketDisconnect();

    delete [] this->clients;
    this->clients = NULL;
}

unsigned short int Server::GetNumClients() const
{
    return this->numClients;
}

unsigned short int Server::GetMaxClients() const
{
    return this->maxClients;
}

unsigned short int Server::GetCurrentClientSocket() const
{
    return this->currentClientSocket;
}

char* Server::GetCurrentClientIP() const
{
    return this->currentClientIP;
}

void Server::SetCurrentClientSocket(unsigned short currentClientSocket)
{
    this->currentClientSocket = currentClientSocket;
}

void Server::SetNumClients(const unsigned short &numClients)
{
    this->numClients = numClients;
}

void Server::SetMaxClients(const unsigned short &maxClients)
{
    this->maxClients = maxClients;
    this->clients = new int[this->maxClients + 1];
}

void Server::SetCurrentClientIP(char *currentClientIP)
{
    this->currentClientIP = currentClientIP;
}

int Server::CheckClientConnection(const unsigned short int &index, fd_set readFileDesc)
{
    return select(this->clients[index] + 1, &readFileDesc, 0, 0, 0);
}

void Server::ResetSockets()
{
    for(unsigned short int i = 0; i <= this->maxClients; i++)
    {
        this->clients[i] = 0; // Null current socket by setting it to 0.
    }
}

unsigned int Server::GetProcessMiliseconds()
{
    struct timeval timeInterval;
    /* structure definition of timeval is
     struct timeval {
          time_t      tv_sec;      seconds
          suseconds_t tv_usec;    microseconds
    };*/

    if (gettimeofday(&timeInterval, NULL) != 0)
        return 0;

    return (timeInterval.tv_sec * 1000) + (timeInterval.tv_usec / 1000);
}

void Server::CloseSocket(unsigned short int index)
{
    if(this->clients[index] == -1)
    {
        this->clients[index] = 0; // Null global socket
    }
    else
    {
        close(this->clients[index]); // Close global socket
        this->clients[index] = 0; // Null global socket
    }
}

void Server::SocketsDisconnect()
{
    for(unsigned short int i = 0; i <= this->maxClients; i++)
    {
        if(clients[i] == -1)
        {
            this->clients[i] = 0; // Null global socket
        }
        else
        {
            close(this->clients[i]); // Close global socket
            this->clients[i] = 0; // Null global socket
        }
    }
}
