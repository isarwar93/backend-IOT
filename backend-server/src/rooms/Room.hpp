
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
    mutable std::mutex m_peerByIdLock;

    std::unordered_map<v_int32, std::shared_ptr<oatpp::websocket::AsyncWebSocket>> m_graphClients;
    std::unordered_map<v_int32, std::shared_ptr<GraphListener>> m_graphById;

    std::thread m_graphThread;
    std::atomic<bool> m_graphRunning = false;
    mutable std::mutex m_graphMutex;
    std::condition_variable m_cv;

    
public:
    Room(const oatpp::String& name);
    ~Room();

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


    void addGraphSocket(v_int32 userId, 
        const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
        const std::shared_ptr<GraphListener>& graphListener);
        
    void leaveGraph(v_int32 userId);

    void streamGraph();

    bool isEmpty() const;

    oatpp::String getName() const; 
};

#endif //ASYNC_SERVER_ROOMS_ROOM_HPP
