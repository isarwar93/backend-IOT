#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "model/Lobby.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/macro/component.hpp"
#include "websocket/GraphWebSocket.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent {
public:

    /**
    * Create Async Executor
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([] {
        return std::make_shared<oatpp::async::Executor>(
          4 /* Data-Processing threads */,
          2 /* I/O threads */, //TODO: Adjust number of I/O threads based on your server's capabilities
          1 /* Timer threads */
        );
    }());

    /**
    *  Create ConnectionProvider component which listens on the port
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
        return oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", 8000, oatpp::network::Address::IP_4});
    }());

    /**
    *  Create Router component
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
        return oatpp::web::server::HttpRouter::createShared();
    }());

    /**
    *  Create ConnectionHandler component which uses Router component to route requests
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)("http", [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor); // get Async executor component
        return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, executor);
    }());

    /**
    *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
        return std::make_shared<oatpp::json::ObjectMapper>();
    }());

    /**
    *  Create websocket connection handler
    */

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)("websocket", [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
        connectionHandler->setSocketInstanceListener(std::make_shared<Lobby>());
        return connectionHandler;
    }());


    /**
    *  Creating graph websocket connection handler
    */

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, graphWebSocketConnHandler)("graphwebsocket", [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
        connectionHandler->setSocketInstanceListener(std::make_shared<GraphWebSocket>());
        return connectionHandler;
    }());
};

#endif /* AppComponent_hpp */