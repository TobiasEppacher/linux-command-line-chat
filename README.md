# linux-command-line-chat
This is a very simple chatserver for the Linux command-line

## Building
As *cmake* is used as build system, a building this project is pretty easy.
Please note, that *doctest* was used for a simple testfile and is therefore required for building.

For a clean build execute the following commands:
```
mkdir build
cd build
cmake ..
make
```

After this if you just want to rebuild, executing *make* within the *build*-directory is sufficient.

## Usage
After building the build folder should contain two executables, *main* and *tests*

As mentioned above, *tests* is a simple testfile that was used to test some very basic functionalities of TCPSocket. 
For using the program itself this executable is not needed and can be ignored.

The usage of *main* is very straightforward.
To start the server simply navigate to the build folder and execute:
```
./main <port>
```
The only argument *<port>* is the portnumber (16bit unsigned int) on which the server should listen for new connections.
Stick to ports within the range [1024, 65536], as the ones from 0 to 1023 are well-known ports and might already be in use.

## Connecting to the server
Until I add a client program, netcat can be used to connect to the server:
```
nc -v <server-ip> <server-port>
```
After connecting every client is asked to enter a username, which is used to uniquely identify the user while he is connected.
Please note, that this is _**no authentication**_ as the username is free again after the client occuping it disconnects and can be taken
up by any newly connected client.

## Functionality
Up to now, the server supports simple message forwarding between registered users (the ones with a username), 
global server messages sent via the server-console, notifications for joining and leaving clients and the command '/exit' to close the server (notified to registered clients).
