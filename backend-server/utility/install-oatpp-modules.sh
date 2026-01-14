#!/bin/bash

# OATPP Modules Installation Script
# This script builds and installs the oatpp and oatpp-websocket libraries
# from local git submodules in the lib/ directory

set -e

echo "Building and installing OATPP modules..."

# Get the absolute path to the backend-server directory
BACKEND_SERVER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

##########################################################
## Build and install oatpp

echo "Building oatpp..."
cd "$BACKEND_SERVER_DIR/lib/oatpp"

if [ ! -d build ]; then
    mkdir build
fi

cd build
cmake ..
make -j$(nproc)
echo "Installing oatpp..."
sudo make install

cd "$BACKEND_SERVER_DIR"

##########################################################
## Build and install oatpp-websocket

echo "Building oatpp-websocket..."
cd "$BACKEND_SERVER_DIR/lib/oatpp-websocket"

if [ ! -d build ]; then
    mkdir build
fi

cd build
cmake ..
make -j$(nproc)
echo "Installing oatpp-websocket..."
sudo make install

cd "$BACKEND_SERVER_DIR"

##########################################################

echo "OATPP modules installation completed successfully."
echo "You can now build the backend server:"
echo "  cd build"
echo "  cmake .."
echo "  make"
