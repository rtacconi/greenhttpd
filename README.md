greenhttp server
================
A trivial static http webserver using Libevent's evhttp (for now).
The main feature will be to serve C scripts
to have fast web applications, low power consumption
and money saving. The URL mapping for C scripts will use the REST convention
and a routing tables will match the controllers in the script. For script 
I mean an automatically compiled program, or compiled on demand.   

Design
------
The heart of the server is the I/O event notification. Sockets are asynchronous, so not blocking. This allows to handle thousands of connection per second. Other high performance softwares using I/O event notification are nginx, memcached and others. This type of pattern work better than heavy threaded and fork-based solutions.

Libevent
--------
I am using libevent 2.0 stable: http://libevent.org/
Currently, libevent supports /dev/poll, kqueue(2), event ports, POSIX select(2), Windows select(), poll(2), and epoll(4). 

Platforms
---------
libevent can support many platforms but I decided not to support MS Windows. It has been only tested on the OSX but it should be and it will be compatible with Linux. It should work on other platforms such as Solaris and BSD.

Installation
------------
Go in the root of the project and compile it:

$ chmod +x build.sh 
$ ./build.sh

Usage
-----

To execute the server:

$ ./httpd

Put your files in www directory. There is not any configuration file, not yet. To visit the server go to:

http://localhost:8080/

The port is hard coded, for now. You can change the port by changing this line:

unsigned short port = 8080;

then recompile it. to stop the server just type CTRL + C like any other shell program. If you want to run it in the background:

$ ./httpd &



Notes
-----
Dynamic linking:

if(argc < 3)
    return printf("USAGE: %s lib-file function-name\n", argv[0]);

    lib = dlopen(argv[1], RTLD_NOW);
    if(lib == NULL)
        return printf("ERROR: Cannot load library\n");

    df = dlsym(lib, argv[2]);
