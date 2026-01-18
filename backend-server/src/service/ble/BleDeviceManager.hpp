#ifndef BLE_DEVICE_MANAGER_HPP
#define BLE_DEVICE_MANAGER_HPP

#include <string>
#include <vector>
#include <gio/gio.h>

struct BleDeviceInfo {
    std::string mac;
    std::string name;
    int rssi;
};

/// Manages BLE device operations such as scanning, connecting, pairing, and trusting devices.
/// Abstracts device-level operations from the main BleService.
class BleDeviceManager {
public:
    BleDeviceManager(GDBusConnection* conn);
    ~BleDeviceManager() = default;

    // Set the D-Bus connection to use for operations
    void setConnection(GDBusConnection* conn);

    // Set the adapter path for device operations
    void setAdapterPath(const std::string& path);

    // Device Discovery
    // Scan for BLE devices with optional timeout
    std::vector<BleDeviceInfo> scanDevices(int timeoutSeconds = 5);

    // Device Connection Management
    // Connect to a BLE device by MAC address
    bool connectToDevice(const std::string& mac);

    // Disconnect from a BLE device
    bool disconnectDevice(const std::string& mac);

    // Device Pairing
    // Check if device is already paired
    bool isPaired(const std::string& mac);

    // Initiate pairing process
    bool pairDevice(const std::string& mac);

    // Device Trust
    // Mark device as trusted
    bool trustDevice(const std::string& mac);

    // Device Management
    // Get list of currently connected devices
    std::vector<std::string> getConnectedDevices();

    // Check if device is currently connected
    bool isConnected(const std::string& mac);

    // Remove device from known devices
    bool removeDevice(const std::string& mac);

    // Cleanup disconnected devices
    bool cleanupDisconnectedDevices();

    // Utility
    // Print scan results to console
    void printScanResults(const std::vector<BleDeviceInfo>& devices);

private:
    GDBusConnection* gDBusConn_ = nullptr;
    std::string adapterPath_;

    // Helper method to construct D-Bus device path from MAC
    std::string macToDevicePath(const std::string& mac);

    // Helper to parse MAC string flexibly (JSON or plain)
    std::string parseMacFlexible(const std::string& input);
};

#endif // BLE_DEVICE_MANAGER_HPP
