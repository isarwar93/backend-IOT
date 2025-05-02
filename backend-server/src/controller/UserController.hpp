// #ifndef USERCONTROLLER_HPP
// #define USERCONTROLLER_HPP

// #include "service/AuthService.hpp"


// #include "oatpp-websocket/Handshaker.hpp"
// #include "oatpp/web/server/api/ApiController.hpp"
// #include "oatpp/network/ConnectionHandler.hpp"
// #include "oatpp/macro/codegen.hpp"
// #include "oatpp/macro/component.hpp"

// #include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

// class UserController : public oatpp::web::server::api::ApiController {
// private:
//   typedef UserController __ControllerType;
// public:
//   UserController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
//     : oatpp::web::server::api::ApiController(objectMapper)
//   {}

//   ENDPOINT("POST", "/login", login,
//            BODY_STRING(String, body))
//   {
//     auto usernamePos = body->find("username=");
//     auto passwordPos = body->find("&password=");

//     if (usernamePos == -1 || passwordPos == -1) {
//         return createResponse(Status::CODE_400, "Bad Request");
//       }

      
//       auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);
//       response->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");

//     auto username = body->substr(usernamePos + 9, passwordPos - (usernamePos + 9));
//     auto password = body->substr(passwordPos + 10);

//     AuthService authService;

//     if (authService.authenticate(username, password)) {
//       return createResponse(Status::CODE_200, "Login Successful");
//     } else {
//       return createResponse(Status::CODE_401, "Unauthorized");
//     }
//   }

//   ENDPOINT("GET", "/whoami", whoami,
//            QUERY(String, username, "username"))
//   {
//     if (!username) {
//       return createResponse(Status::CODE_400, "Username query parameter missing");
//     }

//     auto responseBody = "You are: " + *username;
//     return createResponse(Status::CODE_200, responseBody);
//   }
// };

// #include OATPP_CODEGEN_END(ApiController) //<-- codegen end

// #endif // USERCONTROLLER_HPP


// #ifndef USERCONTROLLER_HPP
// #define USERCONTROLLER_HPP

// #include "service/AuthService.hpp"

// #include "oatpp-websocket/Handshaker.hpp"

// #include "oatpp/web/server/api/ApiController.hpp"

// #include "oatpp/network/ConnectionHandler.hpp"

// #include "oatpp/macro/codegen.hpp"
// #include "oatpp/macro/component.hpp"

// #include "oatpp/web/server/api/ApiController.hpp"
// // #include "oatpp/web/server/AsyncEndpoint.hpp"
// #include "oatpp/network/ConnectionHandler.hpp"
// #include "oatpp/macro/codegen.hpp"
// #include "oatpp/macro/component.hpp"

// #include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

// class UserController : public oatpp::web::server::api::ApiController {
// private:
//   typedef UserController __ControllerType;
// public:
//   UserController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
//     : oatpp::web::server::api::ApiController(objectMapper)
//   {}

//     ENDPOINT_ASYNC("POST", "/login", httpLogin) {
//         ENDPOINT_ASYNC_INIT(httpLogin)

//         Action onBodyRead(const oatpp::String& body) {
//             if (!body) {
//                 auto resp = controller->createResponse(Status::CODE_400, "Empty body");
//                 resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//                 resp->putHeader("Access-Control-Allow-Credentials", "true");
//                 return _return(resp);
//             }
    
//             auto usernamePos = body->find("username=");
//             auto passwordPos = body->find("&password=");
    
//             if (usernamePos == -1 || passwordPos == -1) {
//             auto resp = controller->createResponse(Status::CODE_400, "Bad Request");
//             resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//             resp->putHeader("Access-Control-Allow-Credentials", "true");
//             return _return(resp);
//             }
    
//             auto username = body->substr(usernamePos + 9, passwordPos - (usernamePos + 9));
//             auto password = body->substr(passwordPos + 10);
    
//             AuthService authService;
    
