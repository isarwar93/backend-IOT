#ifndef RoomsController_hpp
#define RoomsController_hpp

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "./AppComponent.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

/**
 * Controller with WebSocket-connect endpoint.
 */
class RoomsController : public oatpp::web::server::api::ApiController {
private:
    typedef RoomsController __ControllerType;
private:
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");
public:
    RoomsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
    {}
public:
  
    ENDPOINT_ASYNC("GET", "/", Root) {    
    ENDPOINT_ASYNC_INIT(Root)

    const char* pageTemplate =
        "<html lang='en'>"
          "<head>"
            "<meta charset=utf-8/>"
          "</head>"
          "<body>"
            "<p>Hello Async WebSocket Rooms Server!</p>"
            "<p>Connect to chat room:</p>"
            "<code>localhost:8000/ws/chat/{room_name}/?nickname={nickname}</code>"
          "</body>"
        "</html>";

        Action act() override {
            OATPP_LOGi("Roomscontroller", "accessing api");
        return _return(controller->createResponse(Status::CODE_200, pageTemplate));
        }
      
    };

    ENDPOINT_ASYNC("GET", "ws/chat/{username}/*", WS) {

        ENDPOINT_ASYNC_INIT(WS)

        Action act() override {

            auto username = request->getPathVariable("username");
            auto nickname = request->getQueryParameter("nickname");

            OATPP_ASSERT_HTTP(nickname, Status::CODE_400, "No nickname specified.");

            /* Websocket handshake */
            auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);

            auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();

            (*parameters)["username"] = username;
            (*parameters)["nickname"] = nickname;
            (*parameters)["type"] = "chat"; // Flag for chat

            /* Set connection upgrade params */
            response->setConnectionUpgradeParameters(parameters);

            return _return(response);

        }
    };
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif /* RoomsController_hpp */
