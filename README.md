# Remote Terminal

Remote Terminal was created as a means of controlling GNU/Linux based machines remotely. It utilises AES-256 bit encryption (GCM) to secure its transmissions via a shared encryption code. Both the client and server application must know the encryption code before successful communication can occur; if the encryption code is incorrect, the client will be disconnected.

Any terminal command can be run on the multi-threaded remote server (providing root access is permitted). If root access is not permitted, the server will run commands on the privileges of the current user. You can customise the number of clients, port number and encryption code for the server application.

It utilises the crypto++ library and Qt Framework to provide its features.

#### Screenshots

Remote Terminal [Client]:

![Image of Remote Terminal [Client] (Version 0.1)](https://raw.githubusercontent.com/mjsware/remote-terminal/master/images/RTC.png)

Remote Terminal [Server]:

![Image of Remote Terminal [Server] (Version 0.1)](https://raw.githubusercontent.com/mjsware/remote-terminal/master/images/RTS.png)

#### Current version

Version Number: 0.15

Dated : 25-06-2017 

#### Features in the current version

* Ability to view command history via the up and down keys in the client application.

* Ability to change directories with in-built cd commands (cd directory_name, cd .., cd) in the server application.

* AES-256 bit (GCM) encrypted transmissions via TCP protocol.

* Client application with graphical user interface (GUI).

* Multi-threaded server application with ability to customise the number of clients, port number and encryption code. If a TCP port is not available to bind to, the server application will take any available port (if not specified as a parameter).


#### Known Issues

* Utilising commands that require a further response from an initial command will not work, unless you're explicit in the initial command. For example, if you decide to execute "apt-get upgrade" command, please specify the "-y" flag at the end otherwise it will hang. If it does hang, please click the "Cancel" button on the client application and this should resolve the hanging issue via killing those processes.


#### Licence

This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.

>Copyright 2017 Matthew James 
 This file is part of "Remote Terminal".  
 "Remote Terminal" is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free Software Foundation,
 either version 3 of the License, or (at your option) any later version.
 "Remote Terminal" is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with "Remote Terminal".
 If not, see http://www.gnu.org/licenses/.

#### Contribute

To contribute, simply folk, clone or patch and send a pull request. The client project has been built using Qt Creator, and the server project was built using Code::Blocks (16.01).

Use Qt Creator with Qt Framework (>= 5.7) to build the project. 
 
#### Support Libraries
 
 - [crypto++] (https://www.cryptopp.com)
