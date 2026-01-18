#include "BleService.hpp"
#include "config/LogAdapt.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ============================================================================
// Lifecycle Management
// ============================================================================

BleService::BleService()
    : m_running(false), m_macAddress(""), m_lastCommandResponse(""), m_adapterPath("")
{
    // Managers will be initialized in start() after BlueZ is initialized
}

BleService::~BleService()
{
    stop();
}

// ============================================================================
// IService Interface Implementation
// ============================================================================

bool BleService::start()
{
    if (m_running) {
        LOGW("BLE Service already running");
        return true;
    }
    
    LOGI("Starting BLE Service...");
    
    if (!initBluez()) {
        LOGE("Failed to initialize BlueZ");
        return false;
    }
    
    m_running = true;
    LOGI("BLE Service started successfully");
    return true;
}

void BleService::stop()
{
    if (!m_running) return;
    
    LOGI("Stopping BLE Service...");
    
    if (m_dataProcessor) {
        m_dataProcessor->shutdown();
    }
    
    m_running = false;
    LOGI("BLE Service stopped");
}

bool BleService::isRunning() const
{
    return m_running;
}

bool BleService::configure(const std::string& jsonConfig)
{
    try {
        auto config = json::parse(jsonConfig);
        // Configuration can be extended as needed
        LOGI("Configuration applied successfully");
        return true;
    } catch (const std::exception& e) {
        LOGE("Configuration error: {}", e.what());
        return false;
    }
}

std::string BleService::sendCommand(const std::string& command)
{
    m_lastCommandResponse = "Command executed: " + command;
    return m_lastCommandResponse;
}

std::string BleService::getStatusJson() const
{
    json status = {
        {"running", m_running},
        {"adapter", m_adapterPath},
        {"connected_devices", m_deviceManager ? m_deviceManager->getConnectedDevices().size() : 0}
    };
    return status.dump();
}

// ============================================================================
// Device Operations - Delegated to BleDeviceManager
// ============================================================================

std::vector<BleDeviceInfo> BleService::scanDevices(int timeoutSeconds)
{
    return m_deviceManager ? m_deviceManager->scanDevices(timeoutSeconds) : std::vector<BleDeviceInfo>{};
}

bool BleService::connectToDevice(const std::string& mac)
{
    return m_deviceManager && m_deviceManager->connectToDevice(mac);
}

bool BleService::disconnectDevice(const std::string& mac)
{
    return m_deviceManager && m_deviceManager->disconnectDevice(mac);
}

std::vector<BleServiceInfo> BleService::getServicesAndCharacteristics(const std::string& mac)
{
    return m_charManager ? m_charManager->getServicesAndCharacteristics(mac) : std::vector<BleServiceInfo>{};
}

bool BleService::removeDevice(const std::string& body)
{
    if (!m_deviceManager) return false;
    
    try {
        std::string mac = json::parse(body).value("mac", "");
        return m_deviceManager->removeDevice(mac);
    } catch (const std::exception& e) {
        LOGE("removeDevice error: {}", e.what());
        return false;
    }
}

bool BleService::isConnected(const std::string& body)
{
    if (!m_deviceManager) return false;
    
    try {
        std::string mac = json::parse(body).value("mac", "");
        return m_deviceManager->isConnected(mac);
    } catch (const std::exception& e) {
        LOGE("isConnected error: {}", e.what());
        return false;
    }
}

bool BleService::isPaired(const std::string& body)
{
    if (!m_deviceManager) return false;
    
    try {
        std::string mac = json::parse(body).value("mac", "");
        return m_deviceManager->isPaired(mac);
    } catch (const std::exception& e) {
        LOGE("isPaired error: {}", e.what());
        return false;
    }
}

bool BleService::pairDevice(const std::string& body)
{
    if (!m_deviceManager) return false;
    
    try {
        std::string mac = json::parse(body).value("mac", "");
        m_deviceManager->pairDevice(mac);
        return true;
    } catch (const std::exception& e) {
        LOGE("pairDevice error: {}", e.what());
        return false;
    }
}

bool BleService::cancelPairing(const std::string& body)
{
    if (!m_deviceManager) return false;
    
    try {
        std::string mac = json::parse(body).value("mac", "");
        m_deviceManager->removeDevice(mac);
        return true;
    } catch (const std::exception& e) {
        LOGE("cancelPairing error: {}", e.what());
        return false;
    }
}

bool BleService::trustDevice(const std::string& mac)
{
    return m_deviceManager && m_deviceManager->trustDevice(mac);
}

std::vector<std::string> BleService::getConnectedDevices()
{
    return m_deviceManager ? m_deviceManager->getConnectedDevices() : std::vector<std::string>{};
}

bool BleService::cleanupDisconnectedDevices()
{
    return m_deviceManager && m_deviceManager->cleanupDisconnectedDevices();
}

