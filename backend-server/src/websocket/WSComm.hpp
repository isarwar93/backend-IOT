#ifndef WSComm_HPP
#define WSComm_HPP

#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/async/Lock.hpp"
#include "oatpp/async/Executor.hpp"
#include "oatpp/macro/component.hpp"
#include "websocket/BaseSocketListener.hpp"


class WSComm : public BaseSocketListener {
private:
    // std::shared_ptr<Room> m_room;
    v_int32 m_userId;

    std::shared_ptr<AsyncWebSocket> m_socket;
    oatpp::String m_nickname;

    /**
    * Buffer for messages. Needed for multi-frame messages.
    */
    oatpp::data::stream::BufferOutputStream m_messageBuffer;

    /**
     *   * Lock for synchronization of writes to the web socket.
    */
    oatpp::async::Lock m_writeLock;
    
    /**
     * Inject async executor object.
    */
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

public:

    WSComm(const std::shared_ptr<AsyncWebSocket>& socket,
        const oatpp::String& nickname,
        v_int32 userId)
        : m_socket(socket)
        , m_nickname(nickname)
        , m_userId(userId)
        {}

    ~WSComm();
  
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
      * Sendmessage
      * @return
    */
  
    void sendMessage(const oatpp::String& message);

    /**
      * Readmessage
      * @return
    */

    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
};

#endif /* WSComm_HPP */