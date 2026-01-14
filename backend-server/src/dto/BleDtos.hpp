#ifndef BLEDTOS_HPP
#define BLEDTOS_HPP

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * DTO for POST /api/ble/connect
 */
class ConnectRequestDto : public oatpp::DTO {
    DTO_INIT(ConnectRequestDto, DTO)

    DTO_FIELD(oatpp::String, mac);
};

/**
 * DTO for POST /api/ble/select-services
 */
class SelectServicesRequestDto : public oatpp::DTO {
    DTO_INIT(SelectServicesRequestDto, DTO)
    DTO_FIELD(oatpp::String, mac);
    DTO_FIELD(oatpp::List<oatpp::String>, uuids);
};

/**
 * DTO representing a single GATT characteristic.
 */
class CharacteristicDto : public oatpp::DTO {
    DTO_INIT(CharacteristicDto, DTO)
    DTO_FIELD(oatpp::String, name);
    DTO_FIELD(oatpp::String, path);
    DTO_FIELD(oatpp::String, uuid);
    DTO_FIELD(oatpp::Boolean, notifying);
    DTO_FIELD(oatpp::Vector<oatpp::String>, properties); 
};

/*
 * DTO representing a BLE service and its characteristics.
 */
class ServiceDto : public oatpp::DTO {
    DTO_INIT(ServiceDto, DTO)
    DTO_FIELD(oatpp::String, name);
    DTO_FIELD(oatpp::String, path);
    DTO_FIELD(oatpp::String, uuid);
    DTO_FIELD(oatpp::List<oatpp::Object<CharacteristicDto>>, characteristics);
};

/**
 * DTO representing a discovered BLE device.
 */
class DeviceDto : public oatpp::DTO {
    DTO_INIT(DeviceDto, DTO)
    DTO_FIELD(oatpp::String, mac);
    DTO_FIELD(oatpp::String, name);
    DTO_FIELD(oatpp::Int32, rssi);
};

class StatusDto : public oatpp::DTO {
    DTO_INIT(StatusDto, DTO)
    DTO_FIELD(oatpp::Boolean, isConnected);
    DTO_FIELD(oatpp::Boolean, isPaired);
};

class StringValueDto : public oatpp::DTO {
    DTO_INIT(StringValueDto, DTO)
    DTO_FIELD(oatpp::String, stringValue);
};

#include OATPP_CODEGEN_END(DTO)

#endif // BLEDTOS_HPP
