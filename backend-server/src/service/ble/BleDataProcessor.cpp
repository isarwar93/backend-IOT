// File: src/service/ble/BleDataProcessor.cpp
#include "BleDataProcessor.hpp"
#include "config/LogAdapt.hpp"
#include "BleFrameBuilder.hpp"
#include "CharRegistry.hpp"
#include "BleSimulation.hpp"
#include <nlohmann/json.hpp>

BleDataProcessor::BleDataProcessor(BleFrameBuilder* frameBuilder, CharRegistry* charRegistry)
    : frameBuilder_(frameBuilder), charRegistry_(charRegistry), simulation_(nullptr) {}

BleDataProcessor::~BleDataProcessor() {
    stopStreaming();
}

void BleDataProcessor::setFrameBuilder(BleFrameBuilder* builder) {
    frameBuilder_ = builder;
}

void BleDataProcessor::setCharRegistry(CharRegistry* registry) {
    charRegistry_ = registry;
}

void BleDataProcessor::setSimulation(BleSimulation* sim) {
    simulation_ = sim;
}

void BleDataProcessor::addGraphSocket(v_int32 graphId,
                                     const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
                                     const std::shared_ptr<WSComm>& wsComm) {
    std::lock_guard<std::mutex> lock(m_graphMutex);
    m_graphClients[graphId] = socket;
    m_graphById[graphId] = wsComm;

    if (!m_graphRunning) {
        m_graphRunning = true;
        m_graphThread = std::thread([this]() {
            LOGI("Starting graph streaming thread");
            streamGraph();
            LOGI("Ending graph streaming thread");
        });
    }
}

void BleDataProcessor::leaveGraph(v_int32 userId) {
    bool shouldJoin = false;
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        m_graphClients.erase(userId);
        m_graphById.erase(userId);
        LOGI("Graph WebSocket user {} leaving", userId);
        if (m_graphClients.empty()) {
            LOGI("All graph clients disconnected, stopping graph streaming");
            m_graphRunning = false;
            m_cv.notify_all();
            shouldJoin = true;
        }
    }

    // Join outside the lock to avoid deadlock
    if (shouldJoin && m_graphThread.joinable()) {
        m_graphThread.join();
    }
}

bool BleDataProcessor::isEmpty() const {
    std::lock_guard<std::mutex> lock(m_graphMutex);
    return m_graphClients.empty();
}

void BleDataProcessor::processData(const std::string& path, const uint8_t* data, size_t len) {
    if (path.empty() || !data || len == 0) {
        LOGE("Invalid data received for path: {}", path);
        return;
    }

    // If streaming is not active, don't process
    if (!m_graphRunning.load()) {
        return;
    }

    // Add timeout to avoid infinite wait
    int timeout = GRAPH_VALUE_TIMEOUT_MS / 10; // 30 attempts at 10ms each
    while (m_graphValueChanged.load() && timeout > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timeout--;
    }

    if (timeout <= 0) {
        LOGW("Timeout waiting for graph data to be processed, overwriting data");
    }

    if (!charRegistry_) {
        LOGE("CharRegistry not set");
        return;
    }

    // Find characteristic metadata by path
    if (auto metaOpt = charRegistry_->getByPath(path)) {
        const auto& meta = *metaOpt;
        std::vector<float> convertedData;

        // Convert data based on UUID type
        if (meta.uuid == "f00d") {
            if (!frameBuilder_) {
                LOGE("FrameBuilder not set");
                return;
            }
            convertedData = frameBuilder_->makeCustom5(data, len);
        } else if (meta.uuid == "2a37") {
            // Heart rate measurement
            if (!frameBuilder_) {
                LOGE("FrameBuilder not set");
                return;
            }
            convertedData = frameBuilder_->makeHeart(data, len);
        } else if (meta.uuid == "2a5f") {
            // Pulse oximeter measurement
            if (!frameBuilder_) {
                LOGE("FrameBuilder not set");
                return;
            }
            convertedData = frameBuilder_->makePoX(data, len);
        } else {
            LOGW("Unknown UUID type: {}", meta.uuid);
            return;
        }

        // Store converted data points
        for (size_t i = 0; i < convertedData.size(); ++i) {
            std::string key = meta.name + "." + std::to_string(i + 1);
            CurrentGraphValue dataPoint;
            dataPoint.value = convertedData[i];
            dataPoint.updated = true;
            m_graphData[key] = dataPoint;
            LOGI("Graph data updated: {} = {}", key, dataPoint.value);
        }

        m_graphValueChanged.store(true);
    } else {
        LOGE("Characteristic path not found in registry: {}", path);
    }
}

void BleDataProcessor::setWebSocketFps(int fps) {
    if (fps < 1) fps = 1;
    if (fps > 100) fps = 100;
    m_webSocketMsgFps.store(fps);
    LOGI("WebSocket FPS set to {}", fps);
}

