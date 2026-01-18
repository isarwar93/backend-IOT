#include "controller/UserController.hpp"
#include "controller/GraphController.hpp"
#include "controller/RoomsController.hpp"
#include "controller/BleController.hpp"
#include "controller/SettingsController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"
#include "CorsInterceptor.hpp"

#include "service/ServiceManager.hpp"

#include "config/Constants.hpp"
#include "service/ServiceRegistry.hpp"
#include "service/ble/BleService.hpp"
#include "service/MqttService.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

std::atomic<bool> serverRunning{true};

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

     /* Create BleController and add all of its endpoints to router */
    router->addController(std::make_shared<BleController>());

    /* Create SettingsController and add all of its endpoints to router */
    router->addController(std::make_shared<SettingsController>());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, wsHandler, "websocket");
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, graphWsHandler, "graphwebsocket");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    LOGI("Server running on port {}", connectionProvider->getProperty("port").toString());

    /* Run server in a separate thread */
    std::thread serverThread([&server]() {
        server.run();
    });

    /* Wait for user input to stop server */
    LOGI("Press 'q' and Enter to stop the server gracefully...");
    std::string input;
    while (serverRunning && std::getline(std::cin, input)) {
        if (input == "q" || input == "Q") {
            LOGI("Shutdown requested. Stopping server...");
            serverRunning = false;
            server.stop();
            break;
        }
    }

    /* Wait for server thread to finish */
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    LOGI("Server stopped successfully");
    
    /* Stop all connection handlers to cleanup WebSockets */
    try {
        LOGI("Stopping HTTP connection handler...");
        connectionHandler->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        LOGI("Stopping WebSocket connection handler...");
        wsHandler->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        LOGI("Stopping Graph WebSocket connection handler...");
        graphWsHandler->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        /* Clear socket instance listeners to release GraphWebSocket and Lobby BEFORE AppComponent is destroyed */
        LOGI("Clearing socket instance listeners...");
        auto* wsHandlerAsync = dynamic_cast<oatpp::websocket::AsyncConnectionHandler*>(wsHandler.get());
        auto* graphWsHandlerAsync = dynamic_cast<oatpp::websocket::AsyncConnectionHandler*>(graphWsHandler.get());
        
        if (wsHandlerAsync) {
            wsHandlerAsync->setSocketInstanceListener(nullptr);
        }
        if (graphWsHandlerAsync) {
            graphWsHandlerAsync->setSocketInstanceListener(nullptr);
        }
        
        LOGI("Socket instance listeners cleared");
        
    } catch (const std::exception& e) {
        LOGI("Exception during connection handler stop: {}", e.what());
    } catch (...) {
        LOGI("Unknown exception during connection handler stop");
    }
    
    LOGI("All connection handlers stopped");
    
    /* Stop the async executor before destroying components */
    try {
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        LOGI("Stopping async executor...");
        executor->stop();
        executor->join();
        LOGI("Async executor stopped");
    } catch (const std::exception& e) {
        LOGI("Exception during executor stop: {}", e.what());
    } catch (...) {
        LOGI("Unknown exception during executor stop");
    }
    
    /* Allow more time for all async tasks to complete */
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    LOGI("Exiting run() - AppComponent will be destroyed");
    
    // Let components go out of scope naturally
    // AppComponent destructor will handle cleanup
}

int main(int argc, const char * argv[]) {
    
    int exitCode = 0;
    
    try {
        oatpp::Environment::init();

        LOGI("Backend Main program started");
        ServiceRegistry::instance().registerService("ble", std::make_shared<BleService>());
        ServiceRegistry::instance().registerService("mqtt", std::make_shared<MqttService>());

        auto bleService = USE_SRVC("ble");
        bleService->start();
        //TODO: fix this
        if (bleService && bleService->isRunning()) {
            //std::cout << "BLE is running!\n";
            //bleService->sendCommand("reset");
        }
        else
        {
            LOGI("Ble not running");
        }

        run();
        
        LOGI("Run completed, cleaning up...");

        /* Gracefully stop all services */
        LOGI("Shutting down services...");
        
        if (bleService && bleService->isRunning()) {
            LOGI("Stopping BLE service...");
            bleService->stop();
        }
        
        auto mqttService = USE_SRVC("mqtt");
        if (mqttService && mqttService->isRunning()) {
            LOGI("Stopping MQTT service...");
            mqttService->stop();
        }
        
        /* Clear service registry before destroying oatpp environment */
        ServiceRegistry::instance().clearServices();
        bleService.reset();  // Explicitly release service reference
        
        LOGI("All services stopped. Cleaning up...");
        
        /* Longer delay to ensure all async operations and websockets complete */
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        LOGI("Destroying oatpp environment...");
        oatpp::Environment::destroy();
        LOGI("Backend Main program exited gracefully");
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        exitCode = 1;
    } catch (...) {
        std::cerr << "Unknown exception in main" << std::endl;
        exitCode = 1;
    }

    return exitCode;
}
