
#include "Lobby.hpp"
#include "./AppComponent.hpp" // for logs


v_int32 Lobby::obtainNewUserId() {
    return m_userIdCounter ++;
}

std::shared_ptr<Room> Lobby::getOrCreateRoom(const oatpp::String& roomName) {
    std::lock_guard<std::mutex> lock(m_roomsMutex);
    std::shared_ptr<Room>& room = m_rooms[roomName];
    if(!room) {
        room = std::make_shared<Room>(roomName);
        //room->setBleService(m_bleService); //  Inject BLE service
    }
    return room;
}

void Lobby::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) {

    if (!params) {
        OATPP_LOGi("Lobby", "params must not be null");
        return;
    }

    auto it = params->find("username");
    if (it != params->end()) {
        //PASS
    } else {
        OATPP_LOGi("Lobby", "username key not found in params");
        return;
    }

    auto roomName = params->find("username")->second;
    auto nickname = params->find("nickname")->second;

    OATPP_LOGi("Lobby", " roomName: {}",roomName);
    OATPP_LOGi("Lobby", " nickname: {}",nickname);

  
    // auto roomName = params->find("roomName")->second;


    auto type = params->find("type")->second;
    OATPP_LOGi("Lobby", " type: {}",type);

    // assign unique id to each nick
    v_int32 userId;
    {
        std::lock_guard<std::mutex> lock(m_roomsMutex);
        auto it = m_nicknameToUserId.find(nickname);
        if (it != m_nicknameToUserId.end()) {
            userId = it->second;
        } else {
            userId = obtainNewUserId();
            m_nicknameToUserId[nickname] = userId;
        }
    }

    if (type == "graph") {
        auto room = getOrCreateRoom(roomName);

        auto userId = obtainNewUserId();

      
        auto graphy = std::make_shared<GraphListener>(socket, room, nickname, userId);

        socket->setListener(graphy);
        room->addGraphSocket(userId, socket,graphy);
        room->sendMessage(nickname + "graph joined " + roomName);
    

    } 
    else {

        auto room = getOrCreateRoom(roomName);
      
        auto peer = std::make_shared<Peer>(socket, room, nickname, obtainNewUserId());
        socket->setListener(peer);
      
        room->addPeer(peer);
        room->sendMessage(nickname + " joined " + roomName);
    }
}

void Lobby::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket) {
    auto listener = socket->getListener();

    // Check if it's a chat peer
    if (auto peer = std::dynamic_pointer_cast<Peer>(listener)) {
        auto room = peer->getRoom();
        auto nickname = peer->getNickname();
        room->removePeerByUserId(peer->getUserId());
        m_nicknameToUserId.erase(nickname);

        if (room->isEmpty()) {
            std::lock_guard<std::mutex> lock(m_roomsMutex);
            m_rooms.erase(room->getName()); // or use roomName directly
            // OATPP_LOGi("Lobby", "peer: Deleted room {} because it's empty", peer->getNickname()->c_str());
        }
    }
    // Check if it's a graph peer
    else if (auto graphPeer = std::dynamic_pointer_cast<GraphListener>(listener)) {

        auto nickname = graphPeer->getNickname();
        auto room = graphPeer->getRoom();
        auto userId = graphPeer->getUserId();
        room->leaveGraph(userId);
        m_nicknameToUserId.erase(nickname);

        if (room->isEmpty()) {
            std::lock_guard<std::mutex> lock(m_roomsMutex);
            m_rooms.erase(room->getName()); // or use roomName directly
            // OATPP_LOGi("Lobby", " graph: Deleted room {} because it's empty", graphPeer->getNickname()->c_str());
        }
    }
    socket->setListener(nullptr);
}