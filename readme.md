# Object / Mesh Viewer

The following repository contains the implementation of a basic mesh/object viewer for `.obj` files. The project has been implemented in C++ using OpenGL and GLSL.

## Usage

To compile, create a directory called `build` and create Makefile using the cmake build system

    mkdir build && cd build
    cmake ..
    make

Once the project is compiled, the binary is directed to a `bin` directory, and takes the `.obj` mesh as a command line argument

    ./bin/viewer objects/bunny.obj

