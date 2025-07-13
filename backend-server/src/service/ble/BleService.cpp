#include "BleService.hpp"
#include "config/LogAdapt.hpp"
#include <nlohmann/json.hpp>
#include <gio/gio.h>
#include <regex>


bool BleService::start() {
    if (running) return true;
    LOGI("Starting BLE Service.");
    running = true;
    // TODO: connect to device, subscribe to notifications
    if (!initBluez()) {
        LOGE("Failed to initialize BlueZ.");
        return false;
    }

    // auto results = scanDevices(1);
    // printScanResults(results);
    // // TODO: later comes from configure json
    // macAddress = "A0:DD:6C:AF:73:9E";
    // startScanUntilFound(macAddress, 10);
    // if (!connectToDevice(macAddress)){
    //     std::cerr << "Failed to Connect" << std::endl;
    //     return false;
    // } 
    // else{
    //     std::cout<<"connected"<<std::endl;
    // }
    //enableHeartRateNotifications(macAddress);
    
   
    return true;
}

void BleService::stop() {
    std::cout << "BLE Service stopped\n";
    running = false;
    // TODO: disconnect from device
    if (m_loop) {
        g_main_loop_quit(m_loop);
    }
    if (m_loopThread.joinable()) {
        m_loopThread.join();
    }
    g_main_loop_unref(m_loop);
    m_loop = nullptr;
    running = false;
}


bool BleService::isRunning() const {
    return running;
}

bool BleService::configure(const std::string& jsonConfig) {
    try {
        auto json = nlohmann::json::parse(jsonConfig);
        if (json.contains("mac")) {
            macAddress = json["mac"];
            std::cout << "Configured MAC: " << macAddress << "\n";
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

std::string BleService::sendCommand(const std::string& command) {
    lastCommandResponse = "Executed command: " + command;
    return lastCommandResponse;
}

std::string BleService::getStatusJson() const {
    return nlohmann::json({
        {"running", running},
        {"mac", macAddress},
        {"lastCommandResponse", lastCommandResponse},
        {"latestValue", latestValue.load()}
    }).dump();
}

float BleService::getLatestMeasurement() const {
    // std::cout << "getting/loading heart rate value: " <<latestValue.load()<< "\n";
    return latestValue.load();
}

void BleService::onHeartRateReceived(float hr) {
    latestValue.store(hr);
}

bool BleService::initBluez() {
    adapterPath = findAdapter();
    return !adapterPath.empty();
}

bool BleService::startScanUntilFound(const std::string& targetMac, int timeoutSeconds) {
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!conn) {
        std::cerr << "❌ Failed to connect to system D-Bus.\n";
        return false;
    }

    GError* error = nullptr;

    // Start BLE scanning
    g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        adapterPath.c_str(),
        "org.bluez.Adapter1",
        "StartDiscovery",
        nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error
    );

    if (error) {
        std::cerr << "❌ StartDiscovery failed: " << error->message << std::endl;
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }

    std::cout << "🔍 Scanning for " << targetMac << " for up to " << timeoutSeconds << " seconds...\n";

    // Normalize MAC for D-Bus path
    std::string normalizedMac = targetMac;
    for (auto& ch : normalizedMac) if (ch == ':') ch = '_';
    std::string targetPath = adapterPath + "/dev_" + normalizedMac;

    bool found = false;

    // Retry every 0.5s up to timeoutSeconds
    for (int i = 0; i < timeoutSeconds * 2; ++i) {
        GVariant* reply = g_dbus_connection_call_sync(
            conn,
            "org.bluez",
            "/",
            "org.freedesktop.DBus.ObjectManager",
            "GetManagedObjects",
            nullptr,     // No parameters
            nullptr,     // Let GLib figure out return type
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );

        if (!reply) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        GVariantIter* iter = nullptr;
        gchar* objectPath = nullptr;
        GVariant* interfaces = nullptr;

        g_variant_get(reply, "(a{oa{sa{sv}}})", &iter);
        while (g_variant_iter_loop(iter, "{oa{sa{sv}}}", &objectPath, &interfaces)) {
            if (targetPath == objectPath) {
                found = true;
                break;
            }
        }

        g_variant_iter_free(iter);
        g_variant_unref(reply);

        if (found) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Stop scanning
    g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        adapterPath.c_str(),
        "org.bluez.Adapter1",
        "StopDiscovery",
        nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        nullptr
    );

    g_object_unref(conn);

    if (found) {
        std::cout << "✅ Device " << targetMac << " found.\n";
    } else {
        std::cerr << "❌ Device " << targetMac << " not found after scanning.\n";
    }

    return found;
}




std::string BleService::findAdapter() {
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!conn) return "";

    GVariant* result = g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        "/",
        "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects",
        nullptr,
        G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        nullptr
    );

    if (!result) {
        g_object_unref(conn);
        return "";
    }

    GVariantIter* iter;
    g_variant_get(result, "(a{oa{sa{sv}}})", &iter);

    gchar* objectPath;
    GVariantIter* interfaces;

    while (g_variant_iter_loop(iter, "{oa{sa{sv}}}", &objectPath, &interfaces)) {
        gchar* interfaceName;
        GVariantIter* properties;
        while (g_variant_iter_loop(interfaces, "{sa{sv}}", &interfaceName, &properties)) {
            if (std::string(interfaceName) == "org.bluez.Adapter1") {
                std::string path = objectPath;
                // Do NOT free iter, interfaces, or properties
                g_variant_unref(result);
                g_object_unref(conn);
                return path;
            }
        }
    }

    g_variant_unref(result);
    g_object_unref(conn);
    return "";
}


