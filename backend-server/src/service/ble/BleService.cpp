#include "BleService.hpp"
#include <nlohmann/json.hpp>
#include <gio/gio.h>

bool BleService::start() {
    if (running) return true;
    std::cout << "BLE Service started\n";
    running = true;
    // TODO: connect to device, subscribe to notifications
    if (!initBluez()) {
        std::cerr << "Failed to initialize BlueZ." << std::endl;
        return false;
    }
    // TODO: later comes from configure json
    macAddress = "A0:DD:6C:AF:73:9E";
    if (!connectToDevice(macAddress)){
        std::cerr << "Failed to Connect" << std::endl;
        return false;
    } 
    enableHeartRateNotifications(macAddress);
    
   
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
    // std::cout << "Heart rate updated: " << hr << "\n";
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

bool BleService::connectToDevice(const std::string& address) {
    std::cout << "Connecting to device " << address << "...\n";
    std::string devicePath = adapterPath + "/dev_" + address;
    for (auto& ch : devicePath) if (ch == ':') ch = '_';

    // Step 1: Wait until device path is available
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!conn) {
        std::cerr << "❌ Failed to connect to system bus.\n";
        return false;
    }

    bool found = false;
    for (int i = 0; i < 10; ++i) {  // retry 10 times max
        GError* error = nullptr;
        GVariant* reply = g_dbus_connection_call_sync(
            conn,
            "org.bluez",
            "/",
            "org.freedesktop.DBus.ObjectManager",
            "GetManagedObjects",
            nullptr,
            nullptr,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            &error
        );

        if (!reply) {
            std::cerr << "❌ GetManagedObjects failed: " << error->message << std::endl;
            g_error_free(error);
            g_object_unref(conn);
            return false;
        }

        GVariantIter* iter;
        gchar* objectPath;
        GVariant* interfaces;
        g_variant_get(reply, "(a{oa{sa{sv}}})", &iter);

        while (g_variant_iter_loop(iter, "{oa{sa{sv}}}", &objectPath, &interfaces)) {
            if (devicePath == std::string(objectPath)) {
                found = true;
                break;
            }
        }

        g_variant_iter_free(iter);
        g_variant_unref(reply);

        if (found) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!found) {
        std::cerr << "❌ Device not found on D-Bus: " << devicePath << std::endl;
        g_object_unref(conn);
        return false;
    }

    // Step 2: Call Connect
    GError* error = nullptr;
    g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        devicePath.c_str(),
        "org.bluez.Device1",
        "Connect",
        nullptr,
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error
    );


    if (error) {
        std::cerr << "❌ Connection failed: " << error->message << std::endl;
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }

    g_object_unref(conn);
    std::cout << "✅ Connected to " << address << "\n";
    return true;
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
