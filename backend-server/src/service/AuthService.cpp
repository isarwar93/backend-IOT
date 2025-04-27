#include "service/AuthService.hpp"

AuthService::AuthService() {
  // Hardcoded users
  m_users["admin"] = "admin";
  m_users["guest"] = "guest";
}

bool AuthService::authenticate(const oatpp::String& username, const oatpp::String& password) const {
    auto it = m_users.find(username->std_str());
    if(it != m_users.end() && it->second == password->std_str()) {
        return true;
    }
    return false;
}
