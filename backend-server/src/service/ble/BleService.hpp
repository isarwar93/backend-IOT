#ifndef BLESERVICE_HPP
#define BLESERVICE_HPP

#include "service/IService.hpp"
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>

#include <iostream>
#include <thread>
#include <chrono>

#include <gio/gio.h>
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
 


    // void startScan(DeviceFoundCallback onDeviceFound);
    bool connectToDevice(const std::string& address);

    bool enableHeartRateNotifications(const std::string& deviceMac);

    bool startScanUntilFound(const std::string& targetMac, int timeoutSeconds = 10);
    bool initBluez();


private:
    void handleHeartRateData(const guint8* data, gsize len);

    static void onPropertiesChanged(GDBusConnection*,
                                const gchar*,
                                const gchar*,
                                const gchar*,
                                const gchar*,
                                GVariant* parameters,
                                gpointer user_data);


    bool running = false;
    std::string macAddress;
    std::string lastCommandResponse;
    std::atomic<float> latestValue {0.0f};

    std::string adapterPath;
    std::string findAdapter();




    GMainLoop* m_loop = nullptr;
    std::thread m_loopThread;

};

#endif // BLESERVICE_HPP