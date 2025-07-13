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


struct BleDeviceInfo {
    std::string mac;
    std::string name;
    int rssi;
};

struct BleCharacteristicInfo {
    std::string uuid;
    bool notifying;
    std::string value;
    std::vector<std::string> properties; // e.g., "read", "write", "notify"
};

struct BleServiceInfo {
    std::string uuid;
    std::vector<BleCharacteristicInfo> characteristics;
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
    bool enableHeartRateNotifications(const std::string& deviceMac);
    bool startScanUntilFound(const std::string& targetMac, int timeoutSeconds = 10);
    std::vector<BleServiceInfo> getServicesAndCharacteristics(const std::string& mac);
    // bool enableServices(const std::string& mac, const std::vector<std::string>& serviceUUIDs);
    bool enableServices(const std::string& mac);
    void printScanResults(const std::vector<BleDeviceInfo>& devices);

    bool disconnectDevice(const std::string& body);
    bool removeDevice(const std::string& body);
    bool isConnected(const std::string& body);
    bool isPaired(const std::string& body);
    bool pairDevice(const std::string& body);
    bool cancelPairing(const std::string& body);
    bool trustDevice(const std::string& mac);
    std::vector<std::string> getConnectedDevices();
    bool cleanupDisconnectedDevices();

private:
    std::string macToDevicePath(const std::string& mac);
    GDBusConnection* conn;

    bool initBluez();
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
};

#endif // BLESERVICE_HPP