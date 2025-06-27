//REST API Controller with oatpp 

#ifndef SERVICECONTROLLER_HPP
#define SERVICECONTROLLER_HPP

#include "core/ServiceManager.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ServiceController : public oatpp::web::server::api::ApiController {
private:
    ServiceManager& serviceManager;

public:
    ServiceController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper),
                      ServiceManager& sm)
        : oatpp::web::server::api::ApiController(objectMapper), serviceManager(sm) {}

    ENDPOINT("POST", "/service/start/{name}", startService,
             PATH(String, name)) {
        try {
            serviceManager.startService(name->std_str());
            return createResponse(Status::CODE_200, "Started");
        } catch (...) {
            return createResponse(Status::CODE_500, "Start failed");
        }
    }

    ENDPOINT("POST", "/service/stop/{name}", stopService,
             PATH(String, name)) {
        try {
            serviceManager.stopService(name->std_str());
            return createResponse(Status::CODE_200, "Stopped");
        } catch (...) {
            return createResponse(Status::CODE_500, "Stop failed");
        }
    }

    ENDPOINT("POST", "/service/configure/{name}", configureService,
             PATH(String, name), BODY_STRING(String, config)) {
        try {
            bool result = serviceManager.configureService(name->std_str(), config->std_str());
            return createResponse(Status::CODE_200, result ? "Configured" : "Invalid config");
        } catch (...) {
            return createResponse(Status::CODE_500, "Config error");
        }
    }

    ENDPOINT("POST", "/service/command/{name}", commandService,
             PATH(String, name), BODY_STRING(String, cmd)) {
        try {
            std::string result = serviceManager.sendCommandToService(name->std_str(), cmd->std_str());
            return createResponse(Status::CODE_200, result);
        } catch (...) {
            return createResponse(Status::CODE_500, "Command error");
        }
    }

    ENDPOINT("GET", "/service/status/{name}", getStatus,
             PATH(String, name)) {
        try {
            std::string statusJson = serviceManager.getServiceStatusJson(name->std_str());
            return createResponse(Status::CODE_200, statusJson);
        } catch (...) {
            return createResponse(Status::CODE_500, "Status error");
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif // SERVICECONTROLLER_HPP
