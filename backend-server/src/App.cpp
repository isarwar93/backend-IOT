#include "controller/UserController.hpp"
#include "controller/GraphController.hpp"
#include "controller/RoomsController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"
#include "CorsInterceptor.hpp"

#include "ble/ble.hpp"

#include <iostream>

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


//   std::string targetMac = "F0:F5:BD:2C:1E:66";
//   BleClient client("blehr_sensor_1.0");

//   // client.startScan([](const std::string& address, const std::string& name) {
//   //     std::cout << "Discovered: " << name << " [" << address << "]" << std::endl;
//   // });

//   std::this_thread::sleep_for(std::chrono::seconds(1));

//   std::cout << "Connecting to " << targetMac << "...\n";
//   if (client.connectToDevice(targetMac)) {
//       std::cout << "Connected successfully.\n";

//       if (client.enableHeartRateNotifications(targetMac)) {
//           std::cout << "Waiting for notifications...\n";
//           // Run forever to keep receiving
         
//       }
//   } else {
//       std::cout << "❌ Failed to connect.\n";
//   }

//   return 0;

    oatpp::Environment::init();

    run();

    oatpp::Environment::destroy();

    return 0;
}
