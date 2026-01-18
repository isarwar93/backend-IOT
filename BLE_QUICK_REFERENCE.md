# BLE Service Refactoring - Quick Reference Guide

## Current Status: Phase 2 COMPLETE

### Phase Progress Timeline

```
Phase 1: Infrastructure [COMPLETE]
  EventLoopManager ............... 157 lines
  BlueZAdapter ................... 181 lines
  BleSimulation Refactor ......... 146 lines

Phase 2: Device Operations [COMPLETE]
  BleDeviceManager ............... 460 lines

Phase 3: Characteristics [PENDING]
  BleCharacteristicManager ....... ~200 lines (planned)

Phase 4: Data Processing [PENDING]
  BleDataProcessor ............... ~300 lines (planned)

Phase 5: Simplification [PENDING]
  BleService Reduction ........... ~100 lines (target)
```

## BleDeviceManager API Quick Reference

### Initialization
```cpp
BleDeviceManager manager(g_dbus_connection);
manager.setAdapterPath("/org/bluez/hci0");
```

### Device Discovery
```cpp
std::vector<BleDeviceInfo> devices = manager.scanDevices(10);  // 10 second timeout
manager.printScanResults(devices);
```

### Device Connection
```cpp
bool connected = manager.connectToDevice("00:11:22:33:44:55");
bool disconnected = manager.disconnectDevice("00:11:22:33:44:55");
bool isConn = manager.isConnected("00:11:22:33:44:55");
```

### Device Pairing
```cpp
bool paired = manager.isPaired("00:11:22:33:44:55");
bool pairSuccess = manager.pairDevice("00:11:22:33:44:55");
bool trusted = manager.trustDevice("00:11:22:33:44:55");
```

### Device Enumeration
```cpp
std::vector<std::string> connectedPaths = manager.getConnectedDevices();
bool removed = manager.removeDevice("00:11:22:33:44:55");
```

### Helper Functions
```cpp
std::string path = manager.macToDevicePath("00:11:22:33:44:55");
std::string mac = manager.parseMacFlexible(jsonInput);
```

## File Locations

| Component | Header | Implementation |
|-----------|--------|-----------------|
| BleDeviceManager | src/service/ble/BleDeviceManager.hpp | src/service/ble/BleDeviceManager.cpp |
| EventLoopManager | src/service/ble/EventLoopManager.hpp | src/service/ble/EventLoopManager.cpp |
| BlueZAdapter | src/service/ble/BlueZAdapter.hpp | src/service/ble/BlueZAdapter.cpp |
| BleSimulation | src/service/ble/BleSimulation.hpp | src/service/ble/BleSimulation.cpp |
| BleService | src/service/ble/BleService.hpp | src/service/ble/BleService.cpp |

## Key Statistics

| Metric | Value |
|--------|-------|
| Phase 1 Total Lines | 484 lines |
| Phase 2 Total Lines | 460 lines |
| Phase 1-2 Total | 944 lines |
| Original BleService | 1,267 lines |
| Reduction Progress | 25.5% extracted |
| Build Time | ~3 minutes |
| Test Time | <1 second |

## Documentation Files

| Document | Location | Purpose |
|----------|----------|---------|
| PHASE_2_SUMMARY.md | docs/PHASE_2_SUMMARY.md | Architecture & implementation details |
| PHASE_2_VERIFICATION.md | docs/PHASE_2_VERIFICATION.md | Verification & testing results |
| PHASE_2_COMPLETION.md | PHASE_2_COMPLETION.md | Overall completion summary |
| README.md | README.md | Updated with Phase 2 status |

## Build Verification

```bash
cd backend-server/build
cmake --build . --config Release
```

Expected output:
```
[ 75%] Built target backend-server-lib
[ 85%] Built target backend-server-exe
[100%] Built target backend-server-test
```

## Test Verification

```bash
cd backend-server/build
./backend-server-test
```

Expected output:
```
TEST[WSTest]:FINISHED - success!
```

## D-Bus Operations Used

### Adapter1 Interface
- StartDiscovery
- StopDiscovery
- RemoveDevice

### Device1 Interface
- Connect
- Disconnect
- Pair
- Properties.Get (Paired, Connected, Name, RSSI)
- Properties.Set (Trusted)

### ObjectManager
- GetManagedObjects (device enumeration)

## Common Patterns

### D-Bus Call Pattern
```cpp
GError* error = nullptr;
GVariant* result = g_dbus_connection_call_sync(
    conn, "org.bluez", path, interface,
    method, params, reply_type, flags, -1, nullptr, &error
);

if (error) {
    LOGE("Method failed: {}", error->message);
    g_error_free(error);
    return false;
}

// Use result...
g_variant_unref(result);
return true;
```

### Property Query Pattern
```cpp
GVariant* result = g_dbus_connection_call_sync(
    conn, "org.bluez", path, "org.freedesktop.DBus.Properties",
    "Get", g_variant_new("(ss)", "org.bluez.Device1", "PropertyName"),
    G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error
);

GVariant* val;
g_variant_get(result, "(v)", &val);
bool value = g_variant_get_boolean(val);  // or get_string, get_int16, etc.
g_variant_unref(val);
g_variant_unref(result);
```

## Integration Checklist for Phase 3

Before starting Phase 3, ensure:

- [ ] Phase 2 code compiles without errors
- [ ] All tests pass
- [ ] Documentation reviewed and understood
- [ ] BleCharacteristicManager scope defined
- [ ] Code extraction points identified in BleService.cpp
- [ ] D-Bus methods for characteristics mapped
- [ ] Error handling strategy aligned with Phase 2

## Performance Notes

- scanDevices() waits for specified timeout - blocking call
- All D-Bus calls are synchronous
- No multithreading within BleDeviceManager
- EventLoopManager handles async notifications
- MAC address parsing is fast (regex-based)

## Logging Integration

All methods use LogAdapt logging:
- `LOGI()` - Info level (operation success)
- `LOGW()` - Warning level (operation skipped)
- `LOGE()` - Error level (operation failed)

Example:
```cpp
LOGI("Successfully connected to device: {}", mac);
LOGE("Connect failed: {}", error->message);
```

## MAC Address Format Support

BleDeviceManager.parseMacFlexible() supports:
1. Raw string: `"00:11:22:33:44:55"`
2. JSON string: `"\"00:11:22:33:44:55\""`
3. JSON object: `"{\"mac\": \"00:11:22:33:44:55\"}"`
4. Quoted string: `"'00:11:22:33:44:55'"`

All formats automatically normalized to colons.

## Next Steps (Phase 3)

1. Review BleCharacteristicManager scope
2. Identify characteristic-related methods in BleService.cpp
3. Extract to new BleCharacteristicManager class
4. Follow Phase 2 patterns
5. Verify compilation and tests
6. Update documentation

---

**Last Updated:** January 16, 2026
**Status:** Phase 2 Complete - Ready for Phase 3
