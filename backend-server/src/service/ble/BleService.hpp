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
#include <condition_variable>

#include "service/IService.hpp"
#include "websocket/GraphWebSocket.hpp"
#include "websocket/WSComm.hpp"

// struct BleDescriptorInfo {
//     std::string name; // e.g., "Client Characteristic Configuration"
//     std::string path; // e.g., "/org/bluez/hci0/dev_A0_DD_6C_AF_73_9E/service0001/char0002/desc0003"
//     std::string uuid;
//     std::vector<std::string> properties; 
// };

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
    
    ~BleService() override{
        stop();
        stopAllLoops();
    }
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

    std::string toggleNotify(const std::string& body);
    std::string readService(const std::string& mac, const std::string& uuid);
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

    // Graph WebSocket related members
    void addGraphSocket(v_int32 userId, 
        const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
        const std::shared_ptr<WSComm>& graphWebSocket);
        
    void leaveGraph(v_int32 userId);
    void streamGraph();
    bool isEmpty() const;
    bool shutdown();


private:
    
    //Global Variables for BLE service
    GDBusConnection* gDBusConn;
    std::atomic<bool> firstNotificationReceived{false};
    std::atomic<int> numOfFloatsReceived{0};
    uint8_t numOfActiveNotifications = 0;
    std::unordered_map<std::string,int> numOfGraphsPerChar;
    void processData(const std::string& path, const guint8* data, gsize len);


    void createGraphWebSocket();
    bool enableNotification(const std::string& path);
    bool disableNotification(const std::string& path);
    // void saveThreadsCharacteristics( 
    //                 std::string characteristicPath,
    //                 GMainLoop* m_loop,
    //                 std::thread m_loopThread);
    //std::unordered_map<std::string,std::pair<GMainLoop*,std::thread>> mapThreadsCharacteristics;
    std::string macToDevicePath(const std::string& mac);


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
    std::vector<BleServiceInfo> currentServices;
    std::string macAddress;
    std::string lastCommandResponse;
    std::atomic<float> latestValue {0.0f};

    std::string adapterPath;
    std::string findAdapter();

    // Later for multiple connections in parallel
    // it will be usefull for the app
    struct LoopWorker {
        GMainContext* ctx = nullptr; // created with g_main_context_new(); unref when done
        GMainLoop*    loop = nullptr; // created with g_main_loop_new(ctx, FALSE); unref when done
        std::thread   th;             // runs g_main_loop_run(loop)
    };

    // IMPORTANT: one GMainContext per thread for multiple loops in parallel
    // (the default context should not be run from multiple threads simultaneously).
    std::unordered_map<std::string, LoopWorker> m_loops;
    mutable std::mutex m_mx;

    static void destroyWorker(LoopWorker& w) {
        // Called after thread is already joined or never started.
        if (w.loop)  { g_main_loop_unref(w.loop);   w.loop = nullptr; }
        if (w.ctx)   { g_main_context_unref(w.ctx); w.ctx  = nullptr; }
    }

    // Start a dedicated GMainLoop on its own thread for a given key.
    // Returns false if a loop for this key already exists or creation fails.
    bool startLoopFor(const std::string& key);
    // Stop + join + free the loop for this key, if present.
    void stopLoopFor(const std::string& key);
    // Stop everything (used in dtor / shutdown).
    void stopAllLoops();
    //helpers
    bool hasLoop(const std::string& key) const;
    size_t loopCount() const;


    
    // For Graph WebSocket
    std::unordered_map<v_int32, std::shared_ptr<oatpp::websocket::AsyncWebSocket>> m_graphClients;
    std::unordered_map<v_int32, std::shared_ptr<WSComm>> m_graphById;

    std::thread m_graphThread;
    std::atomic<bool> m_graphRunning = false;
    mutable std::mutex m_graphMutex;
    std::condition_variable m_cv;

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