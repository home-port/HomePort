Home-Port Refactured Release
=========

The following describes how to checkout and compile the sources

# Checkout

Install git:

`sudo apt-get install git`

Clone the repository:

`git clone https://github.com/home-port/HomePort.git HomePort`

Switch branch:

`cd HomePort`

`git checkout v0.4-refactored`

# Compile

Install the dependencies:

`sudo apt-get install gcc make libev-dev cmake libmxml-dev`

Quick compile:
`./build.sh`

Alternative, compile manually, it is advised to compile in a seperate directory:

`mkdir build && cd build`

`cmake ../`

`make`

# Run tests

Install requirements:

`sudo apt-get install libcurl4-gnutls-dev`

Run cmake as normal:

`mkdir build && cd build`

`cmake ../`

Run the tests:

`make check`

# Build examples

Run cmake as normal:

`mkdir build && cd build`

`cmake ../`

Build examples

`make example`

Example executables can be found in the various example directories
within the build folder.

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

