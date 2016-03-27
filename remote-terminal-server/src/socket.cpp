// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "socket.h"

void Socket::SetPort(unsigned short int &port)
{
    this->port = port;
}
unsigned short int Socket::GetPort() const
{
    return port;
}
int Socket::GetLSocket() const
{
    return listen_sock;
}
void Socket::SetLSocket()
{
    listen_sock = socket(AF_INET, SOCK_STREAM, 0); // Set up socket with properties (AF_NET is TCP/UDP), Protocol (SOCK_STREAM is used by TCP) (SOCK_DGRAM is used for UDP), (0 Default).
    if(listen_sock == -1) // Check condition of socket, INVALID_SOCKET means it failed.
    {
        printf("\n[-] Socket initalise failure!\n");
        exit(-1);
    }
    printf("\t[+] Sockets initalised!\n");
}
void Socket::SetLProperties()
{
    s_prop.sin_port = htons(port); // Setting port with htons by converting it to network byte order.
    s_prop.sin_addr.s_addr = INADDR_ANY; // Setting Remote IP Address.
    s_prop.sin_family = AF_INET; // Socket family to use. (AF_NET = TCP/UDP).
}
void Socket::LSocketBind()
{
    if(bind(listen_sock,(struct sockaddr *)&s_prop, sizeof(s_prop)) == -1) // Bind socket 'serverSock' with properties/size of 'lsock. If running this command produces SOCKET_ERROR it will fail.
    {
        printf("\n\t[-] Failed to bind to port %d (TCP)!\n",GetPort()); // Print error..

        // Retry with a random port
        unsigned short int r_port = (rand() % 65553 + 1);
        SetPort(r_port);
        SetLProperties();
        LSocketBind();
    }
}
void Socket::LSDisconnect()
{
    close(listen_sock); // Close listening sock.
}
