# Oat++ Framework Based Backend Server

A high-performance IoT sensor data collection backend server built with the Oat++ web framework. This server provides WebSocket APIs for real-time data streaming from Bluetooth Low Energy (BLE) sensors, enabling efficient handling of numerous simultaneous connections through asynchronous processing.

## Prerequisites

- Ubuntu 20.04 LTS or later
- CMake 3.10+
- GCC/G++ compiler
- Git
- For BLE support: `libbluetooth-dev`, `libglib2.0-dev`, `libdbus-1-dev`

### Install System Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libbluetooth-dev \
    libglib2.0-dev \
    libdbus-1-dev
```

## Oat++ Library Installation

### Install Oat++

First, clone and build the Oat++ framework:

```bash
git clone https://github.com/oatpp/oatpp.git
cd oatpp
git checkout v1.4.0
mkdir build && cd build
```

**For Ubuntu native build:**
```bash
cmake ..
make -j$(nproc)
sudo make install
```

**For cross-compilation** (replace `/path/to/backend-IOT` with actual path):
```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"
make -j$(nproc)
sudo make install
```

### Install Oat++-WebSocket

After installing Oat++, install the WebSocket module:

```bash
git clone https://github.com/oatpp/oatpp-websocket.git
cd oatpp-websocket
git checkout v1.4.0  # Use the same version as oatpp
mkdir build && cd build
```

**For Ubuntu native build:**
```bash
cmake ..
make -j$(nproc)
sudo make install
```

**For cross-compilation** (replace `/path/to/backend-IOT` with actual path):
```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"
make -j$(nproc)
sudo make install
```

**Alternatively**, use the provided script to install both Oat++ and Oat++-WebSocket automatically:
```bash
cd /path/to/backend-IOT/backend-server
bash utility/install-oatpp-modules.sh
```

## Build and Run on Linux

### Using CMake

**Requirements:** Oat++ and Oat++-WebSocket modules must be installed (see above).

Navigate to the backend-server directory and build:

```bash
cd backend-IOT/backend-server
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run the server:
```bash
./backend-server-exe
```

The server will listen on `0.0.0.0:8000` with WebSocket endpoint at `ws://localhost:8000/`.

## Cross-Compilation for IMX7

This section describes cross-compiling the backend server for Yocto-based IMX7 targets from an Ubuntu host.

### Prerequisites for Cross-Compilation

1. Yocto SDK installed on the host system
2. Source the Yocto environment:
   ```bash
   source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi
   ```
3. Ensure Oat++ and Oat++-WebSocket are built for the target (see installation sections above)

### Building Oat++-WebSocket for Target

```bash
git clone https://github.com/oatpp/oatpp-websocket.git
cd oatpp-websocket
git checkout v1.4.0

mkdir build-target && cd build-target

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"

make -j$(nproc)
sudo make install
```

### Setting up pkg-config for Cross-Compilation

Configure pkg-config to find cross-compiled libraries:

```bash
export PKG_CONFIG_SYSROOT_DIR=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi
export PKG_CONFIG_PATH=$PKG_CONFIG_SYSROOT_DIR/usr/lib/pkgconfig:$PKG_CONFIG_SYSROOT_DIR/usr/share/pkgconfig
export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
export PKG_CONFIG=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/x86_64-pokysdk-linux/usr/bin/pkg-config
```

### Building the Backend Server for Target

```bash
cd /path/to/backend-IOT/backend-server
mkdir build-target && cd build-target

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

The cross-compiled executable `backend-server-exe` will be available in `build-target/`.

### Deploying to Target

Transfer the executable to your IMX7 device:

```bash
scp build-target/backend-server-exe root@<target-ip>:/usr/bin/
```

On the target device:
```bash
ssh root@<target-ip>
chmod +x /usr/bin/backend-server-exe
/usr/bin/backend-server-exe
```

## General IoT Sensor Backend

Backend server for collecting data from different sensors and making them available through WebSocket APIs. Built with Oat++ (Open Athletic Training Platform Plus) web framework using asynchronous APIs. The server can handle a large number of simultaneous connections efficiently.

### Resources

- [Oat++ Website](https://oatpp.io/)
- [Oat++ GitHub Repository](https://github.com/oatpp/oatpp)
- [Getting Started with Oat++](https://oatpp.io/docs/start)

## Overview

The server provides WebSocket endpoints for real-time sensor data streaming. Users can connect to `ws://localhost:8000/` to receive live data from BLE sensors.

