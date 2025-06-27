
#include "Room.hpp"
#include "./AppComponent.hpp"
#include "config/Constants.hpp"
#include "service/ble/BleService.hpp"

Room::Room(const oatpp::String& name): m_name(name){
    OATPP_LOGi("Room:", "{} Constructed",m_name->c_str());
}

Room::~Room() {
    OATPP_LOGi("Room", "{} destroyed!", m_name->c_str());
}

void Room::addPeer(const std::shared_ptr<Peer>& peer) {
    std::lock_guard<std::mutex> guard(m_peerByIdLock);
    m_peerById[peer->getUserId()] = peer;
}

void Room::removePeerByUserId(v_int32 userId) {
    std::lock_guard<std::mutex> guard(m_peerByIdLock);
    m_peerById.erase(userId);
}

void Room::sendMessage(const oatpp::String& message) {
    std::lock_guard<std::mutex> guard(m_peerByIdLock);
    for(auto& pair : m_peerById) {
        OATPP_LOGi("Room:", "sendMessage() - Sending message to userId={}", pair.first);
        pair.second->sendMessage(message);
    }
}

void Room::addGraphSocket(v_int32 userId, 
    const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
    const std::shared_ptr<GraphListener>& graphListener) {
    std::lock_guard<std::mutex> lock(m_graphMutex);
    m_graphClients[userId] = socket;

    m_graphById[graphListener->getUserId()] = graphListener;

    if (!m_graphRunning) {
        m_graphRunning = true;
        m_graphThread = std::thread([this]() { streamGraph(); });
    }
}

void Room::leaveGraph(v_int32 userId) {
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        m_graphClients.erase(userId);
        m_graphById.erase(userId);
        if (m_graphClients.empty()) {
            m_graphRunning = false;
            m_cv.notify_all(); //  wake the thread immediately
        
        }
    }

    if (m_graphThread.joinable()) {
        m_graphThread.join();
    }
}

void Room::streamGraph() {
    using namespace std::chrono;
    std::unique_lock<std::mutex> lock(m_graphMutex);
    float value = 0.0f;
    auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
    while (m_graphRunning) {
        auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        //float value = 20 + static_cast<float>(std::rand() % 100) / 10.0f;
        // float value = USE_SRVC("ble")->getLatestMeasurement();
        
        if (bleService) {
            value = bleService->getLatestMeasurement();
        }

        std::string json =
            "{ \"timestamp\": " + std::to_string(now) +
            ", \"value\": " + std::to_string(value) +
            " }";

        for(auto& pair : m_graphById) {
            //OATPP_LOGi("Room:", "sendMessage() - Sending message to userId={}", pair.first);
            pair.second->sendMessage(json.c_str());
        }

        m_cv.wait_for(lock, std::chrono::milliseconds(10), [this] {
            return !m_graphRunning;
        });
    }
}

bool Room::isEmpty() const {
    std::lock_guard<std::mutex> lockPeers(m_peerByIdLock);
    std::lock_guard<std::mutex> lockGraphs(m_graphMutex);
    return m_peerById.empty() && m_graphClients.empty();
}

oatpp::String Room::getName() const {
    return m_name;
}


// void Room::setBleService(std::shared_ptr<BleService> ble) {
//     m_bleService = std::move(ble);
// }