void BleService::onPropertiesChanged(GDBusConnection*,
                                     const gchar*,
                                     const gchar*,
                                     const gchar*,
                                     const gchar*,
                                     GVariant* parameters,
                                     gpointer user_data)
{
    auto* self = static_cast<BleService*>(user_data);
    if (!self) return;

    const gchar* iface = nullptr;
    GVariant* changedProps = nullptr;
    GVariant* invalidatedProps = nullptr;
    g_variant_get(parameters, "(&s@a{sv}@as)", &iface, &changedProps, &invalidatedProps);
    if (!changedProps) return;

    GVariantIter iter;
    gchar* key;
    GVariant* value;
    g_variant_iter_init(&iter, changedProps);

    while (g_variant_iter_next(&iter, "{sv}", &key, &value)) {
        if (std::string(key) == "Value") {
            gsize len = 0;
            const guint8* data = (const guint8*)g_variant_get_fixed_array(value, &len, sizeof(guint8));
            if (len == 0) continue;
            if (data[0] == 231) { //check if start byte is correct
                uint16_t obtained_value;
                memcpy(&obtained_value,&data[1],2);
                self->onHeartRateReceived(static_cast<float>(obtained_value));
            }
        }
        g_free(key);
        g_variant_unref(value);
    }

    g_variant_unref(changedProps);
    if (invalidatedProps) g_variant_unref(invalidatedProps);
}

