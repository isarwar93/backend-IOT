#include "websocket/BaseSocketListener.hpp"

#include "model/Room.hpp"
#include "./AppComponent.hpp"


oatpp::async::CoroutineStarter BaseSocketListener::onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
    return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}
  
oatpp::async::CoroutineStarter BaseSocketListener::onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
    return nullptr; // do nothing
}
  
oatpp::async::CoroutineStarter BaseSocketListener::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) {
    return nullptr; // do nothing
}
  