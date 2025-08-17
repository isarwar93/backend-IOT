#ifndef GRAPHWEBSOCKET_HPP
#define GRAPHWEBSOCKET_HPP

#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/async/Lock.hpp"
#include "oatpp/async/Executor.hpp"
#include "oatpp/macro/component.hpp"
#include "websocket/BaseSocketListener.hpp"
#include "oatpp-websocket/AsyncConnectionHandler.hpp"

class GraphWebSocket : public BaseSocketListener, 
                      public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener 
{
private:

    std::atomic<v_int32> m_userIdCounter;
    // std::unordered_map<oatpp::String, std::shared_ptr<Room>> m_rooms;
    std::mutex m_roomsMutex;
    std::unordered_map<oatpp::String, v_int32> m_nicknameToUserId;
    v_int32 globGraphId;
    std::shared_ptr<oatpp::websocket::AsyncWebSocket> globGraphSocket;
    oatpp::String globGraphName;

    /*
    * Buffer for messages. Needed for multi-frame messages.
    */
    oatpp::data::stream::BufferOutputStream globMsgBuffer;

    /**
     *   * Lock for synchronization of writes to the web socket.
    */
    oatpp::async::Lock globGraphWriteLock;
    
    /**
     * Inject async executor object.
    */
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutorGraph);

public:

    GraphWebSocket():m_userIdCounter(0){}
    // GraphListener(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
    //     const std::shared_ptr<Room>& room,
    //     const oatpp::String& nickname,
    //     v_int32 userId)
    //     : m_socket(socket)
    //     , m_room(room)
    //     , m_nickname(nickname)
    //     , m_userId(userId)
    //     {}
    ~GraphWebSocket();

    // const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket;
    // const oatpp::String& graphName;
    // v_int32 graphId;

    /**
     *  Called when socket is created
     */
    void onAfterCreate_NonBlocking(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) override;

    /**
     *  Called before socket instance is destroyed.
     */
    void onBeforeDestroy_NonBlocking(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket) override;
  
    /**
     * Get Graph Name.
     * @return
    */
    oatpp::String getGraphName();

    /**
     * Get graph Id.
     * @return
    */
    v_int32 getGraphId();
  
    /**
      * Sendmessage
      * @return
    */
    void sendMessage(const oatpp::String& message);

    /**
      * Readmessage
      * @return
    */
    CoroutineStarter readMessage(const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket, 
                  v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
};

#endif /* GRAPHWEBSOCKET_HPP */