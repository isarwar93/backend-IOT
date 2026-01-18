# BLE Service Module - Complete Documentation

## Folder Contents

### Core Implementation Files

| File | Type | Purpose | Size |
|------|------|---------|------|
| `BleService.hpp` | Header | Main service interface (Facade) | 8.3K |
| `BleService.cpp` | Implementation | Main service logic (1,267 lines) | 42K |
| `BlueZAdapter.hpp` | Header | D-Bus adapter interface | 914B |
| `BlueZAdapter.cpp` | Implementation | D-Bus communication layer | 3.2K |
| `EventLoopManager.hpp` | Header | Event loop management | 1.5K |
| `EventLoopManager.cpp` | Implementation | Async event handling | 3.7K |
| `BleSimulation.hpp` | Header | Sensor simulation (clean) | 1.5K |
| `BleSimulation.cpp` | Implementation | Simulation engine | 3.0K |
| `CharRegistry.hpp` | Header | Characteristic tracking utility | 4.0K |
| `BleFrameBuilder.hpp` | Header | Binary↔Value conversion | 2.1K |

### Documentation Files

| File | Purpose | Audience |
|------|---------|----------|
| **README.md** | This file - Navigation guide | Everyone |
| **PHASE_1_SUMMARY.md** | Phase 1 completion report | Project Managers |
| **ARCHITECTURE.md** | Detailed component breakdown | Developers |
| **ARCHITECTURE_DIAGRAM.md** | Visual diagrams & flows | Architects |
| **REFACTORING_PLAN.md** | Future phases roadmap | Project Leads |

## Quick Start

### Understanding the Structure

1. **Start here**: Read [PHASE_1_SUMMARY.md](PHASE_1_SUMMARY.md) (5 min)
2. **Visual learner**: See [ARCHITECTURE_DIAGRAM.md](ARCHITECTURE_DIAGRAM.md) (10 min)
3. **Deep dive**: Study [ARCHITECTURE.md](ARCHITECTURE.md) (20 min)
4. **Plan ahead**: Review [REFACTORING_PLAN.md](REFACTORING_PLAN.md) (10 min)

### Developer Guide

```
Need to...                          → Read...
──────────────────────────────────────────────────────
Add new BLE feature                 → ARCHITECTURE.md
Understand event handling           → EventLoopManager.hpp
Add device discovery feature        → BlueZAdapter.hpp
Debug simulation data               → BleSimulation.cpp
Extend the service                  → REFACTORING_PLAN.md
```

## 📋 Phase 1 Status - ✅ COMPLETE
Phase 1 Status - COMPLETE

### Completed Components

- **EventLoopManager** (112 lines)
  - Manages GMainLoop workers
  - Thread-safe lifecycle
  - Clean async handling

- **BlueZAdapter** (145 lines)
  - D-Bus communication
  - Adapter discovery
  - Device scan operations

- **BleSimulation** (98 lines - refactored)
  - Sensor simulation
  - Realistic data generation
  - Clean API

### Build Status
```
[OK] Compiles without errors
[OK] All tests pass
[OK] No regressions
[OK]``

## Refactoring Progress

```
Phase 1 [COMPLETE]
├── EventLoopManager (Async infrastructure)
├── BlueZAdapter (D-Bus communication)
└── BleSimulation (Clean simulation)

Phase 2 [IN PROGRESS - ~250 lines]
└── BleDeviceManager (Device operations)

Phase 3 [PLANNED - ~200 lines]
└── BleCharacteristicManager (Characteristic operations)

Phase 4 [PLANNED - ~300 lines]
└── BleDataProcessor (Data processing)

Phase 5 [PLANNED - Final]
└── BleService Simplification (→ 100 lines facade)
```

## Code Metrics

### File Size Distribution

```
BleService (Main)          42 KB  [████████████████████] 67%
BlueZAdapter              3.2 KB  [█] 5%
EventLoopManager          3.7 KB  [█] 6%
BleSimulation             3.0 KB  [█] 5%
CharRegistry              4.0 KB  [█] 6%
Other utilities           3.1 KB  [█] 5%
─────────────────────────────────
TOTAL                      59 KB [OK]
```

### Implementation Line Counts

| Component | Lines | Status |
|-----------|-------|--------|
| BleService | 1,267 | Original (Phase 5: -> 100) |
| BlueZAdapter | 145 | NEW |
| EventLoopManager | 112 | NEW |
| BleSimulation | 98 | CLEAN |
| Utilities | 250 | Existing |
| **Total** | ~1,900 | Well-organized |

## Architecture Overview

```
                    BleService (Facade)
                          │
              ┌───────────┼───────────────────────┐
              │           │                       │
         BlueZAdapter  EventLoop         BleSimulation
         D-Bus Comm    Mgr Async        Sensor Data
              │           │                       │
              └───────────┴───────────────────────┘
                          │
                    BlueZ D-Bus
                    (Bluetooth)
```

## Key Features

### EventLoopManager
- Multi-thread GMainLoop support
- Thread-safe operations
- Automatic resource cleanup
- Easy loop lifecycle management

### BlueZAdapter
- Encapsulated D-Bus operations
- Adapter auto-discovery
- Clean separation of concerns
- Extensible design

### BleSimulation
- Realistic sensor data
- Multiple sensor types
- Configurable parameters
- JSON output format

## Class Reference

### BleService (Facade)
- **Location**: `BleService.hpp/cpp`
- **Lines**: 1,267 (target: 100)
- **Role**: Main entry point for all BLE operations

### BlueZAdapter
- **Location**: `BlueZAdapter.hpp/cpp`
- **Lines**: 145
- **Role**: D-Bus communication layer

### EventLoopManager
- **Location**: `EventLoopManager.hpp/cpp`
- **Lines**: 112
- **Role**: Async event handling infrastructure

### BleSimulation
- **Location**: `BleSimulation.hpp/cpp`
- **Lines**: 98
- **Role**: Sensor data simulation

## Dependencies

### External Libraries
```
GLib-2.0        -> Event loops, threading
GIO-2.0         -> D-Bus integration
BlueZ 5.x       -> Bluetooth stack
oatpp           -> WebSocket framework
nlohmann/json   -> JSON serialization
```

### Internal
```
BleService -> BlueZAdapter, EventLoopManager, BleSimulation
           -> CharRegistry, BleFrameBuilder
           -> WebSocket APIs
