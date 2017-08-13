// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "socket.h"

void Socket::SetServerPort(unsigned short int &serverPort)
{
    this->serverPort = serverPort;
}

unsigned short int Socket::GetServerPort() const
{
    return this->serverPort;
}

int Socket::GetListenSocket() const
{
    return listenSocket;
}

void Socket::SetListenSocket()
{
    listenSocket = socket(AF_INET, SOCK_STREAM, 0); // Set up socket with properties (AF_NET is TCP/UDP), Protocol (SOCK_STREAM is used by TCP) (SOCK_DGRAM is used for UDP), (0 Default).

    if(listenSocket == -1) // Check condition of socket, INVALID_SOCKET means it failed.
    {
        printf("\n[-] Socket initalise failure!\n");
        exit(-1);
    }
    printf("\t[+] Sockets initalised!\n");
}

void Socket::SetListenSocketProperties()
{
    const int reuseOptVal = 1;
    struct timeval timeoutVal;

    timeoutVal.tv_usec = 500;
    listenSocketProperties.sin_port = htons(serverPort); // Setting port with htons by converting it to network byte order.
    listenSocketProperties.sin_addr.s_addr = INADDR_ANY; // Setting Remote IP Address.
    listenSocketProperties.sin_family = AF_INET; // Socket family to use. (AF_NET = TCP/UDP).

    if (setsockopt(listenSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&timeoutVal,sizeof(struct timeval)) < 0)
        perror("setsockopt(SO_RCVTIMEO) failed");

    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseOptVal, sizeof(reuseOptVal)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    #ifdef SO_REUSEPORT
        if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuseOptVal, sizeof(reuseOptVal)) < 0)
            perror("setsockopt(SO_REUSEPORT) failed");
    #endif
}

void Socket::ListenSocketBind()
{
    if(bind(listenSocket,(struct sockaddr *)&listenSocketProperties, sizeof(listenSocketProperties)) == -1) // Bind socket 'listenSocket' with properties/size of 'listenSocketProperties. If running this command produces SOCKET_ERROR it will fail.
    {
        printf("\n\t[-] Failed to bind to port %d (TCP)!\n",GetServerPort()); // Print error..

        // Retry with a random port
        unsigned short int randomPort = (rand() % 65553 + 1);
        SetServerPort(randomPort);
        SetListenSocketProperties();
        ListenSocketBind();
    }
}

void Socket::ListenSocketDisconnect()
{
    if(shutdown(listenSocket, SHUT_RDWR) == 0)
        close(listenSocket);
}
