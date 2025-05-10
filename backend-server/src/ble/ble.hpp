// #pragma once

// #include <string>
// #include <functional>
// #include <cstdint>

// class BleClient {
// public:
//     using BleCallback = std::function<void(uint8_t bpm)>;

//     BleClient(const std::string& deviceMac);
//     ~BleClient();

//     static void scanDevices(int durationSeconds, void* userData);
//     bool connectToDevice();
//     bool subscribeToBle(const BleCallback& callback);

// private:
//     std::string m_deviceMac;
//     std::string m_hrCharPath;
//     BleCallback m_callback;

//     void onBleNotification(const uint8_t* data, size_t len);
// };



#pragma once

#include <string>
#include <functional>

class BleClient {
public:
    using DeviceFoundCallback = std::function<void(const std::string& address, const std::string& name)>;

    explicit BleClient(const std::string& targetName);
    ~BleClient();

    void startScan(DeviceFoundCallback onDeviceFound);
    bool connectToDevice(const std::string& address);

    bool enableHeartRateNotifications(const std::string& deviceMac);


private:
    std::string targetName;
    std::string adapterPath;

    bool initBluez();
    std::string findAdapter();
};
