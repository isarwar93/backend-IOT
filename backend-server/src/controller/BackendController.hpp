//REST API Controller with oatpp 

#ifndef BACKENDCONTROLLER_HPP
#define BACKENDCONTROLLER_HPP

// #include "core/SerManager.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class BackendController : public oatpp::web::server::api::ApiController {
private:
    BackendManager& BackendManager;

public:
    BackendController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper),
                      BackendManager& sm)
        : oatpp::web::server::api::ApiController(objectMapper), BackendManager(sm) {}

    ENDPOINT("POST", "/Backend/start/{name}", startBackend,
             PATH(String, name)) {
        try {
            BackendManager.startBackend(name->std_str());
            return createResponse(Status::CODE_200, "Started");
        } catch (...) {
            return createResponse(Status::CODE_500, "Start failed");
        }
    }

    ENDPOINT("POST", "/Backend/stop/{name}", stopBackend,
             PATH(String, name)) {
        try {
            BackendManager.stopBackend(name->std_str());
            return createResponse(Status::CODE_200, "Stopped");
        } catch (...) {
            return createResponse(Status::CODE_500, "Stop failed");
        }
    }

    ENDPOINT("POST", "/Backend/configure/{name}", configureBackend,
             PATH(String, name), BODY_STRING(String, config)) {
        try {
            bool result = BackendManager.configureBackend(name->std_str(), config->std_str());
            return createResponse(Status::CODE_200, result ? "Configured" : "Invalid config");
        } catch (...) {
            return createResponse(Status::CODE_500, "Config error");
        }
    }

    ENDPOINT("POST", "/Backend/command/{name}", commandBackend,
             PATH(String, name), BODY_STRING(String, cmd)) {
        try {
            std::string result = BackendManager.sendCommandToBackend(name->std_str(), cmd->std_str());
            return createResponse(Status::CODE_200, result);
        } catch (...) {
            return createResponse(Status::CODE_500, "Command error");
        }
    }

    ENDPOINT("GET", "/Backend/status/{name}", getStatus,
             PATH(String, name)) {
        try {
            std::string statusJson = BackendManager.getBackendStatusJson(name->std_str());
            return createResponse(Status::CODE_200, statusJson);
        } catch (...) {
            return createResponse(Status::CODE_500, "Status error");
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif // BackendCONTROLLER_HPP
