#ifndef SETTINGS_CONTROLLER_HPP
#define SETTINGS_CONTROLLER_HPP


#include "oatpp/macro/component.hpp"
#include "oatpp-websocket/Handshaker.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/macro/codegen.hpp"
#include "service/ble/BleService.hpp"
#include "dto/BleDtos.hpp"
#include "config/LogAdapt.hpp"

#include "config/Constants.hpp"
#include "service/ServiceRegistry.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class SettingsController : public oatpp::web::server::api::ApiController {
private:
    // typedef SettingsController __ControllerType;
    // OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, 
    //                 graphWebSocketConnHandler, "graphwebsocket");

public:
    SettingsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    
    ENDPOINT_ASYNC("POST", "/api/settings/ble/simulate", bleSimulate) {
        ENDPOINT_ASYNC_INIT(bleSimulate)
        Action act() override {
            return request->readBodyToStringAsync().callbackTo(&bleSimulate::onBodyRead);
        }
        Action onBodyRead(const oatpp::String& body) {
            if (!body) {
                auto resp = controller->createResponse(Status::CODE_400, "Empty body");
                return _return(resp);
            }
            auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
            if (!bleService) {
                auto resp = controller->createResponse(Status::CODE_500, "BLE service not available");
                return _return(resp);
            }
            bool ok = bleService->setSimulation(body);
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/settings/ble/simulate", optionsConnect) {
        ENDPOINT_ASYNC_INIT(optionsConnect)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };


    ENDPOINT_ASYNC("POST", "/api/settings/websocket/fps", bleWsFps) {
        ENDPOINT_ASYNC_INIT(bleWsFps)
        Action act() override {
            return request->readBodyToStringAsync().callbackTo(&bleWsFps::onBodyRead);
        }
        Action onBodyRead(const oatpp::String& body) {
            if (!body) {
                auto resp = controller->createResponse(Status::CODE_400, "Empty body");
                return _return(resp);
            }
            auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
            if (!bleService) {
                auto resp = controller->createResponse(Status::CODE_500, "BLE service not available");
                return _return(resp);
            }
            bool ok = bleService->webSocketMsgSetFps(body);
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/settings/websocket/fps", optionsbleWsFps) {
        ENDPOINT_ASYNC_INIT(optionsbleWsFps)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };

    
};

#include OATPP_CODEGEN_END(ApiController)
#endif // SETTINGS_CONTROLLER_HPP