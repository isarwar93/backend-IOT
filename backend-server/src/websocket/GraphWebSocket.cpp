#include "websocket/GraphWebSocket.hpp"
#include "config/LogAdapt.hpp"
#include "service/ble/BleService.hpp"
#include "config/Constants.hpp"
#include "websocket/WSComm.hpp"

GraphWebSocket::~GraphWebSocket(){
    // Cleanup resources if needed
    LOGI("Destructor called, cleaning up resources.");
}



void GraphWebSocket::sendMessage(const oatpp::String& message) {
    class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine> {
    private:
        oatpp::async::Lock* m_lock;
        std::shared_ptr<oatpp::websocket::AsyncWebSocket> m_websocket;
        oatpp::String m_message;
    public:
        SendMessageCoroutine(oatpp::async::Lock* lock,
                           const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& websocket,
                           const oatpp::String& message)
            : m_lock(lock)
            , m_websocket(websocket)
            , m_message(message)
            {}
        Action act() override {
            return oatpp::async::synchronize(m_lock, 
                m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
        }
    };
    LOGI("GraphWebSocket::sendMessage() - Sending message: {}", message->c_str());
    m_asyncExecutorGraph->execute<SendMessageCoroutine>(&globGraphWriteLock, globGraphSocket, message);
}
  
oatpp::async::CoroutineStarter GraphWebSocket::readMessage(
                const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket, 
                v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {
    LOGI("GraphWebSocket::readMessage() - Received message with opcode: {}", opcode);
    if(size == 0) { // message transfer finished
        auto wholeMessage = globMsgBuffer.toString();
        globMsgBuffer.setCurrentPosition(0);
        OATPP_LOGi("Graph listener", "readMessage() - Received message: {}", wholeMessage->c_str());
        //m_room->sendMessage(m_nickname + ": " + wholeMessage);
    } else if(size > 0) { // message frame received
        globMsgBuffer.writeSimple(data, size);
    }
    return nullptr; // do nothing
}

oatpp::String GraphWebSocket::getGraphName() {
    return globGraphName;
}

v_int32 GraphWebSocket::getGraphId() {
    // OATPP_LOGi("Graph listener", "getUserId: {}",m_userId);
    return globGraphId;
}


// Override WebSocket lifecycle methods:
// onAfterCreate_NonBlocking - called when a new WebSocket connection is established.
// onBeforeDestroy_NonBlocking - called just before a WebSocket connection is destroyed.
// These methods are called by the oatpp framework to manage connection setup and cleanup.

void GraphWebSocket::onAfterCreate_NonBlocking(
                        const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket, 
                        const std::shared_ptr<const ParameterMap>& params) {
    if (!params) {
        LOGI("params must not be null");
        return;
    }

    auto it = params->find("mac");
    if (it != params->end()) {
        //PASS
    } else {
        LOGI("mac not found in params");
        return;
    }
    auto mac = it->second;

    it = params->find("uuid");
    if (it != params->end()) {
        //PASS
    } else {
        LOGI("uuid not found in params");
        return;
    }

    auto uuid = it->second;

    LOGI(" MAC: {}",mac);
    LOGI(" uuid: {}",uuid);  


    // // assign unique id to each nick
    // v_int32 userId;
    // {
    //     std::lock_guard<std::mutex> lock(m_roomsMutex);
    //     auto it = m_nicknameToUserId.find(nickname);
    //     if (it != m_nicknameToUserId.end()) {
    //         userId = it->second;
    //     } else {
    //         userId = obtainNewUserId();
    //         m_nicknameToUserId[nickname] = userId;
    //     }
    // }

    // if (type == "graph") {
    //     auto room = getOrCreateRoom(roomName);
    //     auto userId = obtainNewUserId();
    // SOCKETS ++;
    //auto graph = std::make_shared<GraphWebSocket>();//(socket, room, nickname, userId);
    //socket->setListener(graph);
    //For graphs, we can create a thread for streaming
    // This graphwebsocket will be having complete solution for streaming graphs
    // inject your service in this class and get graph data out on websocket
    //globGraphSocket = socket;
    //globGraphSocket = socket;
    auto graph = std::make_shared<WSComm>(socket, mac, 1);
    socket->setListener(graph);
    //room->addGraphSocket(userId, socket,graph);

    // globGraphName = mac + "graph";
    // globGraphId = obtainNewUserId();
    //LOGI("GraphWebSocket::onAfterCreate_NonBlocking - Graph Name: {}, Graph ID: {}", globGraphName->c_str(), globGraphId);
    
    auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
    // auto room = getOrCreateRoom(globGraphName);

    bleService->addGraphSocket(getGraphId(), socket, graph);
}

void GraphWebSocket::onBeforeDestroy_NonBlocking(
    const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket) {
    auto listener = socket->getListener();
    LOGI("GraphWebSocket::onBeforeDestroy_NonBlocking - Listener: {}", listener ? "exists" : "null");
    
    if (!listener) return;

    auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
    bleService->leaveGraph(getGraphId());
    socket->setListener(nullptr);
}
