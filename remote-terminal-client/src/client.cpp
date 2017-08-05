// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include "client.h"

const string TCPClient::MESSAGE_CODE = "M";
const string TCPClient::CANCEL_CODE = "X";
const string TCPClient::DISCONNECT_CODE = "C";
const string TCPClient::END_TRANSMISSION_CODE = "EOT";

unsigned int TCPClient::GetServerPort()
{
    return TCPConn.ServerPort;
}

bool TCPClient::IsConnected()
{
    return TCPConn.IsConnected;
}

bool TCPClient::IsTransmissionEnd()
{
    return TCPConn.TransmissionEnd;
}

string TCPClient::GetServerIPAddress()
{
    return TCPConn.ServerIPAddress;
}

string TCPClient::GetServerOutput()
{
    return TCPConn.ServerOutput;
}

void TCPClient::SetTransmissionEnd(const bool& transmissionEnd)
{
    TCPConn.TransmissionEnd = transmissionEnd;
}

bool TCPClient::SendMessage(const std::string& clientCommand)
{
    return send(TCPConn.CurrentSocket, clientCommand.c_str(), clientCommand.length(), 0) > 0;
}

int TCPClient::CheckConnection(fd_set readFileDesc)
{
    return select(TCPConn.CurrentSocket + 1, &readFileDesc, 0, 0, 0);
}

bool TCPClient::Connect(const std::string& serverIPAddress, const unsigned int& serverPort)
{
    TCPConn.ServerIPAddress = serverIPAddress;
    TCPConn.ServerPort = serverPort;
    TCPConn.IsConnected = false;

    memset(&TCPConn.SocketInfo, 0, sizeof(TCPConn.SocketInfo));

    if ((TCPConn.ServerHost = gethostbyname(TCPConn.ServerIPAddress.c_str())) == NULL)
    {
        printf("\n[-] NSlookup failed on '%s'", TCPConn.ServerIPAddress.c_str());
    }
    else
    {
        printf("\n\n[+] Nslook up successful!");
        printf("\n[+] Setting up socket");

        TCPConn.CurrentSocket = socket(AF_INET, SOCK_STREAM, 0); // Set up socket with properties (AF_NET is TCP/UDP), Protocol (SOCK_STREAM is used by TCP) (SOCK_DGRAM is used for UDP), (0 Default).
        TCPConn.SocketInfo.sin_family = AF_INET;
        TCPConn.SocketInfo.sin_port = htons(TCPConn.ServerPort);
        TCPConn.SocketInfo.sin_addr.s_addr = inet_addr(TCPConn.ServerIPAddress.c_str());

        if(TCPConn.CurrentSocket != -1)
        {
            if (connect(TCPConn.CurrentSocket, (struct sockaddr *) &TCPConn.SocketInfo, sizeof(TCPConn.SocketInfo)) < 0)
            {
                close(TCPConn.CurrentSocket);
                printf("\n[-] Couldn't connect to server.\n");
            }
            else
            {
                printf("\n[+] Connected!");
                TCPConn.IsConnected = true;
                pthread_create(&TCPConn.ReceiveThread,NULL,run,NULL);
            }
        }
    }
    return TCPConn.IsConnected;
}

void TCPClient::ClearServerOutput()
{
    TCPConn.ServerOutput = "";
}

void *TCPClient::run(void *ptr)
{
    string sPacket;
    int receivedBytes = 0;
    char packet[512] = {0};

    // Used for checking whether the server exists (via input)
    fd_set readFileDesc;
    FD_ZERO(&readFileDesc);
    FD_SET(TCPConn.CurrentSocket, &readFileDesc);

    do
    {
        if(CheckConnection(readFileDesc) < 0) // Check if server is still there.
            break;

        receivedBytes = recv(TCPConn.CurrentSocket, packet, sizeof(packet), 0);

        if(receivedBytes > 0 && !TCPConn.TransmissionEnd)
        {
            sPacket.assign(packet);

            if(sPacket.length() > 0)
            {
                TCPConn.ServerOutput.append(sPacket);
                TCPConn.TransmissionEnd = sPacket.find(END_TRANSMISSION_CODE) != string::npos;
            }
        }
        memset(packet,0,sizeof(packet));
    }
    while(receivedBytes > 0);

    Disconnect();
    return 0;
}

void TCPClient::Disconnect()
{
    if(TCPConn.CurrentSocket)
    {
        TCPConn.IsConnected = false;
        TCPConn.TransmissionEnd = false;

        if(shutdown(TCPConn.CurrentSocket, SHUT_RDWR) == 0)
            close(TCPConn.CurrentSocket);
    }
}
