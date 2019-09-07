# bookmark-tools
> Tools too add bookmarks

This is a tool to add bookmarks to some common e-book files. Currently only PDF format is supported.

## Table of Contents

- [Build](#build)
    - [Windows](#windows)
    - [Linux or Unix](#linux-or-unix)
    - [Build the Qt application](#build-the-qt-application)
- [TODO](#todo)

## Build

To build this project from source, a C++ compiler that support C++17 is required.
If you are not using Visual Studio 2017 or newer version, CMake is also required.
I recommand you use Visual Studio 2017 to build this project if you use windows.

### Windows

Visual Studio 2017 or newer version can open CMake project directly. So you can
simply open the root CMakeLists.txt of this project and build.

### Linux or Unix

```sh
git clone https://github.com/armaab/bookmark-tools.git
mkdir build && cd build
cmake ..
make
```

### Build the Qt application
This project include a command line utility as well as a Qt GUI application.
In order to build the Qt application, you must have Qt5 installed on you machine.
Set the CMake variable ENABLE_QT to TRUE or ON and set Qt5_DIR to where the cmake
modules of your Qt5 installation are located.

## TODO

- [ ] Add support for extracting bookmark information from PDF file.
- [ ] Suport djvu file.
- [ ] Add more usage and development documentations.