void BleService::printScanResults(const std::vector<BleDeviceInfo>& devices)
{
    if (m_deviceManager) {
        m_deviceManager->printScanResults(devices);
    }
}

// ============================================================================
// Characteristic Operations - Delegated to BleCharacteristicManager
// ============================================================================

std::string BleService::toggleNotify(const std::string& body)
{
    if (!m_charManager) {
        return R"({"error": "Characteristic manager not initialized"})";
    }
    
    try {
        std::string charPath = json::parse(body).value("charPath", "");
        bool enabled = m_charManager->enableNotification(charPath);
        
        return enabled ? 
            R"({"status": "notification enabled"})" : 
            R"({"error": "Failed to enable notification"})";
    } catch (const std::exception& e) {
        LOGE("toggleNotify error: {}", e.what());
        return R"({"error": ")" + std::string(e.what()) + R"("})";
    }
}

size_t BleService::readCharacteristics(const std::string& charPath, guint8* retData)
{
    return m_charManager ? m_charManager->readCharacteristic(charPath, retData) : 0;
}

std::string BleService::readService(const std::string& mac, const std::string& uuid)
{
    return m_charManager ? m_charManager->readService(mac, uuid) : "{}";
}

bool BleService::writeService(const std::string& body)
{
    if (!m_charManager) return false;
    
    try {
        return m_charManager->writeService(body);
    } catch (const std::exception& e) {
        LOGE("writeService error: {}", e.what());
        return false;
    }
}

bool BleService::enableServices(const std::string& mac)
{
    return m_charManager && m_charManager->enableServices(mac);
}

// ============================================================================
// Graph Operations - Delegated to BleDataProcessor
// ============================================================================

void BleService::addGraphSocket(v_int32 userId, 
    const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
    const std::shared_ptr<WSComm>& graphWebSocket)
{
    if (m_dataProcessor) {
        m_dataProcessor->addGraphSocket(userId, socket, graphWebSocket);
    }
}

void BleService::leaveGraph(v_int32 userId)
{
    if (m_dataProcessor) {
        m_dataProcessor->leaveGraph(userId);
    }
}

void BleService::streamGraph()
{
    if (m_dataProcessor) {
        m_dataProcessor->startStreaming();
    }
}

bool BleService::isEmpty() const
{
    return !m_dataProcessor || m_dataProcessor->isEmpty();
}

// ============================================================================
// Simulation and FPS Control
// ============================================================================

bool BleService::setSimulation(std::string body)
{
    try {
        
        auto parsed = json::parse(body);
        bool enable = parsed.value("simulation", false);
        
        if (enable) {
            m_simulation.enableSimulation();
            if (m_dataProcessor) {
                m_dataProcessor->setSimulation(&m_simulation);
                m_dataProcessor->enableSimulation(true);
            }
        } else {
            m_simulation.disableSimulation();
            if (m_dataProcessor) {
                m_dataProcessor->enableSimulation(false);
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOGE("setSimulation error: {}", e.what());
        return false;
    }
}

bool BleService::webSocketMsgSetFps(std::string body)
{
    if (!m_dataProcessor) return false;
    
    try {
        int fps = json::parse(body).value("wsfps", 20);
        m_dataProcessor->setWebSocketFps(fps);
        LOGI("WebSocket FPS set to {}", fps);
        return true;
    } catch (const std::exception& e) {
        LOGE("setFPS error: {}", e.what());
        return false;
    }
}

// ============================================================================
// Private Initialization Helpers
// ============================================================================

bool BleService::initBluez()
{
    LOGI("Initializing BlueZ components...");
    
    // Initialize event loop manager
    m_loopManager = std::make_unique<EventLoopManager>();
    
    // Initialize and configure BlueZ adapter
    m_bluezAdapter = std::make_unique<BlueZAdapter>();
    if (!m_bluezAdapter->initialize()) {
        LOGE("BlueZ adapter initialization failed");
        return false;
    }
    
    m_adapterPath = m_bluezAdapter->getAdapterPath();
    LOGI("BlueZ adapter initialized: {}", m_adapterPath);
    
    // Get D-Bus connection
    auto connection = m_bluezAdapter->getConnection();
    if (!connection) {
        LOGE("Failed to obtain D-Bus connection");
        return false;
    }
    
    // Initialize component managers
    m_deviceManager = std::make_unique<BleDeviceManager>(connection);
    
    m_charManager = std::make_unique<BleCharacteristicManager>(connection);
    m_charManager->setAdapterPath(m_adapterPath);
    
    m_dataProcessor = std::make_unique<BleDataProcessor>(&m_frameBuilder, &m_charRegistry);
    
    initializeManagers();
    
    LOGI("All BLE managers initialized successfully");
    return true;
}

void BleService::initializeManagers()
{
    // Additional manager configuration can be added here
    // Currently, all dependencies are set in constructors
}