bool BleService::enableHeartRateNotifications(const std::string& deviceMac) {
    std::string devicePath = adapterPath + "/dev_" + deviceMac;
    std::replace(devicePath.begin(), devicePath.end(), ':', '_');

    GError* error = nullptr;
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!conn) {
        std::cerr << "❌ Failed to connect to system bus: " << error->message << "\n";
        g_error_free(error);
        return false;
    }

    GVariant* reply = g_dbus_connection_call_sync(
        conn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error
    );

    if (!reply) {
        std::cerr << "❌ GetManagedObjects failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }

    GVariantIter* objIter = nullptr;
    g_variant_get(reply, "(a{oa{sa{sv}}})", &objIter);

    // TODO: Replace with proper scanning
    std::string heartCharPath = "/org/bluez/hci0/dev_A0_DD_6C_AF_73_9E/service0028/char0029";

    std::cout << "🚀 [5] Starting notifications on: " << heartCharPath << "\n";

    if (heartCharPath.empty()) {
        std::cerr << "❌ Heart Rate characteristic path is empty.\n";
        g_object_unref(conn);
        return false;
    }

    g_dbus_connection_call_sync(
        conn,
        "org.bluez", heartCharPath.c_str(),
        "org.bluez.GattCharacteristic1", "StartNotify",
        nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error
    );

    if (error) {
        std::cerr << "❌ StartNotify failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }

    g_dbus_connection_signal_subscribe(
        conn,
        "org.bluez", "org.freedesktop.DBus.Properties", "PropertiesChanged",
        heartCharPath.c_str(), "org.bluez.GattCharacteristic1",
        G_DBUS_SIGNAL_FLAGS_NONE,
        &BleService::onPropertiesChanged,
        this, nullptr
    );

    m_loop = g_main_loop_new(nullptr, FALSE);
    m_loopThread = std::thread([this] {
        g_main_loop_run(m_loop);
    });

    //TODO: May be needed to disconnect
    //g_variant_unref(reply);
    //g_object_unref(conn);
    return true;
}


void BleService::handleHeartRateData(const guint8* data, gsize len) {
    std::cout << "📦 Data (" << len << " bytes): ";
    for (gsize i = 0; i < len; ++i) {
        std::cout << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::endl;

    uint8_t flags = data[0];
    bool is16bit = flags & 0x01;
    uint16_t heartRate = is16bit && len >= 3
        ? (data[1] | (data[2] << 8))
        : data[1];

    std::cout << "❤️  Heart Rate: " << heartRate << " bpm\n";
    onHeartRateReceived(static_cast<float>(heartRate));
}


std::vector<BleDeviceInfo> BleService::scanDevices(int timeoutSeconds) {
    std::vector<BleDeviceInfo> devices;
    GError* error = nullptr;

    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!conn) {
        std::cerr << "❌ Failed to connect to system bus: " << error->message << "\n";
        g_error_free(error);
        return devices;
    }

    g_dbus_connection_call_sync(
        conn, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
        "StartDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        std::cerr << "❌ StartDiscovery failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(conn);
        return devices;
    }

    std::this_thread::sleep_for(std::chrono::seconds(timeoutSeconds));

    GVariant* reply = g_dbus_connection_call_sync(
        conn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (!reply) {
        std::cerr << "❌ GetManagedObjects failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(conn);
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
        conn, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
        "StopDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, nullptr);

    g_object_unref(conn);
    return devices;
}


std::string parseMacFlexible(const std::string& input) {
    try {
        // Try to parse as JSON
        nlohmann::json j = nlohmann::json::parse(input);

        if (j.is_string()) {
            // Case: raw JSON string like "AA:BB:CC:DD:EE:FF"
            return j.get<std::string>();
        } else if (j.is_object() && j.contains("mac") && j["mac"].is_string()) {
            // Case: JSON object like { "mac": "AA:BB:CC:DD:EE:FF" }
            return j["mac"].get<std::string>();
        }
    } catch (...) {
        // Fallback to manual processing
    }

    // Fallback: remove surrounding quotes manually
    std::regex quoteRegex(R"(^["'](.*)["']$)");
    std::smatch match;
    if (std::regex_match(input, match, quoteRegex)) {
        return match[1];
    }

    // Return as-is
    return input;
}
bool BleService::connectToDevice(const std::string& jsonmac) {
    std::string mac =  parseMacFlexible(jsonmac);
    std::string path = "/org/bluez/hci0/dev_" + mac;
    for (auto& ch : path) if (ch == ':') ch = '_';

    GError* error = nullptr;
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!conn) return false;
    std::cout<<"path: "<<path<<std::endl;
    g_dbus_connection_call_sync(
        conn, "org.bluez", 
        path.c_str(), 
        "org.bluez.Device1",
        "Connect", 
        nullptr, 
        nullptr, 
        G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        std::cerr << "❌ Connect failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }
    LOGI("connected, returning");

    currentConnectedDevices.emplace_back();
    g_object_unref(conn);
    return true;
}

std::vector<BleServiceInfo> BleService::getServicesAndCharacteristics(const std::string& mac) {
    // As services and characteristics will be required after the connection
    // established, therefore, mac address can be taken from global private variable
    // const std::string& mac = macAddress;
    std::vector<BleServiceInfo> services;
    std::string basePath = "/org/bluez/hci0/dev_" + mac;
    for (auto& ch : basePath) if (ch == ':') ch = '_';

    GError* error = nullptr;
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!conn) return services;

    GVariant* reply = g_dbus_connection_call_sync(
        conn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (!reply) return services;

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
            if (iface == "org.bluez.GattService1") {
                std::string uuid;
                GVariantIter* props;
                g_variant_get(propDict, "a{sv}", &props);
                const gchar* key;
                GVariant* val;

                while (g_variant_iter_loop(props, "{sv}", &key, &val)) {
                    if (std::string(key) == "UUID") uuid = g_variant_get_string(val, nullptr);
                }
                g_variant_iter_free(props);
                serviceMap[path] = BleServiceInfo{uuid, {}};
            }
            else if (iface == "org.bluez.GattCharacteristic1") {
                std::string uuid, parent;
                bool notifying = false;

                GVariantIter* props;
                g_variant_get(propDict, "a{sv}", &props);
                const gchar* key;
                GVariant* val;
                BleCharacteristicInfo cInfo;

                while (g_variant_iter_loop(props, "{sv}", &key, &val)) {
                    std::string keyStr = key;
                    if (keyStr == "UUID") {
                        uuid = g_variant_get_string(val, nullptr);
                    } else if (keyStr == "Service") {
                        parent = g_variant_get_string(val, nullptr);
                    } else if (keyStr == "Notifying") {
                        notifying = g_variant_get_boolean(val);
                    } else if (keyStr == "Flags") {
                        if (g_variant_is_of_type(val, G_VARIANT_TYPE("as"))) {
                            GVariantIter* flagIter;
                            gchar* flagStr;
                            g_variant_get(val, "as", &flagIter);
                            while (g_variant_iter_loop(flagIter, "s", &flagStr)) {
                                cInfo.properties.push_back(flagStr);  // append to vector
                            }
                            g_variant_iter_free(flagIter);
                        }
                    }
                }
                g_variant_iter_free(props);

                if (!uuid.empty() && !parent.empty()) {
                    
                    cInfo.uuid = uuid;
                    cInfo.value = objectPath; // Use object path as value
                    cInfo.notifying = notifying;
                    serviceMap[parent].characteristics.push_back(cInfo);
                }
            }
        }
        g_variant_iter_free(ifaceIter);
    }

    g_variant_iter_free(iter);
    g_variant_unref(reply);
    g_object_unref(conn);

    for (auto& [_, svc] : serviceMap) {
        services.push_back(std::move(svc));
    }
    return services;
}

