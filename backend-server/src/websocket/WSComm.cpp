#include "websocket/WSComm.hpp"
#include "model/Room.hpp"
#include "./AppComponent.hpp"

WSComm::~WSComm(){

    // OATPP_LOGi("Graph listener", "Class destroyed");
}

void WSComm::sendMessage(const oatpp::String& message) {
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
            return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
        }
    };
    m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, message);
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

// std::shared_ptr<Room> WSComm::getRoom() {
//     return m_room;
// }

oatpp::String WSComm::getNickname() {
    return m_nickname;
}

v_int32 WSComm::getUserId() {
    // OATPP_LOGi("Graph listener", "getUserId: {}",m_userId);
    return m_userId;
}