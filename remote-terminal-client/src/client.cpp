// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "client.h"

unsigned int TCPClient::GetPort()
{
    return c_info.port;
}

bool TCPClient::IsConnected()
{
    return c_info.is_connected;
}

bool TCPClient::IsTransmissionEnd()
{
    return c_info.transmission_end;
}

string TCPClient::GetIPAddress()
{
    return c_info.ip_address;
}

string TCPClient::GetResults()
{
    return c_info.results;
}

void TCPClient::SetTransmissionEnd(const bool& state)
{
    c_info.transmission_end = state;
}

bool TCPClient::SendMessage(const std::string& command)
{
    bool result = false;

    int bytes = send(c_info.c_socket,command.c_str(), command.length(), 0);

    if(bytes > 0){
        result = true;
    }
    return result;
}

int TCPClient::CheckConnection(fd_set rsd)
{
    return select(c_info.c_socket + 1, &rsd, 0, 0, 0);
}

bool TCPClient::Connect(const std::string& ip_address, const unsigned int& port)
{
    bool result = false;

    c_info.ip_address = ip_address;
    c_info.port = port;

    memset(&c_info.servaddr, 0, sizeof(c_info.servaddr));
    c_info.is_connected = false;

    if ((c_info.host = gethostbyname(c_info.ip_address.c_str())) == NULL) {
        printf("\n[-] NSlookup failed on '%s'", c_info.ip_address.c_str());
        result = false;
    }
    if(!result)
    {
        printf("\n\n[+] Nslook up successful!");
        printf("\n[+] Setting up socket");

        c_info.c_socket = socket(AF_INET, SOCK_STREAM, 0); // Set up socket with properties (AF_NET is TCP/UDP), Protocol (SOCK_STREAM is used by TCP) (SOCK_DGRAM is used for UDP), (0 Default).
        c_info.servaddr.sin_family = AF_INET;
        c_info.servaddr.sin_port = htons(c_info.port);
        c_info.servaddr.sin_addr.s_addr = inet_addr(c_info.ip_address.c_str());

        if(c_info.c_socket == -1){
            result = false;
        }
        else
        {
            if (connect(c_info.c_socket, (struct sockaddr *) &c_info.servaddr, sizeof(c_info.servaddr)) < 0) {
                close(c_info.c_socket);
                printf("\n[-] Couldn't connect to server.\n");
                result = false;
            }
            else{
                printf("\n[+] Connected!");

                pthread_create(&c_info.c_thread,NULL,run,NULL);

                c_info.is_connected = true;
                result = true;
            }
        }
    }
    return result;
}

void TCPClient::ClearResults()
{
    c_info.results = "";
}

void *TCPClient::run(void *ptr)
{
    char packet[512] = {0};
    string s_packet;
    int bytes = 0;

    // Used for checking whether the server exists (via input)
    fd_set read_sd;
    FD_ZERO(&read_sd);
    FD_SET(c_info.c_socket, &read_sd);

    do
    {
        fd_set rsd = read_sd;

        if(CheckConnection(rsd) < 0) // Check if server is still there.
        {
            bytes = 0;
        }

        bytes = recv(c_info.c_socket, packet, sizeof(packet), 0);

        if(bytes > 0)
        {
            if(!c_info.transmission_end)
            {
                s_packet.assign(packet);

                //printf(packet);

                if(s_packet.length() > 0)
                {
                    c_info.results.append(s_packet);

                    if(s_packet.find("EOT") != string::npos) {
                        s_packet.erase(s_packet.length() - 3, s_packet.length()); // Remove EOT from transmission
                        c_info.transmission_end = true;
                    }

                }
            }
        }

        memset(packet,0,sizeof(packet));
    }
    while(bytes > 0);

    Disconnect();

    return 0;
}

bool TCPClient::Disconnect()
{
    if(c_info.c_socket)
    {
        c_info.is_connected = false;
        c_info.transmission_end = false;
        close(c_info.c_socket);
    }
    return false;
}
