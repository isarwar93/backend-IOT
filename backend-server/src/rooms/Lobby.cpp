
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
  }
  return room;
}

void Lobby::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) {


  auto roomName = params->find("roomName")->second;
  auto nickname = params->find("nickname")->second;

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
  
  OATPP_LOGi("Lobby", "onBeforeDestroy_NonBlocking");

  auto listener = socket->getListener();

  // Check if it's a chat peer
  if (auto peer = std::dynamic_pointer_cast<Peer>(listener)) {

    auto nickname = peer->getNickname();
    auto room = peer->getRoom();

    room->removePeerByUserId(peer->getUserId());
  }

  // Check if it's a graph peer
  else if (auto graphPeer = std::dynamic_pointer_cast<GraphListener>(listener)) {

    auto room = graphPeer->getRoom();
    auto userId = graphPeer->getUserId();

    room->leaveGraph(userId);

  }

  socket->setListener(nullptr);




}