#ifndef MQTTSERVICE_HPP
#define MQTTSERVICE_HPP

#include "IService.hpp"
#include <iostream>

class MqttService : public IService {
public:
    bool start() override {
        std::cout << "MQTT Connected\n";
        connected = true;
        return true;
    }

    void stop() override {
        std::cout << "MQTT Disconnected\n";
        connected = false;
    }

    bool isRunning() const override {
        return connected;
    }

    bool configure(const std::string& config) override {
        std::cout << "Config received: " << config << "\n";
        return true;
    }

    std::string sendCommand(const std::string& command) override {
        return "MQTT command: " + command;
    }

    std::string getStatusJson() const override {
        return "{\"connected\": " + std::string(connected ? "true" : "false") + "}";
    }

private:
    bool connected = false;
};
#endif // MQTTSERVICE_HPP