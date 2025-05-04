#ifndef GRAPHCONTROLLER_HPP
#define GRAPHCONTROLLER_HPP

#include "oatpp-websocket/Handshaker.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

class GraphController : public oatpp::web::server::api::ApiController {
private:
    typedef GraphController __ControllerType;
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");

public:
        GraphController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
            : oatpp::web::server::api::ApiController(objectMapper)
        {}
        

        ENDPOINT_ASYNC("GET", "ws/graph/{username}/*", WSGraph) {
        ENDPOINT_ASYNC_INIT(WSGraph)

        Action act() override {
            auto username = request->getPathVariable("username");
            auto nickname = request->getQueryParameter("nickname");

            OATPP_ASSERT_HTTP(nickname, Status::CODE_400, "No nickname specified.");

            auto response = oatpp::websocket::Handshaker::serversideHandshake(
                request->getHeaders(), controller->websocketConnectionHandler);

            auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
            (*parameters)["username"] = username;
            (*parameters)["nickname"] = nickname;
            (*parameters)["type"] = "graph";

            response->setConnectionUpgradeParameters(parameters);

            return _return(response);
        }
    };
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif // GRAPHCONTROLLER_HPP