# OATPP Framework Based Backend Server

A high-performance IoT sensor data collection backend server built with the oatpp web framework. The server provides WebSocket APIs for real-time data streaming and can handle numerous simultaneous connections with asynchronous processing.

## General Overview

Backend server for collecting data from different sensors and making them available through WebSocket APIs. Built with oatpp (Open Athletic Training Platform Plus) web framework using async APIs, the server can handle a large number of simultaneous connections efficiently.

Additional Resources:
- [Oatpp Website](https://oatpp.io/)
- [Oatpp GitHub Repository](https://github.com/oatpp/oatpp)
- [Getting Started with Oatpp](https://oatpp.io/docs/start)

## Installation and Setup for Ubuntu

### System Requirements

The following instructions are tested on Ubuntu 20.04 LTS and later. For other Ubuntu versions, package names may vary slightly.

### Prerequisites

Before building the backend server on Ubuntu, ensure all required packages are installed.

#### Step 1: Update System Packages

```bash
sudo apt-get update
sudo apt-get upgrade -y
```

#### Step 2: Install Build Tools and Dependencies

Install the required development tools and libraries:

```bash
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libbluetooth-dev \
    libglib2.0-dev \
    libdbus-1-dev
```

Package descriptions:
- `build-essential` - Compiler and build tools (gcc, g++, make)
- `cmake` - Build system generator
- `git` - Version control system
- `pkg-config` - Package configuration utility
- `libbluetooth-dev` - Bluetooth development libraries
- `libglib2.0-dev` - GLib development libraries (required for D-Bus)
- `libdbus-1-dev` - D-Bus system message bus development libraries

### Installation Steps

#### Step 1: Clone the Repository with Submodules

Clone the backend-IOT repository and initialize git submodules:

```bash
cd ~/projects
git clone https://github.com/your-username/backend-IOT.git
cd backend-IOT/backend-server
git submodule update --init --recursive
```

Verify the submodules are properly initialized:

```bash
git submodule status
```

Expected output should show:
```
 <hash> lib/oatpp (HEAD detached at <commit>)
 <hash> lib/oatpp-websocket (HEAD detached at <commit>)
```

#### Step 2: Build and Install Dependencies

The backend server depends on two oatpp libraries: `oatpp` and `oatpp-websocket`. These are included as git submodules.

##### Option A: Automated Installation (Recommended)

Use the provided installation script to build and install dependencies:

```bash
cd ~/projects/backend-IOT/backend-server
bash utility/install-oatpp-modules.sh
```

The script will:
1. Navigate to the oatpp submodule directory
2. Create a build directory and configure the project with CMake
3. Compile the oatpp library using all available CPU cores
4. Install oatpp to the system (/usr/local)
5. Repeat the same process for oatpp-websocket
6. Provide status messages throughout the process

This may take 5-15 minutes depending on your system specifications.

##### Option B: Manual Installation

If you prefer to build and install dependencies manually:

**Build oatpp:**

```bash
cd ~/projects/backend-IOT/backend-server/lib/oatpp
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
cd ~/projects/backend-IOT/backend-server
```

**Build oatpp-websocket:**

```bash
cd ~/projects/backend-IOT/backend-server/lib/oatpp-websocket
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
cd ~/projects/backend-IOT/backend-server
```

The libraries will be installed to `/usr/local/` and CMake configuration files will be placed in `/usr/local/lib/cmake/`.

#### Step 3: Configure and Build the Backend Server

Navigate to the backend-server directory and create a clean build:

```bash
cd ~/projects/backend-IOT/backend-server
rm -rf build
mkdir build && cd build
cmake ..
```

Expected CMake output should include:
```
-- Found oatpp
-- Found oatpp-websocket
-- Found bluez
-- Found glib-2.0
-- Found dbus-1
-- Configuring done
```

Compile the project:

```bash
make -j$(nproc)
```

The build process will create:
- `backend-server-exe` - Main application executable (approximately 20 MB)
- `backend-server-test` - Test suite executable (approximately 3 MB)

Build time typically ranges from 2-5 minutes on a modern system.

#### Step 4: Verify the Build

Verify that the executables were created successfully:

```bash
ls -lah backend-server-exe backend-server-test
```

Expected output:
```
-rwxrwxr-x 1 user user 20M Jan 14 21:46 backend-server-exe
-rwxrwxr-x 1 user user 3.0M Jan 14 21:46 backend-server-test
```

#### Step 5: Run the Server

Start the backend server:

```bash
./backend-server-exe
```

The server will initialize and listen for WebSocket connections. Default configuration:
- Listen address: `0.0.0.0`
- Port: `8000`
- WebSocket endpoint: `ws://localhost:8000/`

To run the test suite:

```bash
./backend-server-test
```

## Cross-Compilation for IMX7 (Yocto)

This section describes how to cross-compile the backend server for Yocto-based IMX7 targets from an Ubuntu host machine.

### Cross-Compilation Prerequisites

Ensure the following are available on your host system:

1. Yocto SDK for the target platform installed
2. The toolchain file at `*/toolchain.cmake` (or equivalent path)
3. Cross-compilation tools installed in the SDK

#### Setup Environment Variables

Source the Yocto environment setup script to configure cross-compilation settings:

```bash
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi
```

This sets up critical environment variables including:
- `CC` - C compiler
- `CXX` - C++ compiler
- `CFLAGS` - Compiler flags
- `LDFLAGS` - Linker flags
- `PKG_CONFIG_PATH` - Package configuration path

### Cross-Compilation Steps

#### Step 1: Build oatpp for Target

Navigate to the oatpp submodule and configure for cross-compilation:

```bash
cd ~/projects/backend-IOT/backend-server/lib/oatpp
mkdir build-target && cd build-target

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=~/projects/backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"

make -j$(nproc)
sudo make install
```

Parameters explained:
- `-DCMAKE_TOOLCHAIN_FILE` - Path to the cross-compilation toolchain configuration
- `-DCMAKE_INSTALL_PREFIX` - Installation path in the Yocto sysroot
- `-DCMAKE_BUILD_TYPE=Release` - Optimize for production use
- `-DCMAKE_CXX_FLAGS_RELEASE="-O2"` - Enable optimization level 2
- `-DCMAKE_C_FLAGS_RELEASE="-O2"` - C compiler optimization

#### Step 2: Build oatpp-websocket for Target

Configure and build the websocket library with the same cross-compilation settings:

```bash
cd ~/projects/backend-IOT/backend-server/lib/oatpp-websocket
mkdir build-target && cd build-target

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=~/projects/backend-IOT/backend-server/toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2" \
  -DCMAKE_C_FLAGS_RELEASE="-O2"

make -j$(nproc)
sudo make install
```

#### Step 3: Configure pkg-config for Cross-Compilation

Set up environment variables for pkg-config to find cross-compiled libraries:

```bash
export PKG_CONFIG_SYSROOT_DIR=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi

export PKG_CONFIG_PATH=$PKG_CONFIG_SYSROOT_DIR/usr/lib/pkgconfig:$PKG_CONFIG_SYSROOT_DIR/usr/share/pkgconfig

export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH

export PKG_CONFIG=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/x86_64-pokysdk-linux/usr/bin/pkg-config
```

These can be added to your shell profile (`.bashrc`, `.zshrc`) for persistence:

```bash
cat >> ~/.bashrc << 'EOF'
# Cross-compilation environment for IMX7
export PKG_CONFIG_SYSROOT_DIR=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi
export PKG_CONFIG_PATH=$PKG_CONFIG_SYSROOT_DIR/usr/lib/pkgconfig:$PKG_CONFIG_SYSROOT_DIR/usr/share/pkgconfig
export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
export PKG_CONFIG=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/x86_64-pokysdk-linux/usr/bin/pkg-config
EOF

source ~/.bashrc
```

#### Step 4: Build Backend Server for Target

Build the backend server with cross-compilation settings:

```bash
cd ~/projects/backend-IOT/backend-server
rm -rf build-target
mkdir build-target && cd build-target

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/cortexa7t2hf-neon-poky-linux-gnueabi/usr \
  -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

The cross-compiled binary is now available at:
```
~/projects/backend-IOT/backend-server/build-target/backend-server-exe
```

#### Step 5: Deploy to Target

Transfer the compiled executable to your IMX7 target device:

```bash
scp build-target/backend-server-exe root@<target-ip>:/usr/bin/
```

SSH into the target and set executable permissions:

```bash
ssh root@<target-ip>
chmod +x /usr/bin/backend-server-exe
/usr/bin/backend-server-exe
```

## Project Structure

```
backend-server/
|
|- CMakeLists.txt                           # Project CMake configuration
|- README.md                                 # This file
|- toolchain.cmake                          # Cross-compilation toolchain configuration
|
|- lib/                                     # Git submodules for dependencies
|    |- oatpp/                              # Oatpp web framework (v1.4.0)
|    |   |- src/                            # Framework source code
|    |   |- build/                          # Build directory (auto-generated)
|    |   |- CMakeLists.txt
|    |
|    |- oatpp-websocket/                    # Oatpp WebSocket module (v1.4.0)
|        |- src/                            # WebSocket implementation
|        |- build/                          # Build directory (auto-generated)
|        |- CMakeLists.txt
|
|- src/                                     # Backend server source code
|    |- App.cpp                             # Application entry point (main function)
|    |- AppComponent.hpp                    # Application configuration and setup
|    |- CorsInterceptor.hpp                 # CORS (Cross-Origin Resource Sharing) interceptor
|    |
|    |- config/
|    |   |- Constants.hpp                   # Application constants and configurations
|    |
|    |- controller/
|    |   |- GraphController.hpp             # Graph data API endpoints
|    |   |- RoomsController.hpp             # Room management WebSocket endpoints
|    |   |- ServiceController.hpp           # Service management endpoints
|    |   |- UserController.hpp              # User management endpoints
|    |
|    |- dto/                                # Data Transfer Objects
|    |   |- GraphDto.hpp                    # Graph data structure
|    |   |- UserDto.hpp                     # User data structure
|    |
|    |- model/                              # Business logic models
|    |   |- Lobby.cpp                       # Lobby implementation (manages peers and room assignment)
|    |   |- Lobby.hpp                       # Lobby class definition
|    |   |- Room.cpp                        # Room implementation (manages chat room state)
|    |   |- Room.hpp                        # Room class definition
|    |
|    |- service/                            # Service layer
|    |   |- AuthService.cpp                 # Authentication service implementation
|    |   |- AuthService.hpp                 # Authentication class definition
|    |   |- IService.hpp                    # Service interface (base class)
|    |   |- MqttService.hpp                 # MQTT protocol service
|    |   |- ServiceManager.cpp              # Service management implementation
|    |   |- ServiceManager.hpp              # Service manager class definition
|    |   |- ServiceRegistry.hpp             # Service registry pattern implementation
|    |   |
|    |   |- ble/                            # Bluetooth Low Energy services
|    |       |- BleService.cpp              # BLE service implementation
|    |       |- BleService.hpp              # BLE service class definition
|    |
|    |- utils/                              # Utility functions
|    |   |- Crypto.hpp                      # Cryptographic utility functions
|    |   |- JwtUtils.hpp                    # JSON Web Token utility functions
|    |
|    |- websocket/                          # WebSocket server implementation
|        |- BaseSocketListener.cpp          # Base WebSocket listener implementation
|        |- BaseSocketListener.hpp          # Base listener class definition
|        |- GraphListener.cpp               # Graph data WebSocket listener implementation
|        |- GraphListener.hpp               # Graph listener class definition
|        |- Peer.cpp                        # Peer connection implementation
|        |- Peer.hpp                        # Peer class definition
|
|- test/                                    # Test suite
|    |- tests.cpp                           # Main test file
|    |- WSTest.cpp                          # WebSocket tests implementation
|    |- WSTest.hpp                          # WebSocket tests class definition
|
|- utility/                                 # Utility scripts
|    |- install-oatpp-modules.sh            # Script to build and install oatpp dependencies
|
|- build/                                   # Build directory (auto-generated)
|   |- backend-server-exe                   # Main executable
|   |- backend-server-test                  # Test executable
|   |- CMakeCache.txt
|   |- CMakeFiles/
|   |- ...
|
|- build-target/                            # Cross-compilation build directory (auto-generated)
    |- backend-server-exe                   # Cross-compiled executable for target platform
```

## Architecture Overview

The backend server follows a layered architecture:

1. **Controller Layer** - Handles HTTP/WebSocket API endpoints and request routing
2. **Service Layer** - Implements business logic (Authentication, BLE, MQTT, etc.)
3. **Model Layer** - Manages application state (Rooms, Lobbies, Users)
4. **Data Transfer Objects** - Standardizes data exchange between layers
5. **WebSocket Listeners** - Manages real-time peer connections
6. **Utilities** - Provides cryptographic and JWT token handling

## BLE Service Refactoring Progress

The BLE service is undergoing systematic refactoring to improve code organization and maintainability. The monolithic 1,267-line BleService is being decomposed into focused, reusable managers.

### Completed Phases

[PHASE 1] EventLoopManager and BlueZAdapter
- EventLoopManager (157 lines) - Manages GMainLoop workers for async D-Bus operations
- BlueZAdapter (181 lines) - Encapsulates BlueZ adapter communication
- BleSimulation refactored (146 lines, reduced 61%)
- Status: COMPLETE - All tests pass

[PHASE 2] BleDeviceManager
- BleDeviceManager (425 lines) - Device discovery, connection, pairing, trust management
- Extracted 250+ lines of device operations from BleService
- Status: COMPLETE - Build verified [100%]

[PHASE 3] BleCharacteristicManager
- BleCharacteristicManager (415 lines) - Service/characteristic discovery and GATT operations
- Extracted 150+ lines of characteristic operations from BleService
- Includes UUID mapping and notification management
- Status: COMPLETE - All tests pass, build verified [100%]

[PHASE 4] BleDataProcessor
- BleDataProcessor (383 lines) - Graph streaming and sensor data processing
- Extracted 250+ lines of data processing operations from BleService
- Dual-mode operation (simulation/real sensor data)
- FPS regulation and WebSocket client management
- Status: COMPLETE - All tests pass, build verified [100%]

### Upcoming Phases

[PHASE 5] BleService Simplification - Reduce to ~100 line facade

For detailed refactoring progress, see [PHASE_4_SUMMARY.md](docs/PHASE_4_SUMMARY.md)



