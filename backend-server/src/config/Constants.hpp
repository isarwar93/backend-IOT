#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "service/ServiceRegistry.hpp"

#define USE_SRVC(name) ServiceRegistry::instance().get(name)


#endif