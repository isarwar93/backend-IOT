// File: src/service/ble/BleCharacteristicManager.cpp
#include "BleCharacteristicManager.hpp"
#include "config/LogAdapt.hpp"
#include <cstring>
#include <algorithm>
#include <map>

// UUID to readable name mapping
const std::unordered_map<std::string, std::string> UUID_TO_NAME = {
    // Services
    {"0000180d-0000-1000-8000-00805f9b34fb", "Heart Rate Service"},
    {"0000180a-0000-1000-8000-00805f9b34fb", "Device Information"},
    {"0000180f-0000-1000-8000-00805f9b34fb", "Battery Service"},
    
    // Characteristics
    {"00002a37-0000-1000-8000-00805f9b34fb", "Heart Rate Measurement"},
    {"00002a38-0000-1000-8000-00805f9b34fb", "Body Sensor Location"},
    {"00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String"},
    {"00002a25-0000-1000-8000-00805f9b34fb", "Serial Number String"},
    {"00002a19-0000-1000-8000-00805f9b34fb", "Battery Level"},
};

BleCharacteristicManager::BleCharacteristicManager(GDBusConnection* conn)
    : gDBusConn_(conn), numOfActiveNotifications_(0) {}

void BleCharacteristicManager::setConnection(GDBusConnection* conn) {
    gDBusConn_ = conn;
}

void BleCharacteristicManager::setAdapterPath(const std::string& path) {
    adapterPath_ = path;
}

std::string BleCharacteristicManager::uuidToReadableName(const std::string& uuid) const {
    auto it = UUID_TO_NAME.find(uuid);
    return (it != UUID_TO_NAME.end()) ? it->second : "Unknown";
}

std::vector<BleServiceInfo> BleCharacteristicManager::getServicesAndCharacteristics(const std::string& mac) {
    std::vector<BleServiceInfo> services;
    std::string basePath = "/org/bluez/hci0/dev_" + mac;
    for (auto& ch : basePath) if (ch == ':') ch = '_';

    GError* error = nullptr;
    
    if (!gDBusConn_) {
        gDBusConn_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
        if (!gDBusConn_) {
            LOGE("Failed to connect to system bus: {}", error ? error->message : "unknown");
            if (error) g_error_free(error);
            return services;
        }
    }

    GVariant* reply = g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (!reply) {
        LOGE("GetManagedObjects failed: {}", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return services;
    }

    GVariantIter* iter;
    g_variant_get(reply, "(a{oa{sa{sv}}})", &iter);
    const gchar* objectPath;
    GVariant* ifaceDict;
    std::map<std::string, BleServiceInfo> serviceMap;

    while (g_variant_iter_loop(iter, "{o@a{sa{sv}}}", &objectPath, &ifaceDict)) {
        std::string path(objectPath);
        if (path.find(basePath) != 0) continue;

        GVariantIter* ifaceIter;
        g_variant_get(ifaceDict, "a{sa{sv}}", &ifaceIter);
        const gchar* ifaceName;
        GVariant* propDict;

        while (g_variant_iter_loop(ifaceIter, "{s@a{sv}}", &ifaceName, &propDict)) {
            std::string iface(ifaceName);
            
            // Process GATT Service
            if (iface == "org.bluez.GattService1") {
                std::string uuid;
                GVariantIter* props;
                g_variant_get(propDict, "a{sv}", &props);
                const gchar* key;
                GVariant* val;

                while (g_variant_iter_loop(props, "{sv}", &key, &val)) {
                    if (std::string(key) == "UUID") {
                        uuid = g_variant_get_string(val, nullptr);
                    }
                }
                g_variant_iter_free(props);
                
                serviceMap[path] = BleServiceInfo{
                    .name = uuidToReadableName(uuid),
                    .path = path,
                    .uuid = uuid,
                    .characteristics = {}
                };
                LOGI("Found service: {} ({})", serviceMap[path].name, uuid);
            }
            // Process GATT Characteristic
            else if (iface == "org.bluez.GattCharacteristic1") {
                std::string uuid, parent;
                bool notifying = false;
                BleCharacteristicInfo cInfo;

                GVariantIter* props;
                g_variant_get(propDict, "a{sv}", &props);
                const gchar* key;
                GVariant* val;

                while (g_variant_iter_loop(props, "{sv}", &key, &val)) {
                    std::string keyStr(key);
                    
                    if (keyStr == "UUID") {
                        uuid = g_variant_get_string(val, nullptr);
                    } 
                    else if (keyStr == "Service") {
                        parent = g_variant_get_string(val, nullptr);
                    } 
                    else if (keyStr == "Notifying") {
                        notifying = g_variant_get_boolean(val);
                    } 
                    else if (keyStr == "Flags") {
                        if (g_variant_is_of_type(val, G_VARIANT_TYPE("as"))) {
                            GVariantIter* flagIter;
                            gchar* flagStr;
                            g_variant_get(val, "as", &flagIter);
                            while (g_variant_iter_loop(flagIter, "s", &flagStr)) {
                                cInfo.properties.push_back(std::string(flagStr));
                            }
                            g_variant_iter_free(flagIter);
                        }
                    }
                }
                g_variant_iter_free(props);

                if (!uuid.empty() && !parent.empty()) {
                    cInfo.name = uuidToReadableName(uuid);
                    cInfo.uuid = uuid;
                    cInfo.path = objectPath;
                    cInfo.notifying = notifying;
                    LOGI("Found characteristic: {} at {}", cInfo.name, cInfo.path);
                    serviceMap[parent].characteristics.push_back(cInfo);
                }
            }
        }
        g_variant_iter_free(ifaceIter);
    }

    g_variant_iter_free(iter);
    g_variant_unref(reply);

    for (auto& [_, svc] : serviceMap) {
        services.push_back(std::move(svc));
    }

    return services;
}

bool BleCharacteristicManager::enableNotification(const std::string& charPath) {
    if (charPath.empty()) {
        LOGE("Characteristic path is empty");
        return false;
    }

    if (!gDBusConn_) {
        LOGE("D-Bus connection not established");
        return false;
    }

    GError* error = nullptr;
    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", charPath.c_str(), "org.bluez.GattCharacteristic1",
        "StartNotify", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        LOGE("StartNotify failed: {}", error->message);
        g_error_free(error);
        return false;
    }

    numOfActiveNotifications_++;
    LOGI("Enabled notification for characteristic: {}, active: {}", charPath, numOfActiveNotifications_);
    return true;
}

