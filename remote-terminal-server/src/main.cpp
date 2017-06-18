// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2017 Matthew James
// "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctime>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>

#include "server.h"
#include "crypto.h"

#define GetSpacer() "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
#define ServerVersion() "0.15"

void SocketListen();
void *NewClient(void *ptr);
void *NewConsole(void *ptr);

typedef struct ClientThreadData
{
    std::string input;
    std::string hostname; // Current host name of client thread
    std::string username; // Current user name of client thread
    std::string directory; // Current directory of client thread
    unsigned short int socketID; // Socket number
} ThreadData;


Server * Service;
Crypto * Crypt;

int *ClientCodes = NULL; // Client console codes

int main(int argc, char *argv[])
{
    std::string key = ""; // key (mandatory)

    unsigned int serverPort = 6001; // Default port
    unsigned int maxClients = 19; // 20 clients by default

    printf("%s\nRemote Terminal [Server]\n%s\nVersion: %s\n%s\n\n",GetSpacer(),GetSpacer(),ServerVersion(),GetSpacer());
    printf("[+] Usage: ./RTS <Key> <Port> <Max Clients>\n\n");
    printf("- Operations\n\n");

    if(argc > 1) // If the number of console arguments are greater than 1.
    {
        key.assign(argv[1]);

        if(argc > 2)
        {
            if(atoi(argv[2]) <= 65553 && atoi(argv[2]) > 0)
                serverPort = atoi(argv[2]);
        }

        if(argc > 3)
        {
            if(atoi(argv[3]) <= 40 && atoi(argv[3]) > 0)
                maxClients = atoi(argv[3]);
        }
    }
    else
    {
        printf("\t[-] Error - Please provide an encryption key.\n\n");
        exit(-1);
    }

    Service = new Server(serverPort, maxClients);
    Crypt = new Crypto(key);
    ClientCodes = new int[maxClients + 1];

    printf("\n\t[+] Listening on port %d [TCP]\n",Service->GetServerPort());

    Service->ResetSockets();
    SocketListen();

    delete [] ClientCodes;
    delete Crypt;
    delete Service;

    return 0;
}

void SocketListen()
{
    const int clientTimeout = 500;
    int socketPropertiesSize = 0;

    while(1)
    {
        // Listen For Connections
        setsockopt(Service->GetListenSocket(), SOL_SOCKET, SO_RCVTIMEO, (char *)&clientTimeout, sizeof(clientTimeout));
        while(listen(Service->GetListenSocket(),Service->GetMaxClients()) == -1); // Listen for new connections (Stop once someone connects)

        for(unsigned short int i = 0; i <= Service->GetMaxClients(); i++) // Loop through sockets
        {
            // Accept Client
            if(Service->clients[i] == 0)
            {
                socketPropertiesSize = sizeof(Service->listenSocketProperties);
                Service->clients[i] = accept(Service->GetListenSocket(),(struct sockaddr*)&Service->listenSocketProperties, (socklen_t*) &socketPropertiesSize); // Accept new connection from client using properties from other classes

                if(Service->clients[i] == -1)
                {
                    Service->CloseSocket(i); // Close invalid socket
                    break; // Skip this interval
                }

                ClientCodes[i] = 0; // 0 = keep console thread, 1 = close
                Service->SetCurrentClientIP(inet_ntoa(Service->listenSocketProperties.sin_addr));
                Service->SetNumClients(Service->GetNumClients() + 1); // Set Number of clients
                Service->SetCurrentClientSocket(i); // Set current used socket
                pthread_t newClientThread;
                pthread_create(&newClientThread,NULL,&NewClient,NULL);
            }
        }
        sleep(1);
    }
}

void EscapeDirectory(std::string &directory)
{
    std::string part;
    std::string newDirectory;

    for(unsigned int i = 0; i < directory.length(); i++)
    {
        part = directory.at(i);
        newDirectory.append(part == " " ? "\\ " : part);
    }
    directory = newDirectory;
}

