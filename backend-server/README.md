# OATPP Framework based backend server

## OATPP lib install

First install oatpp library
if cmake not installed, first install cmake and then go the next step:

```
sudo apt update
sudo apt install -y cmake
```

```
git clone https://github.com/oatpp/oatpp.git
```
Then go master branch for v1.4.0
```
git checkout v1.4.0
```
```
cd oatpp
mkdir build && cd build
-> for Ubuntu without the need of cross compile
cmake ..   
```
-> For cross compiling
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../../backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"

sudo make install
```


## Build and Run from linux:

### Using CMake

**Requires:** [oatpp](https://github.com/oatpp/oatpp), and [oatpp-websocket](https://github.com/oatpp/oatpp-websocket) 
modules installed. You may run `utility/install-oatpp-modules.sh` 
script to install required oatpp modules.

After all dependencies satisfied:

```
$ mkdir build && cd build
$ cmake ..
$ make 
$ ./backend-server      # - run application.
```

## For cross compilation of Backend server for IMX7:

Following way is used to cross-compile the backend server using Ubuntu based host computer, for Yocto based IMX7 module:


```
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi 

git clone https://github.com/oatpp/oatpp-websocket.git
cd oatpp-websocket


cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../../backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"
make -j$(nproc)

make install
```

### For pkg config path set

export PKG_CONFIG_SYSROOT_DIR=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi

export PKG_CONFIG_PATH=$PKG_CONFIG_SYSROOT_DIR/usr/lib/pkgconfig:$PKG_CONFIG_SYSROOT_DIR/usr/share/pkgconfig

export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH

export PKG_CONFIG=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/x86_64-pokysdk-linux/usr/bin/pkg-config



## General fast backend for IOT sensors

Backend server to collect data from different sensors and make them available on WebSocket APIs. Built with oat++ (AKA oatpp) web framework using Async API. Server can handle a large number of simultaneous connections.

See more:

- [Oat++ Website](https://oatpp.io/)
- [Oat++ Github Repository](https://github.com/oatpp/oatpp)
- [Get Started With Oat++](https://oatpp.io/docs/start)

## Overview

User connectes on a websocket endpoint `ws://localhost:8000/ws/chat/{room_name}/?nickname={nickname}` and
enters the chat room `room_name`. User will appear in the chat as a `nickname`.  

If user is the first user in the room,- new room will be automatically created.

Each message that user send to the chat room will be delivered to every participant of the room.

### Project layout

```
|- CMakeLists.txt                         // projects CMakeLists.txt
|- src/
|    |
|    |- controller/
|    |    |
|    |    |- RoomsController.hpp          // ApiController with websocket endpoint
|    |
|    |- rooms/
|    |    |
|    |    |- Lobby.hpp                    // Class for managing new peers and assigning them to rooms
|    |    |- Peer.hpp                     // Class representing one peer of a chat
|    |    |- Room.hpp                     // Class representing one chat room
|    |
|    |- AppComponent.hpp                  // Application config. 
|    |- App.cpp                           // main() is here
|
|- utility/install-oatpp-modules.sh       // utility script to install required oatpp-modules.  
```

