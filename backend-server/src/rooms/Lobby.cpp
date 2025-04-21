
#include "Lobby.hpp"



v_int32 Lobby::obtainNewUserId() {
  return m_userIdCounter ++;
}

std::shared_ptr<Room> Lobby::getOrCreateRoom(const oatpp::String& roomName) {
  std::lock_guard<std::mutex> lock(m_roomsMutex);
  std::shared_ptr<Room>& room = m_rooms[roomName];
  if(!room) {
    room = std::make_shared<Room>(roomName);
  }
  return room;
}

// void Lobby::onGraphSocket_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket,
//   const std::shared_ptr<const ParameterMap>& params) {

//     auto roomName = params->find("roomName")->second;
//     auto nickname = params->find("nickname")->second;
//     // auto roomName = params->find("roomName")->second;
//     // auto nickname = params->find("nickname")->second;
//   if (!roomName || !nickname) return;

//   auto room = getOrCreateRoom(roomName);

//   auto userId = obtainNewUserId();
//   room->addGraphSocket(userId, socket);

//   // Optional: attach listeners
//   socket->setListener(std::make_shared<GraphSocketListener>(room, userId));
// }

void Lobby::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) {

  // auto roomName = params->find("roomName")->second;
  // auto nickname = params->find("nickname")->second;
  // auto room = getOrCreateRoom(roomName);

  // auto peer = std::make_shared<Peer>(socket, room, nickname, obtainNewUserId());
  // socket->setListener(peer);

  // room->addPeer(peer);
  // room->sendMessage(nickname + " joined " + roomName);
  // room->addClient(socket);



  auto roomName = params->find("roomName")->second;
  auto nickname = params->find("nickname")->second;



  //auto isGraph = socket->getConnection().getRequestStartingLine().find("/graph/") != std::string::npos;

//   auto typeIt = params->find("type");
// oatpp::String type = (typeIt != params->end()) ? typeIt->second : "chat"; // default to chat
//   bool isGraph = (type == "graph");
  // auto type = params->find("type") != params->end() ? params->at("type") : "chat";
  auto type = params->find("type")->second;
  if (type == "graph") {
    // onGraphSocket_NonBlocking(socket, params);
    auto room = getOrCreateRoom(roomName);

    auto userId = obtainNewUserId();

  
    // // Optional: attach listeners
    // auto graphy = std::make_shared<GraphSocketListener>(room, userId);
    auto graphy = std::make_shared<GraphListener>(socket, room, nickname, userId);
    // socket->setListener(std::make_shared<GraphSocketListener>(room, userId));

    socket->setListener(graphy);
    room->addGraphSocket(userId, socket,graphy);
    room->sendMessage(nickname + "graph joined " + roomName);
  

  } else {
    //onChatSocket_NonBlocking(socket, params); // your existing logic

    auto room = getOrCreateRoom(roomName);
  
    auto peer = std::make_shared<Peer>(socket, room, nickname, obtainNewUserId());
    socket->setListener(peer);
  
    room->addPeer(peer);
    room->sendMessage(nickname + " joined " + roomName);
    // room->addClient(socket);
  }

}

void Lobby::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket) {

  auto peer = std::static_pointer_cast<Peer>(socket->getListener());
  auto nickname = peer->getNickname();
  auto room = peer->getRoom();

  room->removePeerByUserId(peer->getUserId());

  room->sendMessage(nickname + " left the room");

  /* Remove circle `std::shared_ptr` dependencies */
  socket->setListener(nullptr);

  for (auto& pair : m_rooms) {
    pair.second->removeClient(socket);
  }

}