int BleDataProcessor::getWebSocketFps() const {
    return m_webSocketMsgFps.load();
}

void BleDataProcessor::enableSimulation(bool enable) {
    m_simulationEnabled.store(enable);
    LOGI("Simulation {}", enable ? "enabled" : "disabled");
}

bool BleDataProcessor::isSimulationEnabled() const {
    return m_simulationEnabled.load();
}

void BleDataProcessor::startStreaming() {
    std::lock_guard<std::mutex> lock(m_graphMutex);
    if (!m_graphRunning) {
        m_graphRunning = true;
        m_graphThread = std::thread([this]() {
            LOGI("Starting graph streaming");
            streamGraph();
            LOGI("Stopped graph streaming");
        });
    }
}

void BleDataProcessor::stopStreaming() {
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        m_graphRunning = false;
        m_cv.notify_all();
    }

    if (m_graphThread.joinable()) {
        m_graphThread.join();
    }
}

bool BleDataProcessor::isStreaming() const {
    return m_graphRunning.load();
}

void BleDataProcessor::streamGraph() {
    using namespace std::chrono;
    std::unique_lock<std::mutex> lock(m_graphMutex);
    
    float baseValue = 23.5f;
    int counter = 0;
    int counter2 = 0;
    int counter3 = 0;

    while (m_graphRunning) {
        // Handle simulation mode
        if (m_simulationEnabled.load()) {
            if (!simulation_) {
                LOGW("Simulation enabled but BleSimulation not set");
                m_graphRunning = false;
                break;
            }

            // Generate simulated data
            std::string jsonData = simulation_->generateSimulationData(counter, counter2, counter3, baseValue);
            auto newJson = nlohmann::json::parse(jsonData);

            // Copy shared_ptr to clients to keep them alive during send
            std::vector<std::shared_ptr<WSComm>> clients;
            clients.reserve(m_graphById.size());
            for (auto& pair : m_graphById) {
                if (pair.second) {
                    clients.push_back(pair.second);
                }
            }
            
            // Release lock before sending to avoid holding lock during I/O
            lock.unlock();
            
            // Broadcast to all connected clients
            for (auto& client : clients) {
                try {
                    client->sendMessage(newJson.dump().c_str());
                } catch (const std::exception& e) {
                    LOGW("Error sending message to client: {}", e.what());
                }
            }
            
            lock.lock();
            if (!m_graphRunning) break;

            // Control FPS
            int fps = m_webSocketMsgFps.load();
            if (fps < 1) fps = 1;
            if (fps > 100) fps = 100;
            auto sleepDuration = milliseconds(1000 / fps);

            // Release lock during sleep to allow add/leave operations
            lock.unlock();
            {
                std::mutex sleeperMutex;
                std::unique_lock<std::mutex> sleeperLock(sleeperMutex);
                m_cv.wait_for(sleeperLock, sleepDuration,
                             [this] { return !m_graphRunning.load(); });
            }
            lock.lock();
            continue;
        }

        // Handle real sensor data mode
        while (!m_graphValueChanged.load() && m_graphRunning) {
            m_cv.wait_for(lock, milliseconds(1),
                         [this] { return !m_graphRunning.load(); });
        }

        if (!m_graphRunning || m_graphData.empty()) {
            continue;
        }

        auto newJson = nlohmann::json::object();

        // Copy shared_ptr to clients to keep them alive during send
        std::vector<std::pair<v_int32, std::shared_ptr<WSComm>>> clients;
        clients.reserve(m_graphById.size());
        for (auto& pair : m_graphById) {
            if (pair.second) {
                clients.emplace_back(pair.first, pair.second);
            }
        }
        
        // Release lock before sending
        lock.unlock();
        
        // Broadcast to all connected clients
        for (auto& [userId, client] : clients) {
            try {
                LOGI("Sending graph data to userId={}", userId);

                // Add timestamp
                auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                newJson["time"] = std::to_string(now);

                // Add all updated graph data points
                for (auto& graphData : m_graphData) {
                    if (!graphData.second.updated) {
                        continue; // Skip non-updated values
                    }

                    newJson["id"] = graphData.first;
                    newJson["value"] = graphData.second.value;
                    client->sendMessage(newJson.dump().c_str());

                    // Mark as processed
                    graphData.second.updated = false;
                }
            } catch (const std::exception& e) {
                LOGW("Error sending message to userId={}: {}", userId, e.what());
            }
        }
        
        lock.lock();
        if (!m_graphRunning) break;

        // Reset change flag after sending
        m_graphValueChanged.store(false);
    }
}

bool BleDataProcessor::shutdown() {
    stopStreaming();
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        m_graphClients.clear();
        m_graphById.clear();
        m_graphData.clear();
    }
    return true;
}
