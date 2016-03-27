// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
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
    struct sockaddr_in servaddr; // Socket info
    struct hostent *host;     // Host information
    pthread_t c_thread; // Receiving thread
    int c_socket; // Current socket

    string results; // Returned results
    string ip_address; // IP to connect to
    unsigned int port; // TCP port
    bool is_connected; // Connection status
    bool transmission_end; // Transmission end status
} c_info;

class TCPClient
{
    public:
        static string GetIPAddress();
        static unsigned int GetPort();
        static string GetResults();
        static int CheckConnection(fd_set);
        static bool IsConnected();
        static bool IsTransmissionEnd();

        static bool Connect(const std::string&, const unsigned int&);
        static void ClearResults();
        static bool Disconnect();
        static void *run(void *);
        static bool SendMessage(const std::string&);
        static void SetTransmissionEnd(const bool& state);
};

#endif // CLIENT_H
