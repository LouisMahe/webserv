# Webserv
## An asynchronous HTTP webserver in C++ (42 School Project with team mate [Anvently](https://github.com/Anvently))

Implemantation was done following [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616) and [RFC 3875](https://datatracker.ietf.org/doc/html/rfc3875). It is incomplete but the features implemented are mostly HTTP1.1 compliant.
The server reads a configuration file inspired by NGINX's one to configurates routes, permissions and so on.


## Usage and Config File
Clone the repository and build the programm with the Makefile,
```
git clone git@github.com:LouisMahe/webserv.git
cd webserv && make
```
If you dont provide a configuration file as argument, a [default one](/conf/template.conf) will be used as demonstration.

The configuration file let you define servers. Each server can listen to several ports and the server will match each of them, see the default one for examples.

## Methods implemented

The configuration file let you enable or disable methods and their options on the routes you define, The implemented methods are :

+ GET The GET method on a directory will provide a directory listing if the option is activated in the configuration file.
+ POST Sending a POST method on a non existing uri will be traited as an upload if the configuration file allow such action for the uri.
+ DELETE

## How it works

The server is single threaded, it uses [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) to monitor new clients arrivals and I/O operations available with each client. Any read or write operation is limited to BUFFER_SIZE (default to 4096) characters to avoid spending 
too much time on a given client. This means that most request won't be satisfied in one loop event but it ensures fluidity. The server use the `keep-alive` header and can deal with pipelined requests. It can receive bodies in chunks.
Since if is single threaded the server can't handle more thant 1024 clients at the same time.

## CGI

The server can call interpreted and compiled CGIs. It will refer to the config file. Environement variables are set according to RFC 3875. You can run the template.conf config file to see simple examples including session cookies. A time-out duration can be
defined with the Macro CGI_TIME_OUT in the `CGIProcess.hpp`.

## Security

Security concerns were not part of this project so the server is not equiped against malicious users. The servers will only time out clients taking too much time to send a complete request.


## Sender

To test the server we developped a sender programm that behaves like a client and that send hand-made http requests and will print the response it gets. You can define the IP, port and buffer_size it uses in the `sender.cpp file`. You can also define 
a PAUSE_TIME to simulate a slow client. 
