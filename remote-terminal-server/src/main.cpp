// This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
// Copyright © 2016 Matthew James 
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
#define ServerVersion() "0.1"

void SocketListen();
void *NewClient(void *ptr);
void *NewConsole(void *ptr);

typedef struct client_thread_data
{
    std::string input;
    std::string cd_command; // Current change directory command of client thread
    std::string hostname; // Current host name of client thread
    std::string username; // Current user name of client thread
    std::string directory; // Current directory of client thread
    bool is_cd; // Is change directory command?
    unsigned short int s_num; // Socket number
} con_data;


// Instances
Server * Service;
Crypto * Crypt;
int *ClientCodes = NULL; // Client console codes

int main(int argc, char *argv[])
{
    std::string key = ""; // key (mandatory)

    unsigned int port = 6001; // Default port
    unsigned int max_clients = 19; // 20 clients by default

    printf("%s\nRemote Terminal [Server]\n%s\nVersion: %s\n%s\n\n",GetSpacer(),GetSpacer(),ServerVersion(),GetSpacer());
    printf("[+] Usage: ./RTS <Key> <Port> <Max Clients>\n\n");
    printf("- Operations\n\n");

    if(argc > 1) // If the number of console arguments are greater than 1.
    {
        key.assign(argv[1]);

        if(argc > 2)
        {
            if(atoi(argv[2]) <= 65553 && atoi(argv[2]) > 0)
            {
                port = atoi(argv[2]);
            }
        }

        if(argc > 3)
        {
            if(atoi(argv[3]) <= 40 && atoi(argv[3]) > 0)
            {
                max_clients = atoi(argv[3]);
            }
        }
    }
    else
    {
        printf("\t[-] Error - Please provide an encryption key.\n\n");
        exit(-1);
    }

    Service = new Server(port, max_clients);
    Crypt = new Crypto(key);
    ClientCodes = new int[max_clients + 1];

    printf("\n\t[+] Listening on port %d [TCP]\n",Service->GetPort());

    // Set up sockets
    Service->ResetSockets();
    SocketListen();

    delete [] ClientCodes;
    delete Crypt;
    delete Service;

    return 0;
}

