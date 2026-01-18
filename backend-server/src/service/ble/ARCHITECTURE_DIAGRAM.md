# BLE Service Architecture - Visual Overview

## Component Interaction Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                          BleService (Facade)                        │
│                     Main API Entry Point                            │
│  start() | stop() | configure() | sendCommand() | getStatusJson()   │
│                                                                     │
│  Device Ops | Characteristic Ops | Graph Streaming | Simulation   │
└──────────┬──────────────────────┬──────────────┬─────────────┬────────┘
           │                      │              │             │
    ┌──────▼────────┐    ┌────────▼──────┐  ┌───▼────────┐  ┌─▼──────────────┐
    │ BlueZAdapter  │    │ EventLoop     │  │Simulation  │  │Device/Char Mgmt│
    │               │    │Manager        │  │            │  │(Future)        │
    │ • Initialize  │    │               │  │ • Simulate │  │                │
    │ • FindAdapter │    │ • startLoop   │  │ • Sensor   │  │ • DeviceManager│
    │ • StartScan   │    │ • stopLoop    │  │   Data     │  │ • CharManager  │
    │ • StopScan    │    │ • stopAll     │  │            │  │ • DataProc     │
    └──────┬────────┘    └────────┬──────┘  └───┬────────┘  └─┬──────────────┘
           │                      │              │             │
           │         ┌────────────▼──────────┐   │             │
           │         │   GLib/GMainLoop      │   │             │
           │         │   Threading & Events  │   │             │
           │         └───────────────────────┘   │             │
           │                                     │             │
           ▼                                     ▼             ▼
    ┌──────────────────────────────────────────────────────────┐
    │              BlueZ D-Bus System Service                  │
    │     (Adapter, Devices, Services, Characteristics)       │
    └──────────────────────────────────────────────────────────┘
```

## Class Dependency Graph

```
BleService (Facade)
├── uses ──────────► BlueZAdapter
│                   ├── calls BlueZ D-Bus API
│                   └── manages GDBusConnection
│
├── uses ──────────► EventLoopManager
│                   ├── creates GMainLoop workers
│                   ├── subscribes to D-Bus signals
│                   └── handles async operations
│
├── uses ──────────► BleSimulation
│                   ├── generates sensor data
│                   └── produces JSON output
│
├── contains ──────► CharRegistry
│                   └── tracks characteristic metadata
│
├── contains ──────► BleFrameBuilder
│                   └── converts binary ↔ sensor values
│
└── uses ──────────► WebSocket APIs (external)
                    └── streams graph data to clients
```

## Data Flow

### Device Discovery Flow
```
User API Call
    │
    ▼
BleService::scanDevices()
    │
    ├─→ BlueZAdapter::startDiscovery()
    │   └─→ D-Bus: Adapter1.StartDiscovery()
    │
    └─→ Get managed objects from BlueZ
        └─→ Parse device list
            └─→ Return BleDeviceInfo[]
```

### Notification Flow
```
User API Call
    │
    ▼
BleService::enableNotification(path)
    │
    ├─→ EventLoopManager::startLoopFor(path)
    │   ├─→ Create GMainContext
    │   ├─→ Create GMainLoop
    │   └─→ Start listener thread
    │
    ├─→ D-Bus: StartNotify() on characteristic
    │
    └─→ Subscribe to PropertiesChanged signals
        │
        ▼ (Signal arrives)
        └─→ onPropertiesChanged() callback
            └─→ processData()
                └─→ Update graph
                    └─→ Broadcast to WebSocket clients
```

### Simulation Flow
```
Graph Streaming Thread
    │
    ├─→ Check if simulation enabled
    │
    ├─→ BleSimulation::generateSimulationData()
    │   ├─→ makeSensorReading() - ECG, HR, RR
    │   ├─→ makeSensorReading() - BP, Temp (every 3s)
    │   └─→ Return JSON with all sensor data
    │
    └─→ Broadcast to WebSocket clients
```

## File Organization by Layer

```
Application Layer
    └── BleService.hpp/cpp (Facade, ~1300 lines → target 100 lines)
            │
            ├── Domain Managers (To be extracted)
            │   ├── BleDeviceManager (Future - ~250 lines)
            │   ├── BleCharacteristicManager (Future - ~200 lines)
            │   └── BleDataProcessor (Future - ~300 lines)
            │
Infrastructure Layer
    ├── BlueZAdapter.hpp/cpp (D-Bus communication - 145 lines) ✅
    ├── EventLoopManager.hpp/cpp (Async handling - 112 lines) ✅
    │
Support/Utility Layer
    ├── BleSimulation.hpp/cpp (Sensor simulation - 98 lines) ✅
    ├── CharRegistry.hpp (Characteristic tracking - 137 lines)
    ├── BleFrameBuilder.hpp (Data conversion - 67 lines)
    │
External Dependencies
    ├── GLib/GObject (Event loops, D-Bus)
    ├── BlueZ (Bluetooth daemon)
    └── oatpp (WebSocket communication)
```

## Phase Progression

```
Phase 1 ✅ (Current)
├── EventLoopManager - Event handling infrastructure
├── BlueZAdapter - D-Bus communication layer
└── BleSimulation - Clean sensor simulation

Phase 2 (Next)
├── BleDeviceManager
│   ├── scanDevices()
│   ├── connectToDevice()
│   ├── pairDevice()
│   └── ...device operations
│
Phase 3 (Then)
├── BleCharacteristicManager
│   ├── getServicesAndCharacteristics()
│   ├── enableNotification()
│   ├── readCharacteristics()
│   └── ...characteristic operations
│
Phase 4 (Finally)
├── BleDataProcessor
│   ├── processData()
│   ├── streamGraph()
│   └── ...data handling
│
Phase 5 (Completion)
└── BleService (Facade)
    └── ~100 lines of delegation only
```

## Technology Stack

```
BLE Service
    │
    ├─ C++20 (Language)
    ├─ GLib/GObject-2.0 (Event handling)
    │   ├─ GMainLoop (Async event processing)
    │   ├─ D-Bus Integration
    │   └─ Thread management
    │
    ├─ BlueZ 5.x (Bluetooth stack)
    │   └─ D-Bus API
    │
    ├─ oatpp (WebSocket server)
    │   └─ Client connections
    │
    └─ nlohmann/json (JSON serialization)
        └─ Data formatting
```

## Performance Characteristics

- **Event Loop**: One GMainLoop per active characteristic notification
- **Threading**: One thread per event loop
- **Memory**: Minimal overhead, efficient resource cleanup
- **Latency**: Sub-millisecond response to D-Bus signals
- **Scalability**: Can handle multiple simultaneous connections