void *NewClient(void *ptr)
{
    std::string output, sPacket;
    std::string hostname, username;
    std::string currentDirectory;

    pthread_t newConsoleThread;
    ThreadData threadData;

    char packet[512] = {0}; // Array to store received data
    char currentHostname[512] = {0}; // Array to store host name
    char startDirectory[PATH_MAX] = {0};
    fd_set readFileDesc;

    const unsigned short int socketID = Service->GetCurrentClientSocket(); // This will store the current socket used
    unsigned short int bytesReceived = 0; // Store the number of bytes received
    struct passwd *userInfo = getpwuid(getuid());

    if (gethostname(currentHostname,sizeof(currentHostname)) != -1)
        hostname.assign(currentHostname); // Get host name

    username.assign(userInfo->pw_name); // Set user name of current user

    readlink("/proc/self/cwd", startDirectory, PATH_MAX);
    threadData.directory = std::string(startDirectory);
    currentDirectory = std::string(startDirectory);

    // Print client information.
    printf("\n\tClient Action");
    printf("\n\t- - - - - - -");
    printf("\n\t- New connection request has been accepted!");
    printf("\n\t- The number of clients have increased to %d\n",Service->GetNumClients());

    // Used for checking whether a client exists (via input)
    FD_ZERO(&readFileDesc);
    FD_SET(Service->clients[socketID], &readFileDesc);

    do
    {
        if(Service->CheckClientConnection(socketID, readFileDesc) < 0) // Check if client is still there.
            break;

        bytesReceived = recv(Service->clients[socketID],packet,sizeof(packet),0); // Receive information from client and get number of clients.

        if(bytesReceived != 0)
        {
            sPacket.assign(packet);
            sPacket = Crypt->DecryptString(sPacket); // Decryption

            if(sPacket.length() > 0)
            {
                switch(sPacket.at(0))
                {
                    case 'M': // Message
                    {
                        memset(packet,0,sizeof(packet));
                        sPacket.erase(0,1);
                        sPacket.erase(std::remove(sPacket.begin(), sPacket.end(), '\n'), sPacket.end()); // Remove newline character
                        sPacket.erase(std::remove(sPacket.begin(), sPacket.end(), '\r'), sPacket.end()); // Remove return character

                        if(ClientCodes[socketID] == 0) // One thread at a time.
                        {
                            if(sPacket.find("sudo ") != std::string::npos && geteuid() != 0)   // Prevent use of sudo if non-root user (prevents thread from getting stuck)
                            {
                                output = "";
                                output.append("\nYou must be root to perform this command.\n");
                                output = Crypt->EncryptString(output); // Encryption
                                output.append("|");

                                send(Service->clients[socketID],output.c_str(), output.length(), 0);

                                // Send end of transmission code
                                output = "EOT";
                                send(Service->clients[socketID], output.c_str(), output.length(), 0);
                            }
                            else if(sPacket.find("cd") != std::string::npos)
                            {
                                if(sPacket.find("cd ..") != std::string::npos && sPacket.length() == 5) // For the command "cd .." - up one directory
                                {
                                    currentDirectory.erase(std::remove(currentDirectory.begin(), currentDirectory.end(), '\\'), currentDirectory.end()); // Remove \\ character
                                    currentDirectory = currentDirectory.substr(0, currentDirectory.find_last_of("/\\"));
                                    EscapeDirectory(currentDirectory);
                                }
                                else if(sPacket.find("cd ",0,2) != std::string::npos && sPacket.length() > 2) // cd directory_name
                                {
                                    currentDirectory = currentDirectory + "/" + sPacket.substr(3, sPacket.length());
                                    EscapeDirectory(currentDirectory);
                                }
                                else if(sPacket.find("cd",0,1) != std::string::npos && sPacket.length() == 2) // Switch to home directory
                                {
                                    currentDirectory.assign(userInfo->pw_dir);
                                    EscapeDirectory(currentDirectory);
                                }

                                threadData.directory = currentDirectory;

                                // Send end of transmission code
                                output = "EOT";
                                send(Service->clients[socketID], output.c_str(), output.length(), 0);
                            }
                            else
                            {
                                threadData.input = sPacket;
                                threadData.socketID = socketID;
                                threadData.username = username;
                                threadData.hostname = hostname;

                                if(pthread_create(&newConsoleThread,NULL,NewConsole,(void *) &threadData) !=0)
                                {
                                    output.append("\nFailed to generate pthread.\n");
                                    output = Crypt->EncryptString(output); // Encryption
                                    output.append("|");

                                    send(Service->clients[socketID],output.c_str(), output.length(), 0);

                                    // Send end of transmission code
                                    output = "EOT";
                                    send(Service->clients[socketID], output.c_str(), output.length(), 0);
                                    bytesReceived = 0;
                                }
                            }
                        }
                    }
                    break;
                    case 'X': // Kill Thread
                    {
                        sPacket.erase(std::remove(sPacket.begin(), sPacket.end(), '\n'), sPacket.end()); // Remove newline character
                        sPacket.erase(std::remove(sPacket.begin(), sPacket.end(), '\r'), sPacket.end()); // Remove return character

                        ClientCodes[socketID] = 1; // Close thread
                        pthread_join(newConsoleThread, NULL); // Wait for thread to close
                    }
                    break;
                    case 'C': // Close
                    {
                        sPacket.erase(std::remove(sPacket.begin(), sPacket.end(), '\n'), sPacket.end()); // Remove newline character
                        sPacket.erase(std::remove(sPacket.begin(), sPacket.end(), '\r'), sPacket.end()); // Remove return character

                        if(sPacket.length() == 1)
                            bytesReceived = 0;
                    }
                    break;

                    default: // Client without correct encryption code
                        bytesReceived = 0;
                        break;
                }
            }
            else // Client without correct encryption code
            {
                bytesReceived = 0;
            }
        }

        memset(packet,0,sizeof(packet));
        sPacket.clear();
        output.clear();
    }
    while (bytesReceived != 0); // Keep checking for data until, nothing is received.

    Service->CloseSocket(socketID);
    Service->SetNumClients(Service->GetNumClients() - 1);

    printf("\n\tClient Action");
    printf("\n\t- - - - - - -");
    printf("\n\t- The number of clients have decreased to %d\n",Service->GetNumClients());
    return 0;
}

