#ifndef BASE_SOCKET_LISTENER_HPP
#define BASE_SOCKET_LISTENER_HPP


#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/async/Lock.hpp"
#include "oatpp/async/Executor.hpp"
#include "oatpp/macro/component.hpp"


class BaseSocketListener : public oatpp::websocket::AsyncWebSocket::Listener {
public:
    BaseSocketListener() = default;
    virtual ~BaseSocketListener() = default;

    /**
     * Lock for synchronization of writes to the web socket.
     */
    oatpp::async::Lock m_writeLock;
    
    CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
    CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
    CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) override;
};

#endif // BASE_SOCKET_LISTENER_HPP
