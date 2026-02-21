# Backend-IOT

IoT Backend Server built with oatpp framework for real-time sensor data collection and WebSocket communication.

## Overview

This repository contains a high-performance backend server for IoT applications, built using the [oatpp web framework](https://oatpp.io/). The server provides:

- WebSocket APIs for real-time data streaming
- Bluetooth Low Energy (BLE) service integration
- MQTT protocol support
- Asynchronous processing for handling multiple simultaneous connections
- Cross-platform support (Ubuntu native and cross-compilation for IMX7/Yocto)

## Quick Start

### Prerequisites

For Ubuntu 20.04 LTS and later:

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

### Clone and Build

1. Clone the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/isarwar93/backend-IOT.git
cd backend-IOT/backend-server
```

2. Install oatpp dependencies:

```bash
bash utility/install-oatpp-modules.sh
```

3. Build the backend server:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

4. Run the server:

```bash
./backend-server-exe
```

The server will start on port 8000 by default.

## Documentation

For detailed installation instructions, architecture overview, and cross-compilation guide, see the [backend-server README](backend-server/README.md).

## Project Structure

```
backend-IOT/
├── backend-server/        # Main server application
│   ├── src/              # Source code
│   ├── test/             # Test suite
│   ├── lib/              # Git submodules (oatpp, oatpp-websocket)
│   └── utility/          # Build and installation scripts
├── config.json           # Configuration file
└── README.md            # This file
```

## License

See [LICENSE.txt](LICENSE.txt) for details.

## Third-Party Components

This project uses oatpp - see [LICENSE-oatpp](LICENSE-oatpp) and [Third-party-notice.txt](Third-party-notice.txt).
