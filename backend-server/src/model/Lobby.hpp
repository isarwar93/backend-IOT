
#ifndef ASYNC_SERVER_ROOMS_LOBBY_HPP
#define ASYNC_SERVER_ROOMS_LOBBY_HPP

#include "./Room.hpp"

#include "oatpp-websocket/AsyncConnectionHandler.hpp"

#include <unordered_map>
#include <mutex>



class Lobby : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {

    public:

    Lobby()
    : m_userIdCounter(0)
    {}
    /**
     * Generate id for new user
     * @return
     */
    v_int32 obtainNewUserId();

    /**
     * Get room by name or create new one if not exists.
     * @param roomName
     * @return
     */
    std::shared_ptr<Room> getOrCreateRoom(const oatpp::String& roomName);
    void onGraphSocket_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket,
                              const std::shared_ptr<const ParameterMap>& params);

    /**
     *  Called when socket is created
     */
    void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) override;

    /**
     *  Called before socket instance is destroyed.
     */
    void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket) override;

    std::atomic<v_int32> m_userIdCounter;
    std::unordered_map<oatpp::String, std::shared_ptr<Room>> m_rooms;
    std::mutex m_roomsMutex;
    std::unordered_map<oatpp::String, v_int32> m_nicknameToUserId;

};

#endif //ASYNC_SERVER_ROOMS_LOBBY_HPP
