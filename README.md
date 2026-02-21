# Backend-IOT

A high-performance IoT sensor data collection backend server built with the Oat++ web framework. This project provides WebSocket APIs for real-time data streaming from Bluetooth Low Energy (BLE) sensors, enabling efficient handling of numerous simultaneous connections through asynchronous processing.

## Features

- **Real-time Data Streaming**: WebSocket-based APIs for live sensor data transmission
- **BLE Integration**: Native Bluetooth Low Energy support for sensor communication
- **High Performance**: Asynchronous processing with Oat++ framework for scalable connections
- **Cross-Platform**: Supports Linux environments with cross-compilation for embedded targets
- **Modular Architecture**: Clean separation of concerns with controllers, services, and models
- **Authentication**: JWT-based user authentication system
- **MQTT Support**: Integration with MQTT protocol for IoT messaging

## Prerequisites

- Ubuntu 20.04 LTS or later
- CMake 3.10+
- GCC/G++ compiler
- Bluetooth development libraries
- Git

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/backend-IOT.git
cd backend-IOT
git submodule update --init --recursive
```

### 2. Install System Dependencies

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

### 3. Build Dependencies

Navigate to the backend-server directory and run the automated installation script:

```bash
cd backend-server
bash utility/install-oatpp-modules.sh
```

This script builds and installs the required Oat++ libraries (oatpp and oatpp-websocket).

## Building the Project

### Native Build

```bash
cd backend-server
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Cross-Compilation (for IMX7/Yocto targets)

1. Source your Yocto SDK environment:
```bash
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi
```

2. Build dependencies for target:
```bash
# Build oatpp
cd lib/oatpp
mkdir build-target && cd build-target
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../toolchain.cmake -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc) && sudo make install

# Build oatpp-websocket
cd ../../lib/oatpp-websocket
mkdir build-target && cd build-target
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../toolchain.cmake -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc) && sudo make install
```

3. Build the backend server:
```bash
cd ../../
mkdir build-target && cd build-target
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Running the Server

After building, start the server:

```bash
./backend-server-exe
```

The server will listen on `0.0.0.0:8000` with WebSocket endpoint at `ws://localhost:8000/`.

### Running Tests

```bash
./backend-server-test
```

## Project Structure

```
backend-IOT/
├── backend-server/                 # Main server application
│   ├── CMakeLists.txt             # Build configuration
│   ├── toolchain.cmake            # Cross-compilation setup
│   ├── src/                       # Source code
│   │   ├── App.cpp                # Application entry point
│   │   ├── controller/            # API endpoint controllers
│   │   ├── service/               # Business logic services
│   │   │   └── ble/               # BLE service implementations
│   │   ├── model/                 # Data models
│   │   ├── dto/                   # Data transfer objects
│   │   ├── websocket/             # WebSocket handlers
│   │   └── utils/                 # Utility functions
│   ├── test/                      # Test suite
│   ├── lib/                       # Dependencies (oatpp, oatpp-websocket)
│   └── utility/                   # Build scripts
├── config.json                    # Configuration file
├── LICENSE                        # License information
└── README.md                      # This file
```

## Architecture

The backend server follows a layered architecture:

1. **Controller Layer**: Handles HTTP/WebSocket API endpoints and request routing
2. **Service Layer**: Implements business logic (Authentication, BLE communication, MQTT)
3. **Model Layer**: Manages application state (Rooms, Lobbies, Users)
4. **DTO Layer**: Standardizes data exchange between layers
5. **WebSocket Layer**: Manages real-time peer connections
6. **Utility Layer**: Provides cryptographic and JWT token handling

## API Endpoints

- **WebSocket**: `ws://localhost:8000/` - Real-time data streaming
- **Graph API**: REST endpoints for graph data management
- **User Management**: Authentication and user-related operations
- **Room Management**: WebSocket-based room and peer management

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the MIT License. Parts of this project are derived from Oat++ (Apache License 2.0). See LICENSE for details.

## Resources

- [Oat++ Website](https://oatpp.io/)
- [Oat++ GitHub](https://github.com/oatpp/oatpp)
- [Oat++ Documentation](https://oatpp.io/docs/start)