#include "websocket/WSComm.hpp"
#include "model/Room.hpp"
#include "./AppComponent.hpp"
#include <thread>
#include <chrono>

WSComm::~WSComm(){
    // Clear the socket reference to prevent further sends
    m_socket = nullptr;
    
    // Give a brief moment for any pending async operations to complete
    // This prevents "pure virtual method called" errors
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void WSComm::sendMessage(const oatpp::String& message) {
    // Check if socket is still valid before sending
    if (!m_socket) {
        return; // Socket already destroyed, skip send
    }
    
    class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine> {
    private:
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        oatpp::String m_message;
    public:
        SendMessageCoroutine(oatpp::async::Lock* lock,
                           const std::shared_ptr<AsyncWebSocket>& websocket,
                           const oatpp::String& message)
            : m_lock(lock)
            , m_websocket(websocket)
            , m_message(message)
            {}
        Action act() override {
            // Check if websocket is still valid
            if (!m_websocket) {
                return finish();
            }
            return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
        }
    };
    
    if (m_asyncExecutor) {
        m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, message);
    }
}
  
oatpp::async::CoroutineStarter WSComm::readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {
    if(size == 0) { // message transfer finished
        auto wholeMessage = m_messageBuffer.toString();
        m_messageBuffer.setCurrentPosition(0);
        OATPP_LOGi("Graph listener", "readMessage() - Received message: {}", wholeMessage->c_str());
        //m_room->sendMessage(m_nickname + ": " + wholeMessage);
    } else if(size > 0) { // message frame received
        m_messageBuffer.writeSimple(data, size);
    }
    return nullptr; // do nothing
}

oatpp::String WSComm::getNickname() {
    return m_nickname;
}

v_int32 WSComm::getUserId() {
    return m_userId;
}