//             if (authService.authenticate(username, password)) {
//             auto resp = controller->createResponse(Status::CODE_200, "Login Successful");
//             resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//             resp->putHeader("Access-Control-Allow-Credentials", "true");
//             return _return(resp);
//             } else {
//             auto resp = controller->createResponse(Status::CODE_401, "Unauthorized");
//             resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//             resp->putHeader("Access-Control-Allow-Credentials", "true");
//             return _return(resp);
//             }
//         }
//     };
// };

// #include OATPP_CODEGEN_END(ApiController) //<-- codegen end

// #endif // USERCONTROLLER_HPP


// #ifndef USERCONTROLLER_HPP
// #define USERCONTROLLER_HPP

// #include "service/AuthService.hpp"

// #include "oatpp/web/server/api/ApiController.hpp"
// #include "oatpp-web/server/AsyncEndpoint.hpp"
// #include "oatpp/network/ConnectionHandler.hpp"
// #include "oatpp/macro/codegen.hpp"
// #include "oatpp/macro/component.hpp"

// #include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

// class UserController : public oatpp::web::server::api::ApiController {
// private:
//     typedef UserController __ControllerType;
// public:
//     UserController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
//       : oatpp::web::server::api::ApiController(objectMapper)
//     {}

//     ENDPOINT_ASYNC("POST", "/login") {
//         ENDPOINT_ASYNC_INIT(Login)

//         Action act() override {
//           // Read the body asynchronously
//           return request->readBodyToStringAsync().callbackTo(&Login::onBodyRead);
//         }

//         Action onBodyRead(const oatpp::String& body) {
//             if (!body) {
//               auto resp = controller->createResponse(Status::CODE_400, "Empty body");
//               resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//               resp->putHeader("Access-Control-Allow-Credentials", "true");
//               return _return(resp);
//             }

//             auto usernamePos = body->find("username=");
//             auto passwordPos = body->find("&password=");

//             if (usernamePos == -1 || passwordPos == -1) {
//               auto resp = controller->createResponse(Status::CODE_400, "Bad Request");
//               resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//               resp->putHeader("Access-Control-Allow-Credentials", "true");
//               return _return(resp);
//             }

//             auto username = body->substr(usernamePos + 9, passwordPos - (usernamePos + 9));
//             auto password = body->substr(passwordPos + 10);

//             AuthService authService;

//             if (authService.authenticate(username, password)) {
//                 auto resp = controller->createResponse(Status::CODE_200, "Login Successful");
//                 resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//                 resp->putHeader("Access-Control-Allow-Credentials", "true");
//                 return _return(resp);
//             } else {
//                 auto resp = controller->createResponse(Status::CODE_401, "Unauthorized");
//                 resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//                 resp->putHeader("Access-Control-Allow-Credentials", "true");
//                 return _return(resp);
//             }
//         }
//     };

//     ENDPOINT("GET", "/whoami", whoami,
//         QUERY(String, username, "username"))
//     {
//         if (!username) {
//             return createResponse(Status::CODE_400, "Username query parameter missing");
//         }

//         auto responseBody = "You are: " + *username;
//         auto resp = createResponse(Status::CODE_200, responseBody);
//         resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
//         resp->putHeader("Access-Control-Allow-Credentials", "true");
//         return resp;
//     }

// };

// #include OATPP_CODEGEN_END(ApiController) //<-- codegen end

// #endif // USERCONTROLLER_HPP




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

        // Action act() override {
        //     auto roomName = request->getPathVariable("room-name");
        //     auto nickname = request->getQueryParameter("nickname");

        //     OATPP_ASSERT_HTTP(nickname, Status::CODE_400, "No nickname specified.");

        //     auto response = oatpp::websocket::Handshaker::serversideHandshake(
        //         request->getHeaders(), controller->websocketConnectionHandler);

        //     auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
        //     (*parameters)["roomName"] = roomName;
        //     (*parameters)["nickname"] = nickname;
        //     (*parameters)["type"] = "graph";

        //     response->setConnectionUpgradeParameters(parameters);

        //     return _return(response);




        // }
    };
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif // GRAPHCONTROLLER_HPP