// bool BleService::enableServices(const std::string& mac, const std::vector<std::string>& serviceUUIDs) {
bool BleService::enableServices(const std::string& body){
    std::cout << "✅ Services selected for " << body<< ":\n";
    // for (const auto& uuid : serviceUUIDs) {
    //     std::cout << "  - " << uuid << "\n";
    // }
    // might store them for later use, e.g., streaming
    return true;
}

void BleService::printScanResults(const std::vector<BleDeviceInfo>& devices) {
    std::cout << "\n📋 Discovered BLE Devices:\n";
    for (const auto& dev : devices) {
        std::cout << "  🔹 MAC: " << dev.mac
                  << ", Name: " << dev.name
                  << ", RSSI: " << dev.rssi << " dBm" << std::endl;
    }
    std::cout << std::endl;
}

// BleService::BleService() {
//     GError* error = nullptr;
//     conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
//     if (!conn) {
//         std::cerr << "❌ Could not connect to system bus: " << error->message << std::endl;
//         g_error_free(error);
//     }
// }

//TODO: may be later to destroy the class
// BleService::~BleService() {
//     if (conn) g_object_unref(conn);
// }

std::string BleService::macToDevicePath(const std::string& mac) {
    std::string path = "/org/bluez/hci0/dev_" + mac;
    for (char& c : path) if (c == ':') c = '_';
    return path;
}

bool BleService::disconnectDevice(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(conn, "org.bluez", path.c_str(), "org.bluez.Device1",
                                 "Disconnect", nullptr, nullptr,
                                 G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error) {
        std::cerr << "⚠️ Disconnect failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    return true;
}

bool BleService::removeDevice(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(conn, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
                                 "RemoveDevice",
                                 g_variant_new("(o)", path.c_str()),
                                 nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error) {
        std::cerr << "⚠️ RemoveDevice failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    return true;
}

bool BleService::isConnected(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    GVariant* result = g_dbus_connection_call_sync(conn, "org.bluez", path.c_str(),
        "org.freedesktop.DBus.Properties", "Get",
        g_variant_new("(ss)", "org.bluez.Device1", "Connected"),
        G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (!result) {
        std::cerr << "⚠️ Check Connected failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    GVariant* val;
    g_variant_get(result, "(v)", &val);
    bool connected = g_variant_get_boolean(val);
    g_variant_unref(val);
    g_variant_unref(result);
    return connected;
}

bool BleService::isPaired(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    GVariant* result = g_dbus_connection_call_sync(conn, "org.bluez", path.c_str(),
        "org.freedesktop.DBus.Properties", "Get",
        g_variant_new("(ss)", "org.bluez.Device1", "Paired"),
        G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (!result) {
        std::cerr << "⚠️ Check Paired failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    GVariant* val;
    g_variant_get(result, "(v)", &val);
    bool paired = g_variant_get_boolean(val);
    g_variant_unref(val);
    g_variant_unref(result);
    return paired;
}

bool BleService::pairDevice(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(conn, "org.bluez", path.c_str(), "org.bluez.Device1",
        "Pair", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error) {
        std::cerr << "⚠️ Pair failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    return true;
}

bool BleService::cancelPairing(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(conn, "org.bluez", path.c_str(), "org.bluez.Device1",
        "CancelPairing", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error) {
        std::cerr << "⚠️ CancelPairing failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    return true;
}

bool BleService::trustDevice(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(conn, "org.bluez", path.c_str(), "org.freedesktop.DBus.Properties",
        "Set", g_variant_new("(ssv)", "org.bluez.Device1", "Trusted",
        g_variant_new_boolean(true)), nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (error) {
        std::cerr << "⚠️ TrustDevice failed: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    return true;
}

std::vector<std::string> BleService::getConnectedDevices() {
    std::vector<std::string> devices;
    GError* error = nullptr;
    GVariant* reply = g_dbus_connection_call_sync(conn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (!reply) {
        std::cerr << "⚠️ Failed to get managed objects: " << error->message << std::endl;
        g_error_free(error);
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

bool BleService::cleanupDisconnectedDevices() {
    for (const auto& path : getConnectedDevices()) {
        // No-op: implement logic here for removing based on timestamp if stored
        std::cout << "✔️ Device connected: " << path << std::endl;
    }
    return true;
    // Extend with timestamp check and call removeDevice(...) if needed
}