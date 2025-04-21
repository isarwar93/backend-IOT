
#include "Room.hpp"
#include "./AppComponent.hpp"

// #include "GraphListener.hpp"

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




void Room::addClient(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket) {
    std::lock_guard<std::mutex> guard(m_peerByIdLock);
    m_clients.insert(socket);

    if (!m_running) {
        m_running = true;
        m_thread = std::thread([this]() {
            using namespace std::chrono;
            while (m_running) {
                OATPP_LOGi("Room:", "addclient()-loop");
                auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                float value = 20 + static_cast<float>(std::rand() % 100) / 10.0f;
                std::string json = "{ \"timestamp\": " + std::to_string(now) + ", \"value\": " + std::to_string(value) + " }";

                std::lock_guard<std::mutex> guard(m_peerByIdLock);
                for (const auto& client : m_clients) {
                    if (client && client->getConnection()) {
                        OATPP_LOGi("Room:", "send frame to client");
                        for(auto& pair : m_peerById) {
                            OATPP_LOGi("Room:", "sendMessage() - Sending message to userId={}", pair.first);
                            pair.second->sendMessage(json.c_str());
                        }
                        //client->sendOneFrameTextAsync(json.c_str());
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
    }
}



void Room::removeClient(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket) {
    std::lock_guard<std::mutex> guard(m_peerByIdLock);
    m_clients.erase(socket);
    if (m_clients.empty()) {
        m_running = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}

Room::~Room() {
    // m_clients.erase(socket);
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
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
    std::lock_guard<std::mutex> lock(m_graphMutex);
    m_graphClients.erase(userId);
    if (m_graphClients.empty()) {
        m_graphRunning = false;
        if (m_graphThread.joinable()) {
            m_graphThread.join();
        }
    }

    m_graphById.erase(userId);
}

void Room::streamGraph() {
    using namespace std::chrono;
    while (m_graphRunning) {
        auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        float value = 20 + static_cast<float>(std::rand() % 100) / 10.0f;

        std::string json =
            "{ \"timestamp\": " + std::to_string(now) +
            ", \"value\": " + std::to_string(value) +
            " }";

        std::lock_guard<std::mutex> lock(m_graphMutex);
        // for (auto& pair : m_graphClients) {
        //     OATPP_LOGi("Room:", "streamGraph() -", pair.first);
        //         pair.second->sendOneFrameTextAsync(json.c_str());
        //         //pair.second->sendMessage(json.c_str());
        //     }
        for(auto& pair : m_graphById) {
            OATPP_LOGi("Room:", "sendMessage() - Sending message to userId={}", pair.first);
            pair.second->sendMessage(json.c_str());
        }

        std::this_thread::sleep_for(milliseconds(50));
    }
}