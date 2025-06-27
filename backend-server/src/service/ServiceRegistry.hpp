#ifndef SERVICE_REGISTRY_HPP
#define SERVICE_REGISTRY_HPP

#include <map>
#include <string>
#include <memory>
#include "IService.hpp"

class ServiceRegistry {
public:
    static ServiceRegistry& instance() {
        static ServiceRegistry registry;
        return registry;
    }

    void registerService(const std::string& name, std::shared_ptr<IService> service) {
        m_services[name] = std::move(service);
    }

    std::shared_ptr<IService> get(const std::string& name) const {
        auto it = m_services.find(name);
        if (it != m_services.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    ServiceRegistry() = default;
    std::map<std::string, std::shared_ptr<IService>> m_services;
};

#endif // SERVICE_REGISTRY_HPP