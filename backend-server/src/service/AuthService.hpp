#ifndef AUTHSERVICE_HPP
#define AUTHSERVICE_HPP

#include "oatpp/Types.hpp"
#include <unordered_map>
#include <string>

class AuthService {
private:
    std::unordered_map<std::string, std::string> m_users;

public:
    AuthService();

    bool authenticate(const oatpp::String& username, const oatpp::String& password) const;
};

#endif // AUTHSERVICE_HPP
