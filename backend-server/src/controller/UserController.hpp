#ifndef USERCONTROLLER_HPP
#define USERCONTROLLER_HPP

#include "oatpp-websocket/Handshaker.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "service/AuthService.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

class UserController : public oatpp::web::server::api::ApiController {
private:
    typedef UserController __ControllerType;
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");

public:
    UserController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper)
    {}

    ENDPOINT_ASYNC("POST", "/login", Login) {
    ENDPOINT_ASYNC_INIT(Login)

        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&Login::onBodyRead);
        }

        Action onBodyRead(const oatpp::String& body) {
            if (!body) {
                auto resp = controller->createResponse(Status::CODE_400, "Empty body");
                resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
                resp->putHeader("Access-Control-Allow-Credentials", "true");
                return _return(resp);
            }

            auto usernamePos = body->find("username=");
            auto passwordPos = body->find("&password=");

            if (usernamePos == -1 || passwordPos == -1) {
                auto resp = controller->createResponse(Status::CODE_400, "Bad Request");
                resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
                resp->putHeader("Access-Control-Allow-Credentials", "true");
                return _return(resp);
            }

            auto username = body->substr(usernamePos + 9, passwordPos - (usernamePos + 9));
            auto password = body->substr(passwordPos + 10);

            AuthService authService;

            if (authService.authenticate(username, password)) {
                auto resp = controller->createResponse(Status::CODE_200, "Login Successful");
                resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
                resp->putHeader("Access-Control-Allow-Credentials", "true");
                return _return(resp);
            } else {
                auto resp = controller->createResponse(Status::CODE_401, "Unauthorized");
                resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
                resp->putHeader("Access-Control-Allow-Credentials", "true");
                return _return(resp);
            }
        }
    };

    // ENDPOINT_ASYNC("GET", "/login", getLogin) {
    //     ENDPOINT_ASYNC_INIT(getLogin)
    //     Action act() override {
    //         OATPP_LOGi("UserController.hpp", "GET login endpoint");
    //         auto resp = controller->createResponse(Status::CODE_405, "Use POST for login");
    //         resp->putHeader("Content-Type", "text/plain");
    //         resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    //         // resp->putHeader("Access-Control-Allow-Credentials", "true");
    //         return _return(resp);
    //     }
    // };

};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif // GRAPHCONTROLLER_HPP