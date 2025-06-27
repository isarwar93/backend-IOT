#ifndef SERVICEMANAGER_HPP
#define SERVICEMANAGER_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include "IService.hpp"

class ServiceManager {
public:
    template<typename T>
    void registerService(const std::string& name) {
        services[name] = std::make_shared<T>();
    }

    std::shared_ptr<IService> getService(const std::string& name);
    bool startService(const std::string& name);
    void stopService(const std::string& name);
    bool isRunning(const std::string& name);
    bool configureService(const std::string& name, const std::string& config);
    std::string sendCommandToService(const std::string& name, const std::string& command);
    std::string getServiceStatusJson(const std::string& name);

private:
    std::unordered_map<std::string, std::shared_ptr<IService>> services;
};

#endif // SERVICEMANAGER_HPP