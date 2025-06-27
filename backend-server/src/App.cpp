#include "controller/UserController.hpp"
#include "controller/GraphController.hpp"
#include "controller/RoomsController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"
#include "CorsInterceptor.hpp"

// #include "ble/ble.hpp"
#include "service/ServiceManager.hpp"

#include "config/Constants.hpp"
#include "service/ServiceRegistry.hpp"
#include "service/ble/BleService.hpp"
#include "service/MqttService.hpp"

#include <iostream>
#include <chrono>

void run() {
    /* Register Components in scope of run() method */
    AppComponent components;

    /* Create CorsInterceptor */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::interceptor::RequestInterceptor>, requestInterceptor)([] {
        return std::make_shared<CorsInterceptor>();
    }());

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    /* Create UserController and add all of its endpoints to router */
    router->addController(std::make_shared<UserController>());

    /* Create GraphController and add all of its endpoints to router */
    router->addController(std::make_shared<GraphController>());

    /* Create RoomsController and add all of its endpoints to router */
    router->addController(std::make_shared<RoomsController>());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    OATPP_LOGi("MyApp", "Server running on port {}", connectionProvider->getProperty("port").toString());

    /* Run server */
    server.run();
}

int main(int argc, const char * argv[]) {
    oatpp::Environment::init();

    OATPP_LOGi("MyApp", "Backend Main program started");
    ServiceRegistry::instance().registerService("ble", std::make_shared<BleService>());
    ServiceRegistry::instance().registerService("mqtt", std::make_shared<MqttService>());

    auto bleService = USE_SRVC("ble");
    bleService->start();
// TODO fix this
    if (bleService && bleService->isRunning()) {
        //std::cout << "BLE is running!\n";
        //bleService->sendCommand("reset");
    }
    else
    {
        OATPP_LOGi("MyApp", "Ble not running");
    }

    run();

    oatpp::Environment::destroy();

    return 0;
}
