#ifndef ISERVICE_HPP
#define ISERVICE_HPP

#include <string>

class IService {
public:
    virtual ~IService() = default;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    virtual bool configure(const std::string& jsonConfig) { return false; }
    virtual std::string getStatusJson() const { return "{}"; }
    virtual std::string sendCommand(const std::string& command) { return "Unsupported"; }
};
#endif // ISERVICE_HPP