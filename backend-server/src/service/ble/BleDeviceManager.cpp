// File: src/service/ble/BleDeviceManager.cpp
#include "BleDeviceManager.hpp"
#include "config/LogAdapt.hpp"
#include <nlohmann/json.hpp>
#include <regex>
#include <chrono>
#include <thread>

BleDeviceManager::BleDeviceManager(GDBusConnection* conn)
    : gDBusConn_(conn) {}

void BleDeviceManager::setConnection(GDBusConnection* conn) {
    gDBusConn_ = conn;
}

void BleDeviceManager::setAdapterPath(const std::string& path) {
    adapterPath_ = path;
}

std::string BleDeviceManager::parseMacFlexible(const std::string& input) {
    try {
        nlohmann::json j = nlohmann::json::parse(input);
        if (j.is_string()) {
            return j.get<std::string>();
        } else if (j.is_object() && j.contains("mac") && j["mac"].is_string()) {
            return j["mac"].get<std::string>();
        }
    } catch (...) {
        // Fallback to manual processing
    }

    std::regex quoteRegex(R"(^["'](.*)["']$)");
    std::smatch match;
    if (std::regex_match(input, match, quoteRegex)) {
        return match[1];
    }
    return input;
}

std::string BleDeviceManager::macToDevicePath(const std::string& mac) {
    std::string path = "/org/bluez/hci0/dev_" + mac;
    for (char& c : path) if (c == ':') c = '_';
    return path;
}

std::vector<BleDeviceInfo> BleDeviceManager::scanDevices(int timeoutSeconds) {
    std::vector<BleDeviceInfo> devices;
    GError* error = nullptr;

    if (!gDBusConn_) {
        gDBusConn_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
        if (!gDBusConn_) {
            LOGE("Failed to connect to system bus: {}", error ? error->message : "unknown");
            if (error) g_error_free(error);
            return devices;
        }
    }

    std::string adapterPath = "/org/bluez/hci0";
    if (!adapterPath_.empty()) {
        adapterPath = adapterPath_;
    }

    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", adapterPath.c_str(), "org.bluez.Adapter1",
        "StartDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        LOGE("StartDiscovery failed: {}", error->message);
        g_error_free(error);
        return devices;
    }

    std::this_thread::sleep_for(std::chrono::seconds(timeoutSeconds));

    GVariant* reply = g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (!reply) {
        LOGE("GetManagedObjects failed: {}", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return devices;
    }

    GVariantIter* iter;
    g_variant_get(reply, "(a{oa{sa{sv}}})", &iter);

    const gchar* objectPath;
    GVariant* ifaceDict;

    while (g_variant_iter_loop(iter, "{o@a{sa{sv}}}", &objectPath, &ifaceDict)) {
        std::string path(objectPath);
        if (path.find("/dev_") == std::string::npos) continue;

        std::string mac = path.substr(path.find("/dev_") + 5);
        for (auto& ch : mac) if (ch == '_') ch = ':';

        std::string name = "Unknown";
        int rssi = -999;

        GVariantIter* ifaceIter;
        g_variant_get(ifaceDict, "a{sa{sv}}", &ifaceIter);

        const gchar* ifaceName;
        GVariant* propDict;

        while (g_variant_iter_loop(ifaceIter, "{s@a{sv}}", &ifaceName, &propDict)) {
            if (std::string(ifaceName) == "org.bluez.Device1") {
                GVariantIter* propIter;
                g_variant_get(propDict, "a{sv}", &propIter);
                const gchar* key;
                GVariant* val;

                while (g_variant_iter_loop(propIter, "{sv}", &key, &val)) {
                    std::string k(key);
                    if (k == "Name") name = g_variant_get_string(val, nullptr);
                    else if (k == "RSSI") rssi = g_variant_get_int16(val);
                }
                g_variant_iter_free(propIter);
            }
        }
        g_variant_iter_free(ifaceIter);
        devices.push_back({mac, name, rssi});
    }
    g_variant_iter_free(iter);
    g_variant_unref(reply);

    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", adapterPath.c_str(), "org.bluez.Adapter1",
        "StopDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, nullptr);

    return devices;
}

bool BleDeviceManager::connectToDevice(const std::string& jsonmac) {
    if (!gDBusConn_) return false;

    std::string mac = parseMacFlexible(jsonmac);
    std::string path = macToDevicePath(mac);

    GError* error = nullptr;
    LOGI("Connecting to device at path: {}", path);
    
    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", 
        path.c_str(), 
        "org.bluez.Device1",
        "Connect", 
        nullptr, 
        nullptr, 
        G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        LOGE("Connect failed: {}", error->message);
        g_error_free(error);
        return false;
    }
    
    LOGI("Successfully connected to device");
    return true;
}

bool BleDeviceManager::disconnectDevice(const std::string& jsonmac) {
    if (!gDBusConn_) return false;

    LOGI("Disconnecting device with MAC: {}", jsonmac);
    std::string mac = parseMacFlexible(jsonmac);
    
    if (!isConnected(mac)) {
        LOGW("Device with MAC: {} is not connected", mac);
        return true;
    }

    std::string path = macToDevicePath(mac);
    GError* error = nullptr;

    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez",
        path.c_str(),
        "org.bluez.Device1",
        "Disconnect",
        nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1, nullptr, &error
    );

    if (error) {
        LOGE("Disconnect failed: {}", error->message);
        g_error_free(error);
        return false;
    }

    LOGI("Successfully disconnected from device");
    return true;
}

