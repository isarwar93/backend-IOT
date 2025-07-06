#ifndef BLECONTROLLER_HPP
#define BLECONTROLLER_HPP

#include "service/ble/BleService.hpp"
#include "dto/BleDtos.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp-websocket/Handshaker.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/macro/codegen.hpp"

#include "config/Constants.hpp"
#include "service/ServiceRegistry.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class BleController : public oatpp::web::server::api::ApiController {
public:
    BleController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    ENDPOINT_ASYNC("GET", "/api/ble/scan", scanDevices){
        ENDPOINT_ASYNC_INIT(scanDevices)
        Action act() override {
            OATPP_LOGi("BleController.hpp", "/api/ble/scan endpoint");
            auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
            if (!bleService) {
                auto resp = controller->createResponse(Status::CODE_500, "BLE service not available");
                return _return(resp);
            }
            // Timeout should be controlled from frontend
            auto list = bleService->scanDevices(2);
            // bleService->printScanResults(list);
            oatpp::List<oatpp::Object<DeviceDto>> result = oatpp::List<oatpp::Object<DeviceDto>>::createShared();
            for (const auto& d : list) {
                auto dto = DeviceDto::createShared();
                dto->mac = d.mac;
                dto->name = d.name;
                dto->rssi = d.rssi;
                result->push_back(dto);
            }
            auto resp = controller->createDtoResponse(Status::CODE_200, result);
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };

    ENDPOINT_ASYNC("POST", "/api/ble/connect", connectToDevice) {
        ENDPOINT_ASYNC_INIT(connectToDevice)
        Action act() override {
            OATPP_LOGi("BleController.hpp", "POST /api/ble/connect endpoint");
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&connectToDevice::onBodyRead);
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
            bool ok = bleService->connectToDevice(body);
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/connect", optionsConnect) {
        ENDPOINT_ASYNC_INIT(optionsConnect)
        Action act() override {
            OATPP_LOGi("BleController.hpp", "OPTIONS /api/ble/connect endpoint");
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };

    ENDPOINT_ASYNC("GET", "/api/ble/services", getServices) {
        ENDPOINT_ASYNC_INIT(getServices)
        Action act() override {
            OATPP_LOGi("BleController.hpp", "GET /api/ble/services endpoint");
            auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
            if (!bleService) {
                auto resp = controller->createResponse(Status::CODE_500, "BLE service not available");
                return _return(resp);
            }
            auto services = bleService->getServicesAndCharacteristics();
            oatpp::List<oatpp::Object<ServiceDto>> result = oatpp::List<oatpp::Object<ServiceDto>>::createShared();
            for (const auto& s : services) {
                auto sDto = ServiceDto::createShared();
                sDto->uuid = s.uuid;
                sDto->characteristics = oatpp::List<oatpp::Object<CharacteristicDto>>::createShared();

                for (const auto& c : s.characteristics) {
                    auto cDto = CharacteristicDto::createShared();
                    cDto->uuid = c.uuid;
                    cDto->value = c.value;
                    sDto->characteristics->push_back(cDto);
                }
                result->push_back(sDto);
            }
            auto resp = controller->createDtoResponse(Status::CODE_200, result);
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };



    ENDPOINT_ASYNC("POST", "/api/ble/enable", enableServices) {
        ENDPOINT_ASYNC_INIT(enableServices)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&enableServices::onBodyRead);
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

            // //TODO: May be in json
            // std::vector<std::string> selected;
            // for (const auto& uuid : *body->uuids) {
            //     selected.push_back((const char*)uuid->c_str());
            // }

            bool ok = bleService->enableServices(body);
            // bool ok = true;//bleService->connectToDevice(body);
            
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/enable", optionsEnable) {
        ENDPOINT_ASYNC_INIT(optionsEnable)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };

    // Disconnect the current ble device
    ENDPOINT_ASYNC("POST", "/api/ble/disconnect", disconnectDevice) {
        ENDPOINT_ASYNC_INIT(disconnectDevice)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&connectToDevice::onBodyRead);
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

            //TODO: May be in json
            bool ok = bleService->disconnectDevice(body);
            
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/disconnect", optionsDisconnect) {
        ENDPOINT_ASYNC_INIT(optionsDisconnect)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };


    ENDPOINT_ASYNC("POST", "/api/ble/remove", removeDevice) {
        ENDPOINT_ASYNC_INIT(removeDevice)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&connectToDevice::onBodyRead);
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

            //TODO: May be in json
            bool ok = bleService->removeDevice(body);
            
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/remove", optionsRemove) {
        ENDPOINT_ASYNC_INIT(optionsRemove)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };

    // Pair with the ble device
    ENDPOINT_ASYNC("POST", "/api/ble/pair", pairDevice) {
        ENDPOINT_ASYNC_INIT(pairDevice)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&connectToDevice::onBodyRead);
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

            //TODO: May be in json
            bool ok = bleService->pairDevice(body);
            
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/pair", optionsPair) {
        ENDPOINT_ASYNC_INIT(optionsPair)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };


    //Cancel the Pair with the ble device
    ENDPOINT_ASYNC("POST", "/api/ble/cancel-pair", cancelPairDevice) {
        ENDPOINT_ASYNC_INIT(cancelPairDevice)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&connectToDevice::onBodyRead);
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

            //TODO: May be in json
            bool ok = bleService->cancelPairing(body);
            
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/cancel-pair", optionsCancelPair) {
        ENDPOINT_ASYNC_INIT(optionsCancelPair)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };


    //Trust with the ble device
    ENDPOINT_ASYNC("POST", "/api/ble/trust", trustDevice) {
        ENDPOINT_ASYNC_INIT(trustDevice)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&trustDevice::onBodyRead);
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

            //TODO: May be in json
            bool ok = bleService->trustDevice(body);
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/trust", optionsTrust) {
        ENDPOINT_ASYNC_INIT(optionsTrust)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };



    ENDPOINT_ASYNC("GET", "/api/ble/status", getStatus) {
        ENDPOINT_ASYNC_INIT(getStatus)
        Action act() override {
            OATPP_LOGi("BleController.hpp", "GET /api/ble/getStatus endpoint");
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&getStatus::onBodyRead);
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

            oatpp::List<oatpp::Object<StatusDto>> result = oatpp::List<oatpp::Object<StatusDto>>::createShared();
            auto dto = StatusDto::createShared();
            
            dto->isConnected = bleService->isConnected(body);
            dto->isPaired = bleService->isPaired(body);
            result->push_back(dto);
          
            auto resp = controller->createDtoResponse(Status::CODE_200, result);
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };




    //Trust with the ble device
    ENDPOINT_ASYNC("POST", "/api/ble/cleanup", cleanupDevices) {
        ENDPOINT_ASYNC_INIT(cleanupDevices)
        Action act() override {
            // Read the body asynchronously
            return request->readBodyToStringAsync().callbackTo(&cleanupDevices::onBodyRead);
        }
        Action onBodyRead(const oatpp::String& body) {
            // if (!body) {
            //     auto resp = controller->createResponse(Status::CODE_400, "Empty body");
            //     return _return(resp);
            // }
            auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
            if (!bleService) {
                auto resp = controller->createResponse(Status::CODE_500, "BLE service not available");
                return _return(resp);
            }

            //TODO: May be in json
            bool ok = bleService->cleanupDisconnectedDevices();
            auto resp = controller->createDtoResponse(Status::CODE_200, oatpp::Boolean(ok));
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            return _return(resp);
        }
    };
    //For the browser to understand which headers are needed
    ENDPOINT_ASYNC("OPTIONS", "/api/ble/cleanup", optionsCleanup) {
        ENDPOINT_ASYNC_INIT(optionsCleanup)
        Action act() override {
            auto resp = controller->createResponse(Status::CODE_200, "");
            resp->putHeader("Access-Control-Allow-Origin", "http://localhost:5173");
            resp->putHeader("Access-Control-Allow-Headers", "Content-Type");
            return _return(resp);
        }
    };

    //TODO: Completely delete the BLE class
    // ENDPOINT("POST", "/api/ble/turnoff", cleanup) {
    //     m_bleService->cleanupDisconnectedDevices();
    //     return createResponse(Status::CODE_200, "Cleanup started");
    // }

    //TODO: Create the ble class 
    // ENDPOINT("POST", "/api/ble/turnon", cleanup) {
    //     m_bleService->cleanupDisconnectedDevices();
    //     return createResponse(Status::CODE_200, "Cleanup started");
    // }
};

#include OATPP_CODEGEN_END(ApiController)
#endif // BLECONTROLLER_HPP