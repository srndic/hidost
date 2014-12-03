================
HIDOST INSTALL
================

Hidost was developed on 64-bit Ubuntu 12.04 and tested on
Ubuntu 14.04. It consists of source files written in C++11,
Python 2.7 and Java 7 and uses CMake for building.

Required Dependencies
=======================

The following well-known C++ libraries are required (default
versions from Ubuntu 14.04)::

 - CMake, version 2.8
 - a C++ compiler, though only g++ was tested
 - Boost Filesystem
 - Boost Program Options
 - Boost Regex
 - Boost System
 - Boost Thread
 - Poppler

Hidost depends on the Java library
`SWFREtools <https://github.com/sporst/SWFREtools>`_ for SWF reading.
Please build it and install it, i.e., make sure its binaries are
in the system CLASSPATH, before building Hidost.

Hidost also depends on libquickly, a C++ library for multiprocessing.
You can get its source separately.

Building Hidost
====================

Hidost uses the usual CMake procedure for building. Here are the
individual steps (assuming you are in the root of Hidost, i.e.,
where the README file is)::

  mkdir build
  cd build
  cmake ..
  make

After these steps have been run successfully, both the PDF part
(consisting of C++ executables) and the SWF part (consisting of
a Python library and a Java class file) should be built.

Installation
===================



There is no need to install any of the C++ executables.

To install Hidost, cd to the build directory as created in
the build procedure description above and type::

  make install

On Ubuntu, you should use sudo::

  sudo make install
