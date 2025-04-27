#ifndef PEER_HPP
#define PEER_HPP

#include "websocket/BaseSocketListener.hpp"

#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/async/Lock.hpp"
#include "oatpp/async/Executor.hpp"
#include "oatpp/macro/component.hpp"

class Room; // FWD

class Peer : public BaseSocketListener {
private:
    /**
     * Buffer for messages. Needed for multi-frame messages.
     */
    oatpp::data::stream::BufferOutputStream m_messageBuffer;

    /**
     * Lock for synchronization of writes to the web socket.
     */
    oatpp::async::Lock m_writeLock;

    std::shared_ptr<AsyncWebSocket> m_socket;
    std::shared_ptr<Room> m_room;
    oatpp::String m_nickname;
    v_int32 m_userId;

    /**
     * Inject async executor object.
     */
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

public:

    Peer(const std::shared_ptr<AsyncWebSocket>& socket,
        const std::shared_ptr<Room>& room,
        const oatpp::String& nickname,
        v_int32 userId)
      : m_socket(socket)
      , m_room(room)
      , m_nickname(nickname)
      , m_userId(userId)
    {}

    /**
     * Get room of the peer.
     * @return
     */
    std::shared_ptr<Room> getRoom();

    /**
     * Get peer nickname.
     * @return
     */
    oatpp::String getNickname();

    /**
     * Get peer userId.
     * @return
     */
    v_int32 getUserId();

    /**
     * Send message  (to user).
     * @param message
     */
    void sendMessage(const oatpp::String& message);

    /**
     * Read message  (from user).
     * @param message
     */
    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

};


#endif //PEER_HPP


