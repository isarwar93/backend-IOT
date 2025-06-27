#include "ServiceManager.hpp"
#include <stdexcept>
std::shared_ptr<IService> ServiceManager::getService(const std::string& name) {
    if (services.count(name) == 0)
        throw std::runtime_error("Service not found: " + name);
    return services.at(name);
}

bool ServiceManager::startService(const std::string& name) {
    return getService(name)->start();
}

void ServiceManager::stopService(const std::string& name) {
    getService(name)->stop();
}

bool ServiceManager::isRunning(const std::string& name) {
    return getService(name)->isRunning();
}

bool ServiceManager::configureService(const std::string& name, const std::string& config) {
    return getService(name)->configure(config);
}

std::string ServiceManager::sendCommandToService(const std::string& name, const std::string& command) {
    return getService(name)->sendCommand(command);
}

std::string ServiceManager::getServiceStatusJson(const std::string& name) {
    return getService(name)->getStatusJson();
}
