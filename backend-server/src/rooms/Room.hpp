
#ifndef ASYNC_SERVER_ROOMS_ROOM_HPP
#define ASYNC_SERVER_ROOMS_ROOM_HPP

#include "./Peer.hpp"
#include "./GraphListener.hpp"
#include <unordered_map>

#include <unordered_set>
#include <mutex>
#include <thread>
#include <atomic>

class Room {
private:
  oatpp::String m_name;
  std::unordered_map<v_int32, std::shared_ptr<Peer>> m_peerById;
  std::mutex m_peerByIdLock;



  //std::mutex m_lock;
  std::unordered_set<std::shared_ptr<oatpp::websocket::AsyncWebSocket>> m_clients;
  std::thread m_thread;
  std::atomic<bool> m_running{false};


  std::unordered_map<v_int32, std::shared_ptr<oatpp::websocket::AsyncWebSocket>> m_graphClients;
  std::unordered_map<v_int32, std::shared_ptr<GraphListener>> m_graphById;
  std::mutex m_graphMutex;
  std::thread m_graphThread;
  std::atomic<bool> m_graphRunning = false;
public:

  Room(const oatpp::String& name)
    : m_name(name)
  {}

  /**
   * Add peer to the room.
   * @param peer
   */
  void addPeer(const std::shared_ptr<Peer>& peer);

  /**
   * Remove peer from the room.
   * @param userId
   */
  void removePeerByUserId(v_int32 userId);

  /**
   * Send message to all peers in the room.
   * @param message
   */
  void sendMessage(const oatpp::String& message);


 
  void addClient(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket);

  void removeClient(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket);

  ~Room();

  //void addGraphSocket(v_int32 userId, const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket);
  void addGraphSocket(v_int32 userId, 
    const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
    const std::shared_ptr<GraphListener>& graphListener);
    
  void leaveGraph(v_int32 userId);

  void streamGraph();
};

#endif //ASYNC_SERVER_ROOMS_ROOM_HPP
