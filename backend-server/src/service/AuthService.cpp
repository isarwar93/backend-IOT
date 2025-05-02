#include "service/AuthService.hpp"
#include "./AppComponent.hpp"

AuthService::AuthService() {
  m_users["admin"] = "admin";
  m_users["guest"] = "guest";
}

bool AuthService::authenticate(const oatpp::String& username, const oatpp::String& password) const {
    OATPP_LOGi("AuthService", "Checking username='{}' password={}", username->c_str(), password->c_str());

    auto it = m_users.find(*username);
    if(it != m_users.end() && it->second == *password) { 
        OATPP_LOGi("AuthService", "Authentication successful for {}", username->c_str());
        return true;
    }
    OATPP_LOGi("AuthService", "Authentication failed for {}", username->c_str());
    return false;
}