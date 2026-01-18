// File: src/service/ble/BleCharacteristicManager.hpp
#ifndef BLE_CHARACTERISTIC_MANAGER_HPP
#define BLE_CHARACTERISTIC_MANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <gio/gio.h>
#include <cstdint>

// Data structures for service and characteristic information
struct BleCharacteristicInfo {
    std::string name;               // e.g., "Heart Rate Measurement"
    std::string path;               // e.g., "/org/bluez/hci0/dev_XX/.../char0002"
    std::string uuid;               // e.g., "00002a37-0000-1000-8000-00805f9b34fb"
    bool notifying;                 // Current notification state
    std::vector<std::string> properties; // e.g., ["read", "write", "notify"]
};

struct BleServiceInfo {
    std::string name;               // e.g., "Heart Rate Service"
    std::string path;               // e.g., "/org/bluez/hci0/dev_XX/.../service0001"
    std::string uuid;               // e.g., "0000180d-0000-1000-8000-00805f9b34fb"
    std::vector<BleCharacteristicInfo> characteristics;
};

// UUID to readable name mapping
extern const std::unordered_map<std::string, std::string> UUID_TO_NAME;

class BleCharacteristicManager {
public:
    BleCharacteristicManager(GDBusConnection* conn = nullptr);

    // Setup
    void setConnection(GDBusConnection* conn);
    void setAdapterPath(const std::string& path);

    // Service and Characteristic Discovery
    std::vector<BleServiceInfo> getServicesAndCharacteristics(const std::string& mac);

    // Notification Management
    bool enableNotification(const std::string& charPath);
    bool disableNotification(const std::string& charPath);

    // Characteristic Read/Write Operations
    size_t readCharacteristic(const std::string& charPath, uint8_t* retData);
    bool writeCharacteristic(const std::string& charPath, const std::vector<uint8_t>& data);

    // Service Operations
    std::string readService(const std::string& mac, const std::string& uuid);
    bool writeService(const std::string& body);
    bool enableServices(const std::string& body);

    // Notification State
    int getActiveNotificationCount() const { return numOfActiveNotifications_; }
    void incrementNotifications() { numOfActiveNotifications_++; }
    void decrementNotifications() {
        numOfActiveNotifications_--;
        if (numOfActiveNotifications_ < 0) numOfActiveNotifications_ = 0;
    }

    // Utility
    std::string extractUuidPrefix(const std::string& uuid);

private:
    GDBusConnection* gDBusConn_;
    std::string adapterPath_;
    int numOfActiveNotifications_;

    // Helper to convert UUID to human-readable name
    std::string uuidToReadableName(const std::string& uuid) const;
};

#endif // BLE_CHARACTERISTIC_MANAGER_HPP