void *NewConsole(void *ptr)
{
    int pipes[2];
    pid_t processID;

    std::string serverOutput = "";
    std::string sPacket;

    unsigned int commandWaitTimer = 0;
    unsigned int timerStart = 0;

    ThreadData *threadData = (ThreadData *)ptr;

    serverOutput.append("\n" + threadData->username + "@" + threadData->hostname + ":"
                             + threadData->directory + "$ " + threadData->input);

    sPacket.append(threadData->input);
    sPacket.append(" 2>&1"); // Get output of stderr along with stdout

    serverOutput.append("\n\n");
    serverOutput = Crypt->EncryptString(serverOutput);
    serverOutput.append("|");

    send(Service->clients[threadData->socketID],serverOutput.c_str(), serverOutput.length(), 0);

    chdir(threadData->directory.c_str());

    if(pipe(pipes) != -1)
    {
        if(fcntl(pipes[0], F_SETFL, fcntl(pipes[0], F_GETFL) | O_NONBLOCK) != -1) // Set output pipe to non-blocking
        {
            if ((processID = fork()) != -1) // Create pipes
            {
                // Read from pipes
                if(processID == 0)  // Child
                {
                    close(pipes[0]);
                    dup2(pipes[1], 1);

                    execlp("/bin/sh","/bin/sh","-c",sPacket.c_str(),NULL);
                    exit(0);
                }
                else  // Parent
                {
                    close(pipes[1]);

                    int totalPipeBytes = 0;
                    char processPath[100] = {0};
                    char readPipeBuffer[513] = {0};

                    FILE *proc;
                    ssize_t bytesRead;
                    bool processOpen = true;

                    sprintf(processPath, "/proc/%d/status", processID + 1);
                    timerStart = Service->GetProcessMiliseconds();

                    do
                    {
                        commandWaitTimer = (Service->GetProcessMiliseconds() - timerStart);

                        bytesRead = read(pipes[0], readPipeBuffer, 512);

                        if (bytesRead == -1 && errno == EAGAIN)  // No data available from pipe
                        {
                            if((commandWaitTimer > 6000 && !processOpen) || (totalPipeBytes < 512 && !processOpen)) // No data left in pipe
                            {
                                break;
                            }
                            else if(commandWaitTimer > 1000 && processOpen)
                            {
                                proc = fopen(processPath,"r"); // Check to see if process is running

                                if (proc == NULL)
                                {
                                    processOpen = false;
                                }
                                else
                                {
                                    fclose(proc);
                                }
                            }

                            if(ClientCodes[threadData->socketID] == 1) // || iCommandWaitTimer > 1500)
                            {
                                processOpen = false;
                                break;
                            }
                        }
                        else if (bytesRead > 0) // Data available from pipe
                        {
                            serverOutput = "";
                            serverOutput.assign(readPipeBuffer);

                            totalPipeBytes += strlen(readPipeBuffer);

                            serverOutput = Crypt->EncryptString(serverOutput);
                            serverOutput.append("|");

                            send(Service->clients[threadData->socketID],serverOutput.c_str(), serverOutput.length(), 0);
                        }
                        else // Pipe closed
                        {
                            break;
                        }

                        serverOutput = "";
                        memset(readPipeBuffer, 0, sizeof(readPipeBuffer));

                        usleep(10);
                    }
                    while(1);

                    close(pipes[0]);

                    kill(processID, SIGKILL); // Kill shell pipe process
                    kill(processID + 1, SIGKILL); // Kill pipe process

                    //serverOutput.append("\n"); //  For debugging purposes
                    //serverOutput.append(sPacket);
                }
            }
        }
        else
        {
            close(pipes[0]);
            close(pipes[1]);
        }
    }

    // Send end of transmission code
    serverOutput = "EOT";
    send(Service->clients[threadData->socketID], serverOutput.c_str(), serverOutput.length(), 0);
    ClientCodes[threadData->socketID] = 0; // Clear code

    pthread_exit(NULL);
}

