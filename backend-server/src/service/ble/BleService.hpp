#ifndef BLESERVICE_HPP
#define BLESERVICE_HPP

#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <gio/gio.h>
#include <vector>
#include "service/IService.hpp"


struct BleDescriptorInfo {
    std::string name; // e.g., "Client Characteristic Configuration"
    std::string path; // e.g., "/org/bluez/hci0/dev_A0_DD_6C_AF_73_9E/service0001/char0002/desc0003"
    std::string uuid;
    std::vector<std::string> properties; 
};

struct BleCharacteristicInfo {
    std::string name; // e.g., "Heart Rate Measurement"
    std::string path; // e.g., "/org/bluez/hci0/dev_A0_DD_6C_AF_73_9E/service0001/char0002"
    std::string uuid;
    bool notifying;
    // e.g., "read", "write", "notify"
    std::vector<std::string> properties; 
};

struct BleServiceInfo {
    std::string name; // e.g., "Heart Rate Service"  
    std::string path; // e.g., "/org/bluez/hci0/dev_A0_DD_6C_AF_73_9E/service0001"
    std::string uuid; // e.g., "0000180D-0000-1000-8000-00805f9b34fb"
    std::vector<BleCharacteristicInfo> characteristics;
};

struct BleDeviceInfo {
    std::string mac;
    std::string name;
    int rssi;
};



class BleService : public IService {
public:
    BleService() = default;
    
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    bool configure(const std::string& jsonConfig) override;
    std::string sendCommand(const std::string& command) override;
    std::string getStatusJson() const override;

    float getLatestMeasurement() const;
    void onHeartRateReceived(float hr); 

    std::vector<BleDeviceInfo> scanDevices(int timeoutSeconds);
    bool connectToDevice(const std::string& mac);
    bool disconnectDevice(const std::string& jsonmac);
    std::vector<BleServiceInfo> getServicesAndCharacteristics(const std::string& mac);

    bool toggleNotify(const std::string& body);
    double readService(const std::string& mac, const std::string& uuid);
    bool writeService(const std::string& body);
    // bool enableServices(const std::string& mac, const std::vector<std::string>& serviceUUIDs);
    bool enableServices(const std::string& mac);
    void printScanResults(const std::vector<BleDeviceInfo>& devices);

    bool removeDevice(const std::string& body);
    bool isConnected(const std::string& body);
    bool isPaired(const std::string& body);
    bool pairDevice(const std::string& body);
    bool cancelPairing(const std::string& body);
    bool trustDevice(const std::string& mac);
    std::vector<std::string> getConnectedDevices();
    bool cleanupDisconnectedDevices();

private:
    bool enableNotification(const std::string& path);
    bool disableNotification(const std::string& path);
    std::string macToDevicePath(const std::string& mac);
    GDBusConnection* conn;

    bool initBluez();
    void parseHeartRate(uint8_t* data, gsize len);
    void handleHeartRateData(const guint8* data, gsize len);
    static void onPropertiesChanged(GDBusConnection*,
                                const gchar*,
                                const gchar*,
                                const gchar*,
                                const gchar*,
                                GVariant* parameters,
                                gpointer user_data);

    bool running = false;
    std::vector<BleDeviceInfo> currentConnectedDevices;
    std::string macAddress;
    std::string lastCommandResponse;
    std::atomic<float> latestValue {0.0f};

    std::string adapterPath;
    std::string findAdapter();

    GMainLoop* m_loop = nullptr;
    std::thread m_loopThread;


    // Add more UUIDs and names as needed
    std::unordered_map<std::string, std::string> uuidToName = {
        {"00001801-0000-1000-8000-00805f9b34fb", "Generic Attribute Profile"},
        {"00002a05-0000-1000-8000-00805f9b34fb", "Service Changed"},
        {"00002902-0000-1000-8000-00805f9b34fb", "Client Characteristic Configuration"},
        {"00002b3a-0000-1000-8000-00805f9b34fb", "Server Supported Features"},
        {"00002b29-0000-1000-8000-00805f9b34fb", "Client Supported Features"},
        {"0000180a-0000-1000-8000-00805f9b34fb", "Service Device Information"},
        {"00002a29-0000-1000-8000-00805f9b34fb", "Characteristic Manufacturer Name String"},
        {"00002a24-0000-1000-8000-00805f9b34fb", "Characteristic Model Number String"},
        {"0000180f-0000-1000-8000-00805f9b34fb", "Service Battery Information"},
        {"00002a19-0000-1000-8000-00805f9b34fb", "Characteristic Battery Level"},
        {"0000dead-0000-1000-8000-00805f9b34fb", "Service custom encrypted"},
        {"0000beef-0000-1000-8000-00805f9b34fb", "Characteristic custom encrypted"},
        {"0000baad-0000-1000-8000-00805f9b34fb", "Service Custom"},
        {"0000f00d-0000-1000-8000-00805f9b34fb", "Characteristic Custom"},
        {"0000c01d-0000-1000-8000-00805f9b34fb", "Characteristic Custom Descriptor"},
        {"0000180d-0000-1000-8000-00805f9b34fb", "Service Heart Rate"},
        {"00002a37-0000-1000-8000-00805f9b34fb", "Characteristic Heart Rate Measurement"},
        {"00002a38-0000-1000-8000-00805f9b34fb", "Characteristic Body Sensor Location"},
    };
};

#endif // BLESERVICE_HPP