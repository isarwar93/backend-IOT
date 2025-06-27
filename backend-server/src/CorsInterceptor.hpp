#pragma once

#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"

class CorsInterceptor : public oatpp::web::server::interceptor::RequestInterceptor {
public:

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> intercept(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request
    ) override {
        // Handle CORS preflight request
        if(request->getStartingLine().method == "OPTIONS") {
            auto response = oatpp::web::protocol::http::outgoing::Response::createShared(
                oatpp::web::protocol::http::Status::CODE_204, nullptr
            );

            response->putHeader("Access-Control-Allow-Origin", "http://localhost:5173"); // or "*" if you're okay with any origin
            response->putHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            response->putHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            response->putHeader("Access-Control-Allow-Credentials", "true");

            return response;
        }

        return nullptr; // continue to next interceptor or controller
    }
};