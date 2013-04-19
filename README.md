Home-Port Consolidated Release
=========

The following describes how to checkout and compile the sources

# Checkout

Install git:

`sudo apt-get install git`

Clone the repository:

`git clone https://github.com/eudyptula/Encourage.git encourage`

# Compile

Install the dependencies:

`sudo apt-get install gcc make libev-dev cmake`

Compile:

It is advised to compile in a seperate directory:
`mkdir build`

First build your makefiles with cmake:
`cmake ../`

Then compile:
`make`

# Update

Update the repository to the current version:

`git pull`

# Build API Documentation

Install doxygen:

`sudo apt-get install doxygen graphviz`

Compile it:

`make doc`

# List of various useful links

* [Socket programming in C](http://www.linuxhowtos.org/C_C++/socket.htm)
* [Beej's Guide to Network Programming (includes IPv6 support)](http://beej.us/guide/bgnet/output/html/multipage/index.html)
* [libev documentation](http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod)
* [microhttpd](http://www.gnu.org/software/libmicrohttpd/)
* [RFC 2616](http://www.w3.org/Protocols/rfc2616/rfc2616.html)

