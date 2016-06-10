# HomePort Documentation

Full API Documentation can be found at [https://home-port.github.io/HomePort/index.html](https://home-port.github.io/HomePort/index.html)

## Download and install HomePort

This guide has been tested on Ubuntu Desktop 16.04, with the (as of
writing) current version of HomePort.

## Prerequisites

For checkout of sources (Alternatively zip-files can be downloaded
through GitHub):

`sudo apt-get install git`

To compile in general:

`sudo apt-get install cmake gcc make g++ libbsd-dev libev-dev libcurl4-gnutls-dev libjansson-dev libmxml-dev`

If you want to build the documentation (Notice that plantuml may 
require a running X server):

`sudo apt-get install doxygen graphviz plantuml`

To build the LaTeX/PDF version of the documentation:

`sudo apt-get install doxygen-latex`

## Checkout

First we clone the repository. Shown URL is for the official GitHub
project, you need to change this to use one of the unofficial/personal
forks of the project.

To get a single branch:

`git clone https://github.com/home-port/HomePort.git HomePort.git -b HomePortFunctionAPI`

`cd HomePort.git`

`git submodule init`

`git submodule update`

## Configure

`mkdir build && cd build`

Normal builds:

`cmake ../`

Debug builds:

`cmake -DCMAKE_BUILD_TYPE=Debug ../`

## Compile and install HPD and modules

`make`

`sudo make install`

## Compile documentation

For regular HTML Doxygen documentation:

`make doc-html`

For LaTeX version:

`make doc-latex`

For PDF version:

`make doc-pdf`

For all:

`make doc-all`