```

## BlueZ Testing and Debugging

### Debug Mode - Enable Bluetooth Logging

To read the logs about bluetooth operations:

```bash
sudo btmon
```

This provides real-time monitoring of all Bluetooth activity on the system.

### Restart Bluetooth Service

If you need to restart the BlueZ daemon:

```bash
sudo systemctl restart bluetooth
```

### Interactive Testing with bluetoothctl
Contributing

When adding new features:

1. **Check Phase Plan** - See which phase applies
2. **Follow Structure** - Place code in appropriate manager
3. **Write Tests** - Add unit tests for new components
4. **Update Docs** - Reflect changes in ARCHITECTURE.md
5. **Build & Test** - Verify no regressions

##
```
scan on
```

This starts device discovery. Find your BLE peripheral and copy its MAC address.

```
scan off
```

Stop scanning when you've found your device.

#### Device Connection and Exploration

```
connect [MAC address]
```Troubleshooting

### Build Issues
- **Solution**: Run `cmake ..` to regenerate build files
- **Verify**: Check all new .cpp files are in build directory

### Runtime Issues
- **Check**: Ensure BlueZ daemon is running
- **Test**: Use `bluetoothctl` to verify Bluetooth

### Compilation Errors
- **Cause**: Old CMake cache
- **Fix**: `rm -rf CMakeCache.txt CMakeFiles && cmake ..`

### BlueZ Connection Issues

If you cannot connect to devices:

1. Verify BlueZ is running:
   ```bash
   sudo systemctl status bluetooth
   ```

2. Check adapter status:
   ```bash
   bluetoothctl
   list
   ```

3. Enable adapter if disabled:
   ```bash
   power on
   ```

4. Try restarting BlueZ:
   ```bash
   sudo systemctl restart bluetooth
   ```

## Support

For questions about:
- **Architecture** - See ARCHITECTURE.md
- **Implementation** - Check source code comments
- **Future Plans** - Review REFACTORING_PLAN.md
- **Bluetooth Testing** - See BlueZ Testing and Debugging section above

---

## Summary

Status: Phase 1 Complete
- Infrastructure properly separated
- 600+ lines of architecture documentation
- Production ready - builds and runs successfully
- Clear path to Phase 2: BleDeviceManager extraction

Next Phase: Phase 2 - Device Manager (in progress)
### Practical Example

```bash
# Start bluetoothctl
bluetoothctl

# Inside bluetoothctl:
[bluetooth]# scan on
# Wait for devices...
# Found device: 12:34:56:78:AB:CD MyDevice

[bluetooth]# scan off

[bluetooth]# connect 12:34:56:78:AB:CD
# Connection established

[bluetooth]# menu gatt
[gatt]# list-attributes
# Shows services and characteristics

[gatt]# select-attribute /org/bluez/hci0/dev_12_34_56_78_AB_CD/service0028/char0029

[gatt]# notify on
# Now receiving notifications
```

## Verification Steps

To verify the refactoring:

```bash
# Build the project
cd /home/ismail-sarwar/backend-IOT/backend-server/build
cmake ..
make -j4

# Run tests
./backend-server-test

# Check compilation
# ✅ Should see: [100%] Built target backend-server-exe

# Test runtime
timeout 5 ./backend-server-exe
# ✅ Should see: Server running on port 8000
```

## 📝 Contributing

When adding new features:

1. **Check Phase Plan** → See which phase applies
2. **Follow Structure** → Place code in appropriate manager
3. **Write Tests** → Add unit tests for new components
4. **Update Docs** → Reflect changes in ARCHITECTURE.md
5. **Build & Test** → Verify no regressions

## 🎓 Learning Path

### Beginner
1. Read PHASE_1_SUMMARY.md
2. Look at ARCHITECTURE_DIAGRAM.md
3. Browse BlueZAdapter.hpp (simplest component)

### Intermediate
1. Study EventLoopManager.hpp/cpp
2. Understand BleSimulation implementation
3. Review BleService facade pattern

### Advanced
1. Deep dive into ARCHITECTURE.md
2. Study REFACTORING_PLAN.md for future work
3. Plan Phase 2 components

## 🐛 Troubleshooting

### Build Issues
- **Solution**: Run `cmake ..` to regenerate build files
- **Verify**: Check all new .cpp files are in build directory

### Runtime Issues
- **Check**: Ensure BlueZ daemon is running
- **Test**: Use `bluetoothctl` to verify Bluetooth

### Compilation Errors
- **Cause**: Old CMake cache
- **Fix**: `rm -rf CMakeCache.txt CMakeFiles && cmake ..`

## 📞 Support

For questions about:
- **Architecture** → See ARCHITECTURE.md
- **Implementation** → Check source code comments
- **Future Plans** → Review REFACTORING_PLAN.md

---

## 📌 Summary

✅ **Phase 1 Complete**: Infrastructure properly separated
✅ **Well Documented**: 600+ lines of architecture docs
✅ **Production Ready**: Builds and runs successfully
✅ **Ready for Phase 2**: Clear path forward

**Status**: ✅ Ready for next refactoring phase
