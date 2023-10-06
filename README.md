# linux-command-line-chat
This is a very simple chatserver for the Linux command-line

## Building
As *cmake* is used as build system, a building this project is pretty easy.
Please note, that *Sqlite* is used as a database for the users login data and make sure to install it before building the project.
```
sudo apt install libsqlite3-dev
```

For a clean build execute the following commands, starting in the project directory:
```
mkdir build
cd build
cmake ..
make
```

To rebuild afterwards, executing *make* within the *build*-directory is sufficient.

## Usage
After building the build folder should contain the executable named *'server'*

To start the server simply navigate to the directory that contains the executable and execute it using:
```
./main <port>
```
The only argument *'port'* is the portnumber (16bit unsigned int) on which the server should listen for new connections.
It is recommended to stick to portnumbers within the range [1024, 65536], as the ones from 0 to 1023 are well-known ports and might already be in use.

## Connecting to the server
Until I add a client program, netcat can be used to connect to the server:
```
nc -v <server-ip> <server-port>
```
After connecting every client is promted to register or login with username and password, using the corresponding command.

```
/register <username> <password>
/login <username> <password>
```

The usernames are unique and can only be used once.

**Disclaimer**: The communication between server and clients is by no means encrypted, as raw TCP sockets are used.

## Functionality
Up to now, the server supports simple message forwarding between registered users (the ones with a username), 
global server messages sent via the server-console, notifications for joining and leaving clients and the command '/exit' to close the server (notifies that the server was closed to logged in clients).