The server supports:
- Real-time data collection from BLE sensors
- Asynchronous processing for high concurrency
- Cross-platform deployment (Linux native and embedded targets)

## Project Structure

```
backend-IOT/
├── backend-server/
│   ├── CMakeLists.txt                         // Project CMake configuration
│   ├── src/
│   │   ├── App.cpp                            // Application entry point
│   │   ├── AppComponent.hpp                   // Application configuration
│   │   ├── controller/
│   │   │   ├── BackendController.hpp          // Main backend controller
│   │   │   ├── BleController.hpp              // BLE service endpoints
│   │   │   ├── GraphController.hpp            // Graph data API
│   │   │   ├── RoomsController.hpp            // Room management WebSocket
│   │   │   ├── ServiceController.hpp          // Service management
│   │   │   ├── SettingsController.hpp         // Settings management
│   │   │   └── UserController.hpp             // User management
│   │   ├── dto/
│   │   │   ├── BleDtos.hpp                    // BLE data structures
│   │   │   ├── GraphDto.hpp                   // Graph data structures
│   │   │   └── UserDto.hpp                    // User data structures
│   │   ├── model/
│   │   │   ├── Lobby.cpp                      // Lobby management
│   │   │   ├── Lobby.hpp                      // Lobby class
│   │   │   ├── Room.cpp                       // Room implementation
│   │   │   └── Room.hpp                       // Room class
│   │   ├── service/
│   │   │   ├── AuthService.cpp                // Authentication service
│   │   │   ├── AuthService.hpp                // Authentication class
│   │   │   ├── IService.hpp                   // Service interface
│   │   │   ├── MqttService.hpp                // MQTT service (future)
│   │   │   ├── ServiceManager.cpp             // Service management
│   │   │   ├── ServiceManager.hpp             // Service manager
│   │   │   ├── ServiceRegistry.hpp            // Service registry
│   │   │   └── ble/
│   │   │       ├── BleService.cpp             // BLE service implementation
│   │   │       └── BleService.hpp             // BLE service class
│   │   ├── utils/
│   │   │   ├── Crypto.hpp                     // Cryptographic utilities
│   │   │   └── JwtUtils.hpp                   // JWT token utilities
│   │   └── websocket/
│   │       ├── BaseSocketListener.cpp         // Base WebSocket listener
│   │       ├── BaseSocketListener.hpp         // Base listener class
│   │       ├── GraphListener.cpp              // Graph data listener
│   │       ├── GraphListener.hpp              // Graph listener class
│   │       ├── GraphWebSocket.cpp             // Graph WebSocket implementation
│   │       ├── GraphWebSocket.hpp             // Graph WebSocket class
│   │       ├── Peer.cpp                       // Peer connection
│   │       ├── Peer.hpp                       // Peer class
│   │       ├── WSComm.cpp                     // WebSocket communication
│   │       └── WSComm.hpp                     // WebSocket communication class
│   ├── test/
│   │   ├── tests.cpp                          // Test suite
│   │   ├── WSTest.cpp                         // WebSocket tests
│   │   └── WSTest.hpp                         // WebSocket test class
│   ├── lib/
│   │   ├── oatpp/                             // Oatpp framework submodule
│   │   └── oatpp-websocket/                   // Oatpp WebSocket submodule
│   └── utility/
│       └── install-oatpp-modules.sh           // Script to install oatpp modules
├── config.json                                // Configuration file
├── LICENSE                                    // License information
├── README.md                                  // This file
└── Third-party-notice.txt                     // Third-party notices
```

## Future Features

The following features are planned for future development:

### Data Structures (DTOs)
- **GraphDto.hpp**: Data structures for graph data serialization and deserialization
- **UserDto.hpp**: Data structures for user information and authentication

### Utilities
- **Crypto.hpp**: Cryptographic utilities for secure data handling
- **JwtUtils.hpp**: JWT (JSON Web Token) utilities for authentication and authorization

### Services
- **MQTT Integration**: Full MQTT protocol support for IoT messaging (currently has placeholder implementation)

### Testing
- **WebSocket Tests**: Comprehensive test suite for WebSocket functionality (currently minimal)

## License

Parts of this project are derived from Oat++ (Apache License 2.0).

## Third-Party Components

This project uses Oat++ - see LICENSE and Third-party-notice.txt.
