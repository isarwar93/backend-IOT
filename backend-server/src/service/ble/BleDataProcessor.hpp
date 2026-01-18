// File: src/service/ble/BleDataProcessor.hpp
#ifndef BLE_DATA_PROCESSOR_HPP
#define BLE_DATA_PROCESSOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <chrono>
#include <oatpp-websocket/WebSocket.hpp>
#include <oatpp/Environment.hpp>
#include "websocket/WSComm.hpp"

// Graph data value with update tracking
struct CurrentGraphValue {
    float value;
    bool updated;
};

// Forward declarations
class BleSimulation;
class BleFrameBuilder;
class CharRegistry;

class BleDataProcessor {
public:
    BleDataProcessor(BleFrameBuilder* frameBuilder = nullptr, 
                     CharRegistry* charRegistry = nullptr);
    ~BleDataProcessor();

    // Graph client management
    void addGraphSocket(v_int32 graphId, 
                       const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
                       const std::shared_ptr<WSComm>& wsComm);
    void leaveGraph(v_int32 userId);
    bool isEmpty() const;

    // Data processing
    void processData(const std::string& path, const uint8_t* data, size_t len);
    
    // Graph streaming control
    void startStreaming();
    void stopStreaming();
    bool isStreaming() const;
    
    // FPS control for WebSocket messages
    void setWebSocketFps(int fps);
    int getWebSocketFps() const;
    
    // Simulation control
    void enableSimulation(bool enable);
    bool isSimulationEnabled() const;

    // Configuration
    void setSimulation(BleSimulation* sim);
    void setFrameBuilder(BleFrameBuilder* builder);
    void setCharRegistry(CharRegistry* registry);

    // Utility
    bool shutdown();

private:
    // Helper for streaming thread
    void streamGraph();
    
    // Graph client management
    std::unordered_map<v_int32, std::shared_ptr<oatpp::websocket::AsyncWebSocket>> m_graphClients;
    std::unordered_map<v_int32, std::shared_ptr<WSComm>> m_graphById;
    
    // Data management
    std::unordered_map<std::string, CurrentGraphValue> m_graphData;
    
    // Thread and synchronization
    std::thread m_graphThread;
    std::atomic<bool> m_graphRunning{false};
    std::atomic<bool> m_graphValueChanged{false};
    mutable std::mutex m_graphMutex;
    std::condition_variable m_cv;
    
    // Configuration
    std::atomic<int> m_webSocketMsgFps{20};
    std::atomic<bool> m_simulationEnabled{false};
    
    // Dependencies
    BleFrameBuilder* frameBuilder_;
    CharRegistry* charRegistry_;
    BleSimulation* simulation_;
    
    // Timeout constant
    static constexpr int GRAPH_VALUE_TIMEOUT_MS = 300; // 3 seconds max wait
};

#endif // BLE_DATA_PROCESSOR_HPP
