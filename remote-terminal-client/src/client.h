// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>          /* hostent struct, gethostbyname() */
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

using namespace std;

static struct Connection
{
    string ServerOutput; // Returned results
    bool IsConnected; // Connection status
    int CurrentSocket;
    struct hostent *ServerHost;     // Host information
    bool TransmissionEnd; // Transmission end status
    string ServerIPAddress; // Server IP
    unsigned int ServerPort; // Server TCP port
    pthread_t ReceiveThread;
    struct sockaddr_in SocketInfo; // Socket info
} TCPConn;

class TCPClient
{
    public:
        static string GetServerIPAddress();
        static unsigned int GetServerPort();
        static string GetServerOutput();
        static int CheckConnection(fd_set);
        static bool IsConnected();
        static bool IsTransmissionEnd();

        static bool Connect(const std::string&, const unsigned int&);
        static void ClearServerOutput();
        static void Disconnect();
        static void *run(void *);
        static bool SendMessage(const std::string&);
        static void SetTransmissionEnd(const bool&);

        static const string MESSAGE_CODE;
        static const string CANCEL_CODE;
        static const string DISCONNECT_CODE;
        static const string END_TRANSMISSION_CODE;
};

#endif // CLIENT_H