void SocketListen()
{
    const int timeout = 500;
    int s_prop_size = 0;

    while(1)
    {
        // Listen For Connections
        setsockopt(Service->GetLSocket(), SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
        while(listen(Service->GetLSocket(),Service->GetMaxClients()) == -1); // Listen for new connections (Stop once someone connects)
        for(unsigned short int i = 0; i <= Service->GetMaxClients(); i++) // Loop through sockets
        {
            // Accept Client
            if(Service->clients[i] == 0)
            {
                s_prop_size = sizeof(Service->s_prop);
                Service->clients[i] = accept(Service->GetLSocket(),(struct sockaddr*)&Service->s_prop, (socklen_t*) &s_prop_size); // Accept new connection from client using properties from other classes
                if(Service->clients[i] == -1)
                {
                    Service->CloseSocket(i); // Close invalid socket
                    break; // Skip this interval
                }

                ClientCodes[i] = 0; // 0 = keep console thread, 1 = close
                Service->SetCurrentIP(inet_ntoa(Service->s_prop.sin_addr)); // Set temporary IP
                Service->SetNumClients(Service->GetNumClients() + 1); // Set Number of clients
                Service->SetCSocket(i); // Set current used socket
                pthread_t tNewClient;
                pthread_create(&tNewClient,NULL,&NewClient,NULL);
            }
        }
        sleep(1);
    }
}

void EscapeDirectory(std::string &directory)
{
    std::string s;
    std::string new_directory;

    for(unsigned int i = 0; i < directory.length(); i++)
    {
        s = directory.at(i);

        if(s == " ")
        {
            new_directory.append("\\ ");
        }
        else
        {
            new_directory.append(s);
        }
    }

    directory = new_directory;
}

void *NewClient(void *ptr)
{
    std::string output, s_packet, exec_directory;
    std::string cd_command, directory, hostname, username;

    pthread_t tNewConsole;
    con_data tp;

    bool is_cd = false;
    char packet[512] = {0}; // Array to store received data
    char c_hostname[512] = {0}; // Array to store host name

    const unsigned short int s_num = Service->GetCSocket(); // This will store the current socket used
    unsigned short int bytes_received = 0; // Store the number of bytes received
    struct passwd *pw = getpwuid(getuid());

    if (gethostname(c_hostname,sizeof(c_hostname)) != -1)
    {
        hostname.assign(c_hostname); // Get host name
    }

    username.assign(pw->pw_name); // Set user name of current user

    if(cd_command.length() == 0)
    {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

        directory = std::string(result, (count > 0) ? count : 0); // Get current directory
        directory = directory.substr(0, directory.find_last_of("/\\")); // Go up one level

        exec_directory = directory;

        cd_command = "cd ";
        cd_command.append(directory);
    }

    // Print client information.
    printf("\n\tClient Action");
    printf("\n\t- - - - - - -");
    printf("\n\t- New connection request has been accepted!");
    printf("\n\t- The number of clients have increased to %d\n",Service->GetNumClients());

    // Used for checking whether a client exists (via input)
    fd_set read_sd;
    FD_ZERO(&read_sd);
    FD_SET(Service->clients[s_num], &read_sd);

    do
    {
        fd_set rsd = read_sd;

        if(Service->CheckConnection(s_num, rsd) < 0) // Check if client is still there.
        {
            bytes_received = 0;
        }

        bytes_received = recv(Service->clients[s_num],packet,sizeof(packet),0); // Receive information from client and get number of clients.

        if(bytes_received != 0)
        {
            s_packet.assign(packet);
            s_packet = Crypt->decryptToString(s_packet); // Decryption

            if(s_packet.length() > 0)
            {
                switch(s_packet.at(0))
                {
                case 'M': // Message
                {
                    memset(packet,0,sizeof(packet));
                    s_packet.erase(0,1);
                    s_packet.erase(std::remove(s_packet.begin(), s_packet.end(), '\n'), s_packet.end()); // Remove newline character
                    s_packet.erase(std::remove(s_packet.begin(), s_packet.end(), '\r'), s_packet.end()); // Remove return character

                    if(ClientCodes[s_num] == 0) // One thread at a time.
                    {
                        if(s_packet.find("sudo ") != std::string::npos && geteuid() != 0)   // Prevent use of sudo if non-root user (prevents thread from getting stuck)
                        {
                            output = "";
                            output.append("\nYou must be root to perform this command.\n");
                            output = Crypt->encryptToString(output); // Encryption
                            output.append("|");

                            send(Service->clients[s_num],output.c_str(), output.length(), 0);

                            // Send end of transmission code
                            output = "EOT";
                            send(Service->clients[s_num], output.c_str(), output.length(), 0);
                        }
                        else
                        {
                            // Check for cd command
                            if(s_packet.find("cd ..") != std::string::npos && s_packet.length() == 5) // For the command "cd .." - up one directory
                            {
                                directory.erase(std::remove(directory.begin(), directory.end(), '\\'), directory.end()); // Remove \\ character

                                directory = directory.substr(0, directory.find_last_of("/\\"));

                                // Add \ before spaces to indicate spaces in path
                                EscapeDirectory(directory);

                                cd_command = "cd ";
                                cd_command.append(directory);

                                s_packet = cd_command;

                                is_cd = true;
                            }
                            else if(s_packet.find("cd ",0,2) != std::string::npos && s_packet.length() > 2) // cd directory_name
                            {
                                directory = directory + "/" + s_packet.substr(3, s_packet.length());

                                // Add \ before spaces to indicate spaces in path
                                EscapeDirectory(directory);

                                cd_command = "cd " + directory;
                                is_cd = true;
                            }
                            else if(s_packet.find("cd",0,1) != std::string::npos && s_packet.length() == 2) // Switch to home directory
                            {
                                directory.assign(pw->pw_dir);

                                // Add \ before spaces to indicate spaces in path
                                EscapeDirectory(directory);

                                cd_command = "cd ";
                                cd_command.append(directory);

                                is_cd = true;
                            }
                            else
                            {
                                is_cd = false;
                            }

                            tp.input = s_packet;
                            tp.s_num = s_num;
                            tp.is_cd = is_cd;

                            tp.cd_command = cd_command;
                            tp.username = username;
                            tp.hostname = hostname;
                            tp.directory = directory;

                            if(pthread_create(&tNewConsole,NULL,NewConsole,(void *) &tp) !=0)
                            {
                                output.append("\nFailed to generate pthread.\n");
                                output = Crypt->encryptToString(output); // Encryption
                                output.append("|");

                                send(Service->clients[s_num],output.c_str(), output.length(), 0);

                                // Send end of transmission code
                                output = "EOT";
                                send(Service->clients[s_num], output.c_str(), output.length(), 0);
                            }
                        }
                    }
                }
                break;
                case 'X': // Kill Thread
                {
                    s_packet.erase(std::remove(s_packet.begin(), s_packet.end(), '\n'), s_packet.end()); // Remove newline character
                    s_packet.erase(std::remove(s_packet.begin(), s_packet.end(), '\r'), s_packet.end()); // Remove return character

                    ClientCodes[s_num] = 1; // Close thread
                    pthread_join(tNewConsole, NULL); // Wait for thread to close
                }
                break;
                case 'C': // Close
                {
                    s_packet.erase(std::remove(s_packet.begin(), s_packet.end(), '\n'), s_packet.end()); // Remove newline character
                    s_packet.erase(std::remove(s_packet.begin(), s_packet.end(), '\r'), s_packet.end()); // Remove return character

                    if(s_packet.length() == 1)
                    {
                        bytes_received = 0;
                    }
                }
                break;

                default: // Client without correct encryption code
                    bytes_received = 0;
                    break;
                }
            }
            else // Client without correct encryption code
            {
                bytes_received = 0;
            }
        }

        memset(packet,0,sizeof(packet));
        s_packet.clear();
        output.clear();
    }
    while (bytes_received != 0); // Keep checking for data until, nothing is received.

    Service->CloseSocket(s_num);
    Service->SetNumClients(Service->GetNumClients() - 1);

    printf("\n\tClient Action");
    printf("\n\t- - - - - - -");
    printf("\n\t- The number of clients have decreased to %d\n",Service->GetNumClients());

    return 0;
}

void *NewConsole(void *ptr)
{
    std::string s_output = "";
    std::string s_packet;
    std::string splitter = "";
    std::string cd_command = "";

    bool is_cd = false;
    unsigned int iCommandWaitTimer = 0;
    unsigned int iTimerStart = 0;

    unsigned short int s_num;

    con_data *pt;
    pt = (con_data *) ptr;

    s_output.append("\n" + pt->username + "@" + pt->hostname + ":" + pt->directory + "$ " + pt->input);

    cd_command = pt->cd_command;
    is_cd = pt->is_cd;

    if(cd_command.length() > 0 && !is_cd)
    {
        s_packet.append(cd_command + " && ");
    }

    s_packet.append(pt->input);
    s_num = pt->s_num;

    if(!is_cd)
    {
        s_packet.append(" 2>&1"); // Get output of stderr along with stdout

        s_output.append("\n\n");
        s_output = Crypt->encryptToString(s_output);
        s_output.append("|");

        send(Service->clients[s_num],s_output.c_str(), s_output.length(), 0);

        int fd[2];
        pid_t pid;

        if(pipe(fd) != -1)
        {
            if(fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL) | O_NONBLOCK) != -1) // Set output pipe to non-blocking
            {
                if ((pid = fork()) != -1) // Create pipes
                {
                    // Read from pipes

                    if(pid == 0)  // Child
                    {
                        close(1);
                        dup2(fd[1], 1);
                        close(fd[0]);
                        execlp("/bin/sh","/bin/sh","-c",s_packet.c_str(),NULL);
                        exit(0);
                    }
                    else  // Parent
                    {
                        int total_len = 0;
                        char procpath[100];
                        char bytes[513] = {0};
                        ssize_t bytes_read;

                        FILE *proc;
                        sprintf(procpath, "/proc/%d/status", pid + 1);

                        iTimerStart = Service->GetMiliseconds();

                        bool process_open = true;

                        do
                        {
                            iCommandWaitTimer = (Service->GetMiliseconds() - iTimerStart);

                            bytes_read = read(fd[0], bytes, 512);

                            if (bytes_read == -1 && errno == EAGAIN)  // No data available from pipe
                            {
                                if((iCommandWaitTimer > 6000 && !process_open) || (total_len < 512 && !process_open)) // No data left in pipe
                                {
                                    break;
                                }
                                else if(iCommandWaitTimer > 1000 && process_open)
                                {
                                    proc = fopen(procpath,"r"); // Check to see if process is running
                                    if (proc == NULL)
                                    {
                                        process_open = false;
                                    }
                                    else
                                    {
                                        fclose(proc);
                                    }
                                }

                                if(ClientCodes[s_num] == 1) // || iCommandWaitTimer > 1500)
                                {
                                    process_open = false;
                                    break;
                                }
                            }
                            else if (bytes_read > 0) // Data available from pipe
                            {
                                s_output = "";
                                s_output.assign(bytes);

                                total_len += strlen(bytes);

                                s_output = Crypt->encryptToString(s_output);
                                s_output.append("|");

                                send(Service->clients[s_num],s_output.c_str(), s_output.length(), 0);
                            }
                            else // Pipe closed
                            {
                                break;
                            }

                            s_output = "";
                            memset(bytes, 0, sizeof(bytes));

                            usleep(10);
                        }
                        while(1);

                        close(fd[0]);

                        kill(pid, SIGKILL); // Kill shell pipe process
                        kill(pid + 1, SIGKILL); // Kill pipe process

                        //s_output.append("\n"); //  For debugging purposes
                        //s_output.append(s_packet);
                    }
                }
            }
            else
            {
                close(fd[0]);
                close(fd[1]);
            }
        }
    }

    // Send end of transmission code
    s_output = "EOT";
    send(Service->clients[s_num], s_output.c_str(), s_output.length(), 0);
    ClientCodes[s_num] = 0; // Clear code

    return 0;
}

