# BLE Service Refactoring - Architecture

## Current Structure

The BLE service has been refactored from a monolithic `BleService` class (1266 lines in .cpp, 214 lines in .hpp) into separate, focused classes following Single Responsibility Principle.

## New Architecture

### 1. **EventLoopManager** (Async Event Handling)
- **Files**: `EventLoopManager.hpp`, `EventLoopManager.cpp`
- **Responsibility**: Manages GMainLoop workers for handling asynchronous BLE operations
- **Key Methods**:
  - `startLoopFor(key)` - Start a new event loop thread
  - `stopLoopFor(key)` - Stop a specific event loop
  - `stopAllLoops()` - Clean up all loops
  - `hasLoop()`, `loopCount()` - Query loop status

### 2. **BlueZAdapter** (D-Bus Communication)
- **Files**: `BlueZAdapter.hpp`, `BlueZAdapter.cpp`
- **Responsibility**: Manages BlueZ adapter operations via D-Bus
- **Key Methods**:
  - `initialize()` - Setup adapter connection
  - `findAdapter()` - Locate available adapter
  - `startDiscovery()` - Begin device scan
  - `stopDiscovery()` - End device scan
  - `getConnection()`, `getAdapterPath()` - Access adapter info

### 3. **BleSimulation** (Sensor Simulation) ✓ [COMPLETED]
- **Files**: `BleSimulation.hpp`, `BleSimulation.cpp`
- **Responsibility**: Generate simulated BLE sensor data
- **Key Methods**:
  - `generateSimulationData()` - Create realistic sensor readings
  - `enableSimulation()`, `disableSimulation()` - Control simulation mode

### 4. **Future Classes** (To be created)

#### **BleDeviceManager** (Device Operations)
- Device scanning, pairing, connection management
- Methods: `scanDevices()`, `connectToDevice()`, `pairDevice()`, `trustDevice()`, etc.

#### **BleCharacteristicManager** (Characteristic Operations)
- Characteristic discovery, notification handling
- Methods: `getServicesAndCharacteristics()`, `enableNotification()`, `readCharacteristics()`, etc.

#### **BleDataProcessor** (Data Processing & Streaming)
- Process incoming BLE data, graph streaming
- Methods: `processData()`, `streamGraph()`, handle WebSocket communication

## File Organization

```
src/service/ble/
├── BleService.hpp           (Facade/main interface)
├── BleService.cpp           (Facade implementation)
├── EventLoopManager.hpp      (Async handling)
├── EventLoopManager.cpp
├── BlueZAdapter.hpp          (D-Bus adapter)
├── BlueZAdapter.cpp
├── BleSimulation.hpp         (Sensor simulation)
├── BleSimulation.cpp
├── BleFrameBuilder.hpp       (Data frame building)
├── CharRegistry.hpp          (Characteristic registry)
├── ble-readme.md            (Documentation)
└── [Future additions]
    ├── BleDeviceManager.hpp/cpp
    ├── BleCharacteristicManager.hpp/cpp
    └── BleDataProcessor.hpp/cpp
```

## Design Principles Applied

1. **Single Responsibility**: Each class has one reason to change
2. **Facade Pattern**: BleService acts as a unified interface
3. **Separation of Concerns**: D-Bus, async loops, data processing isolated
4. **Testability**: Smaller classes are easier to test
5. **Maintainability**: Clear boundaries and reduced complexity per file

## Benefits

- **Readability**: Files reduced from 1500+ lines to <400 lines each
- **Maintainability**: Each concern is isolated
- **Reusability**: Classes can be used independently
- **Testability**: Smaller units are easier to unit test
- **Scalability**: Easy to add new features without cluttering existing code