bool BleDeviceManager::isPaired(const std::string& mac) {
    if (!gDBusConn_) return false;

    std::string path = macToDevicePath(mac);
    GError* error = nullptr;
    
    GVariant* result = g_dbus_connection_call_sync(gDBusConn_, "org.bluez", path.c_str(),
        "org.freedesktop.DBus.Properties", "Get",
        g_variant_new("(ss)", "org.bluez.Device1", "Paired"),
        G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (!result) {
        LOGE("Check Paired failed: {}", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return false;
    }
    
    GVariant* val;
    g_variant_get(result, "(v)", &val);
    bool paired = g_variant_get_boolean(val);
    g_variant_unref(val);
    g_variant_unref(result);
    
    return paired;
}

bool BleDeviceManager::pairDevice(const std::string& mac) {
    if (!gDBusConn_) return false;

    std::string path = macToDevicePath(mac);
    GError* error = nullptr;
    
    g_dbus_connection_call_sync(gDBusConn_, "org.bluez", path.c_str(), "org.bluez.Device1",
        "Pair", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    
    if (error) {
        LOGE("Pair failed: {}", error->message);
        g_error_free(error);
        return false;
    }
    
    LOGI("Successfully paired with device: {}", mac);
    return true;
}

bool BleDeviceManager::trustDevice(const std::string& mac) {
    if (!gDBusConn_) return false;

    std::string path = macToDevicePath(mac);
    GError* error = nullptr;
    
    g_dbus_connection_call_sync(gDBusConn_, "org.bluez", path.c_str(), "org.freedesktop.DBus.Properties",
        "Set", g_variant_new("(ssv)", "org.bluez.Device1", "Trusted",
        g_variant_new_boolean(true)), nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (error) {
        LOGE("TrustDevice failed: {}", error->message);
        g_error_free(error);
        return false;
    }
    
    LOGI("Successfully trusted device: {}", mac);
    return true;
}

std::vector<std::string> BleDeviceManager::getConnectedDevices() {
    std::vector<std::string> devices;
    if (!gDBusConn_) return devices;

    GError* error = nullptr;
    GVariant* reply = g_dbus_connection_call_sync(gDBusConn_, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (!reply) {
        LOGE("Failed to get managed objects: {}", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return devices;
    }

    GVariantIter* iter;
    g_variant_get(reply, "(a{oa{sa{sv}}})", &iter);
    gchar* object_path;
    GVariant* interfaces;
    
    while (g_variant_iter_loop(iter, "{oa{sa{sv}}}", &object_path, &interfaces)) {
        if (g_strstr_len(object_path, -1, "/dev_")) {
            GVariantIter* iface_iter;
            gchar* iface;
            GVariant* props;
            g_variant_get(interfaces, "a{sa{sv}}", &iface_iter);
            
            while (g_variant_iter_loop(iface_iter, "{sa{sv}}", &iface, &props)) {
                if (std::string(iface) == "org.bluez.Device1") {
                    GVariantIter* prop_iter;
                    gchar* key;
                    GVariant* val;
                    g_variant_get(props, "a{sv}", &prop_iter);
                    
                    while (g_variant_iter_loop(prop_iter, "{sv}", &key, &val)) {
                        if (std::string(key) == "Connected" && g_variant_is_of_type(val, G_VARIANT_TYPE_BOOLEAN)) {
                            if (g_variant_get_boolean(val)) {
                                devices.emplace_back(object_path);
                            }
                        }
                    }
                    g_variant_iter_free(prop_iter);
                }
            }
            g_variant_iter_free(iface_iter);
        }
    }
    g_variant_iter_free(iter);
    g_variant_unref(reply);
    
    return devices;
}

bool BleDeviceManager::isConnected(const std::string& mac) {
    if (!gDBusConn_) return false;

    std::string path = macToDevicePath(mac);
    GError* error = nullptr;
    
    GVariant* result = g_dbus_connection_call_sync(gDBusConn_, "org.bluez", path.c_str(),
        "org.freedesktop.DBus.Properties", "Get",
        g_variant_new("(ss)", "org.bluez.Device1", "Connected"),
        G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    
    if (!result) {
        LOGW("Device might not be available: {}", mac);
        if (error) g_error_free(error);
        return false;
    }
    
    GVariant* val;
    g_variant_get(result, "(v)", &val);
    bool connected = g_variant_get_boolean(val);
    g_variant_unref(val);
    g_variant_unref(result);
    
    return connected;
}

bool BleDeviceManager::removeDevice(const std::string& mac) {
    if (!gDBusConn_) return false;

    std::string path = macToDevicePath(mac);
    std::string adapterPath = "/org/bluez/hci0";
    if (!adapterPath_.empty()) {
        adapterPath = adapterPath_;
    }

    GError* error = nullptr;
    g_dbus_connection_call_sync(gDBusConn_, "org.bluez", 
                                adapterPath.c_str(), 
                                "org.bluez.Adapter1",
                                "RemoveDevice",
                                g_variant_new("(o)", path.c_str()),
                                nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    
    if (error) {
        LOGE("RemoveDevice failed: {}", error->message);
        g_error_free(error);
        return false;
    }
    
    LOGI("Successfully removed device: {}", mac);
    return true;
}

bool BleDeviceManager::cleanupDisconnectedDevices() {
    for (const auto& path : getConnectedDevices()) {
        LOGI("Device connected: {}", path);
    }
    return true;
}

void BleDeviceManager::printScanResults(const std::vector<BleDeviceInfo>& devices) {
    LOGI("Discovered BLE Devices:");
    for (const auto& dev : devices) {
        LOGI("  MAC: {}, Name: {}, RSSI: {} dBm", dev.mac, dev.name, dev.rssi);
    }
}