bool BleCharacteristicManager::disableNotification(const std::string& charPath) {
    if (charPath.empty()) {
        LOGE("Characteristic path is empty");
        return false;
    }

    if (!gDBusConn_) {
        LOGE("D-Bus connection not established");
        return false;
    }

    GError* error = nullptr;
    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", charPath.c_str(), "org.bluez.GattCharacteristic1",
        "StopNotify", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        LOGE("StopNotify failed: {}", error->message);
        g_error_free(error);
        return false;
    }

    numOfActiveNotifications_--;
    if (numOfActiveNotifications_ < 0) {
        numOfActiveNotifications_ = 0;
    }
    LOGI("Disabled notification for characteristic: {}, active: {}", charPath, numOfActiveNotifications_);
    return true;
}

size_t BleCharacteristicManager::readCharacteristic(const std::string& charPath, uint8_t* retData) {
    if (!gDBusConn_) {
        LOGE("D-Bus connection not established");
        return 0;
    }

    if (!retData) {
        LOGE("Invalid output buffer");
        return 0;
    }

    GError* error = nullptr;
    GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    GVariant* v1 = g_variant_builder_end(builder);

    GVariant* ret = g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", charPath.c_str(), "org.bluez.GattCharacteristic1",
        "ReadValue", g_variant_new_tuple(&v1, 1), G_VARIANT_TYPE("(ay)"),
        G_DBUS_CALL_FLAGS_NONE, 5000, nullptr, &error);

    if (error) {
        LOGE("ReadValue failed: {}", error->message);
        g_error_free(error);
        return 0;
    }

    if (!ret) {
        LOGE("ReadValue returned null");
        return 0;
    }

    GVariant* arr = nullptr;
    g_variant_get(ret, "(@ay)", &arr);

    gsize len = 0;
    const uint8_t* data = static_cast<const uint8_t*>(
        g_variant_get_fixed_array(arr, &len, sizeof(uint8_t))
    );

    if (len > 0) {
        memcpy(retData, data, len);
        LOGI("Read {} bytes from characteristic", len);
    }

    g_variant_unref(arr);
    g_variant_unref(ret);

    return len;
}

bool BleCharacteristicManager::writeCharacteristic(const std::string& charPath, const std::vector<uint8_t>& data) {
    if (!gDBusConn_) {
        LOGE("D-Bus connection not established");
        return false;
    }

    if (data.empty()) {
        LOGE("No data to write");
        return false;
    }

    GError* error = nullptr;
    GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    GVariant* options = g_variant_builder_end(builder);

    // Create byte array from vector
    GVariantBuilder* arrayBuilder = g_variant_builder_new(G_VARIANT_TYPE("ay"));
    for (uint8_t byte : data) {
        g_variant_builder_add(arrayBuilder, "y", byte);
    }
    GVariant* byteArray = g_variant_builder_end(arrayBuilder);

    GVariant* ret = g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", charPath.c_str(), "org.bluez.GattCharacteristic1",
        "WriteValue", g_variant_new("(@ay@a{sv})", byteArray, options),
        nullptr, G_DBUS_CALL_FLAGS_NONE, 5000, nullptr, &error);

    if (error) {
        LOGE("WriteValue failed: {}", error->message);
        g_error_free(error);
        return false;
    }

    if (ret) {
        g_variant_unref(ret);
    }

    LOGI("Wrote {} bytes to characteristic", data.size());
    return true;
}

std::string BleCharacteristicManager::readService(const std::string& mac, const std::string& uuid) {
    LOGI("Read service: {}-{}", mac, uuid);
    // TODO: Implement actual read operation if needed
    return std::string("/org/bluez/hci0/dev_") + mac + "/" + uuid;
}

bool BleCharacteristicManager::writeService(const std::string& body) {
    LOGI("Write service: {}", body);
    // TODO: Implement actual write operation if needed
    return true;
}

bool BleCharacteristicManager::enableServices(const std::string& body) {
    LOGI("Enable services: {}", body);
    // TODO: Implement actual enable operation if needed
    return true;
}

std::string BleCharacteristicManager::extractUuidPrefix(const std::string& uuid) {
    // Take substring before first dash
    size_t dashPos = uuid.find('-');
    if (dashPos == std::string::npos) {
        return uuid;
    }
    return uuid.substr(0, dashPos);
}
