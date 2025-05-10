// #include "ble.hpp"
// #include <iostream>
// #include <thread>
// #include <chrono>
// #include <gio/gio.h>
// #include <algorithm>

// #include <cstdint>

// static void onInterfacesAdded(GDBusConnection* conn,
//     const gchar* sender_name,
//     const gchar* object_path,
//     const gchar* interface_name,
//     const gchar* signal_name,
//     GVariant* parameters,
//     gpointer user_data) {
//         GVariantIter* interfaces;
//         const gchar* objPath;

//         g_variant_get(parameters, "(&o@a{sa{sv}})", &objPath, &interfaces);
//         gchar* deviceName = nullptr;

//         // Print basic device info
//         std::string pathStr(objPath);
//         if (pathStr.find("/dev_") != std::string::npos) {
//             std::string mac = pathStr.substr(pathStr.find("/dev_") + 5);
//             std::replace(mac.begin(), mac.end(), '_', ':');
//             std::cout << "Found device: " << mac << "\n";

//             const gchar* iface;
//             GVariant* properties;
//             while (g_variant_iter_next(interfaces, "{&s@a{sv}}", &iface, &properties)) {
//                 if (std::string(iface) == "org.bluez.Device1") {
//                     GVariantIter* propIter;
//                     g_variant_get(properties, "a{sv}", &propIter);
//                     const gchar* key;
//                     GVariant* value;
//                     while (g_variant_iter_next(propIter, "{&sv}", &key, &value)) {
//                         if (std::string(key) == "Name") {
//                             std::cout << "  Name: " << g_variant_get_string(value, nullptr) << "\n";
//                         }
//                         g_variant_unref(value);
//                     }
//                     g_variant_iter_free(propIter);
//                 }
//                 g_variant_unref(properties);
//             }
//         g_variant_iter_free(interfaces);
//         }
// }


// BleClient::BleClient(const std::string& deviceMac)
//     : m_deviceMac(deviceMac) {}

// BleClient::~BleClient() {}

// void BleClient::scanDevices(int durationSeconds, void* userData) {
//     GError* error = nullptr;
//     GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
//     if (!conn) {
//         std::cerr << "Failed to get system bus: " << error->message << "\n";
//         g_error_free(error);
//         return;
//     }

//     // --- Signal handler ---
//     auto onInterfacesAdded = [](GDBusConnection*, const gchar*, const gchar* object_path,
//                                 const gchar*, const gchar*, GVariant* parameters, gpointer user_data) {
//         std::string* targetMac = static_cast<std::string*>(user_data);
//         const gchar* objPath;
//         GVariant* interfacesVariant;
//         g_variant_get(parameters, "(&o@a{sa{sv}})", &objPath, &interfacesVariant);

//         std::string pathStr(objPath);
//         if (pathStr.find("/dev_") != std::string::npos) {
//             std::string mac = pathStr.substr(pathStr.find("/dev_") + 5);
//             std::replace(mac.begin(), mac.end(), '_', ':');
//             std::cout<<"SCAN:mac:"<<mac<<std::endl;

//             GVariantIter interfacesIter;
//             g_variant_iter_init(&interfacesIter, interfacesVariant);

//             const gchar* iface;
//             GVariant* props;
//             while (g_variant_iter_next(&interfacesIter, "{&s@a{sv}}", &iface, &props)) {
//                 if (std::string(iface) == "org.bluez.Device1") {
//                     std::string name;

//                     GVariantIter propIter;
//                     g_variant_iter_init(&propIter, props);

//                     const gchar* key;
//                     GVariant* val;
//                     while (g_variant_iter_next(&propIter, "{&sv}", &key, &val)) {
//                         if (std::string(key) == "Name") {
//                             name = g_variant_get_string(val, nullptr);
//                         }
//                         g_variant_unref(val);
//                     }

//                     if (name == "blehr_sensor_1.0") {
//                         std::cout << "✅ Found target device: " << mac << " (" << name << ")\n";
//                         *targetMac = mac;
//                     }

//                     g_variant_unref(props);
//                 } else {
//                     g_variant_unref(props);
//                 }
//             }

//             g_variant_unref(interfacesVariant);
//         }
//     };

//     // --- Subscribe ---
//     unsigned int sub_id = g_dbus_connection_signal_subscribe(
//         conn,
//         "org.bluez",
//         "org.freedesktop.DBus.ObjectManager",
//         "InterfacesAdded",
//         nullptr,
//         nullptr,
//         G_DBUS_SIGNAL_FLAGS_NONE,
//         onInterfacesAdded,
//         userData,
//         nullptr
//     );

//     // --- Start discovery ---
//     GVariant* result = g_dbus_connection_call_sync(
//         conn,
//         "org.bluez",
//         "/org/bluez/hci0",
//         "org.bluez.Adapter1",
//         "StartDiscovery",
//         nullptr, nullptr,
//         G_DBUS_CALL_FLAGS_NONE,
//         -1, nullptr, &error
//     );

//     if (!result) {
//         std::cerr << "StartDiscovery failed: " << error->message << "\n";
//         g_error_free(error);
//         g_dbus_connection_signal_unsubscribe(conn, sub_id);
//         g_object_unref(conn);
//         return;
//     }
//     g_variant_unref(result);

//     GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
//     std::thread loopThread([&]() {
//         g_main_loop_run(loop);
//     });

//     std::cout << "Scanning for " << durationSeconds << " seconds...\n";
//     std::this_thread::sleep_for(std::chrono::seconds(durationSeconds));

//     g_dbus_connection_call_sync(conn, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
//                                 "StopDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE,
//                                 -1, nullptr, nullptr);

//     g_dbus_connection_signal_unsubscribe(conn, sub_id);
//     g_main_loop_quit(loop);
//     loopThread.join();
//     g_main_loop_unref(loop);
//     g_object_unref(conn);
// }



// // void BleClient::scanDevices(int durationSeconds) {
// //     GError* error = nullptr;
// //     GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
// //     if (!conn) {
// //         std::cerr << "Failed to get system bus: " << error->message << "\n";
// //         g_error_free(error);
// //         return;
// //     }

// //     // Callback for InterfacesAdded signal
// //     auto onInterfacesAdded = [](GDBusConnection*,
// //                                 const gchar*,
// //                                 const gchar* object_path,
// //                                 const gchar*,
// //                                 const gchar*,
// //                                 GVariant* parameters,
// //                                 gpointer) {
// //         const gchar* objPath;
// //         GVariant* interfacesVariant;

// //         g_variant_get(parameters, "(&o@a{sa{sv}})", &objPath, &interfacesVariant);

// //         std::string pathStr(objPath);
// //         if (pathStr.find("/dev_") != std::string::npos) {
// //             std::string mac = pathStr.substr(pathStr.find("/dev_") + 5);
// //             std::replace(mac.begin(), mac.end(), '_', ':');

// //             GVariantIter interfacesIter;
// //             g_variant_iter_init(&interfacesIter, interfacesVariant);

// //             const gchar* iface;
// //             GVariant* props;
// //             while (g_variant_iter_next(&interfacesIter, "{&s@a{sv}}", &iface, &props)) {
// //                 if (std::string(iface) == "org.bluez.Device1") {
// //                     std::string name;

// //                     GVariantIter propIter;
// //                     g_variant_iter_init(&propIter, props);

// //                     const gchar* key;
// //                     GVariant* val;
// //                     while (g_variant_iter_next(&propIter, "{&sv}", &key, &val)) {
// //                         if (std::string(key) == "Name") {
// //                             name = g_variant_get_string(val, nullptr);
// //                         }
// //                         g_variant_unref(val);
// //                     }

// //                     std::cout << "Found device: " << mac;
// //                     if (!name.empty()) {
// //                         std::cout << " (" << name << ")";
// //                     }
// //                     std::cout << "\n";

// //                     g_variant_unref(props);
// //                 } else {
// //                     g_variant_unref(props);
// //                 }
// //             }
// //             g_variant_unref(interfacesVariant);
// //         }
// //     };

// //     // Subscribe to InterfacesAdded signal
// //     unsigned int sub_id = g_dbus_connection_signal_subscribe(
// //         conn,
// //         "org.bluez",
// //         "org.freedesktop.DBus.ObjectManager",
// //         "InterfacesAdded",
// //         nullptr,
// //         nullptr,
// //         G_DBUS_SIGNAL_FLAGS_NONE,
// //         onInterfacesAdded,
// //         nullptr,
// //         nullptr
// //     );

// //     // Start discovery
// //     GVariant* result = g_dbus_connection_call_sync(
// //         conn,
// //         "org.bluez",
// //         "/org/bluez/hci0",
// //         "org.bluez.Adapter1",
// //         "StartDiscovery",
// //         nullptr,
// //         nullptr,
// //         G_DBUS_CALL_FLAGS_NONE,
// //         -1,
// //         nullptr,
// //         &error
// //     );

// //     if (!result) {
// //         std::cerr << "StartDiscovery failed: " << error->message << "\n";
// //         g_error_free(error);
// //         g_dbus_connection_signal_unsubscribe(conn, sub_id);
// //         g_object_unref(conn);
// //         return;
// //     }
// //     g_variant_unref(result);

// //     // Run GLib main loop in another thread
// //     GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
// //     std::thread loopThread([&]() {
// //         g_main_loop_run(loop);
// //     });

// //     std::cout << "Scanning for " << durationSeconds << " seconds...\n";
// //     std::this_thread::sleep_for(std::chrono::seconds(durationSeconds));

// //     // Stop discovery
// //     result = g_dbus_connection_call_sync(
// //         conn,
// //         "org.bluez",
// //         "/org/bluez/hci0",
// //         "org.bluez.Adapter1",
// //         "StopDiscovery",
// //         nullptr,
// //         nullptr,
// //         G_DBUS_CALL_FLAGS_NONE,
// //         -1,
// //         nullptr,
// //         &error
// //     );

// //     if (!result) {
// //         std::cerr << "StopDiscovery failed: " << error->message << "\n";
// //         g_error_free(error);
// //     } else {
// //         g_variant_unref(result);
// //     }

// //     // Cleanup
// //     g_dbus_connection_signal_unsubscribe(conn, sub_id);
// //     g_main_loop_quit(loop);
// //     loopThread.join();
// //     g_main_loop_unref(loop);
// //     g_object_unref(conn);
// // }


// bool BleClient::connectToDevice() {
//     std::string devicePath = "/org/bluez/hci0/dev_" + m_deviceMac;
//     std::replace(devicePath.begin(), devicePath.end(), ':', '_');
//     std::cout<<"device path:"<<devicePath<<std::endl;

//     GError* error = nullptr;
//     GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
//     if (!conn) {
//         std::cerr << "Failed to get DBus system connection.\n";
//         return 0;
//     }

//     // Wait for the object to exist (retry for ~2s)
//     bool deviceExists = false;
//     for (int i = 0; i < 20; ++i) {
//         GVariant* result = g_dbus_connection_call_sync(
//             conn,
//             "org.bluez",
//             devicePath.c_str(),
//             "org.freedesktop.DBus.Properties",
//             "GetAll",
//             g_variant_new("(s)", "org.bluez.Device1"),
//             G_VARIANT_TYPE("(a{sv})"),
//             G_DBUS_CALL_FLAGS_NONE,
//             100 * 1000, // 100 ms
//             nullptr,
//             &error
//         );

//         if (result) {
//             deviceExists = true;
//             g_variant_unref(result);
//             break;
//         } else {
//             g_error_free(error);
//             error = nullptr;
//             std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         }
//     }

//     g_object_unref(conn);

//     if (!deviceExists) {
//         std::cerr << "❌ Device object did not appear on D-Bus.\n";
//         return 0;
//     }
//     return 1;
// }

// bool BleClient::subscribeToBle(const BleCallback& callback) {
//     // Replace with your real characteristic path
//     m_hrCharPath = "/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/service0010/char0011";

//     GError* error = nullptr;
//     GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
//     if (!conn) {
//         std::cerr << "DBus error: " << error->message << "\n";
//         g_error_free(error);
//         return false;
//     }

//     GVariant* result = g_dbus_connection_call_sync(
//         conn,
//         "org.bluez",
//         m_hrCharPath.c_str(),
//         "org.bluez.GattCharacteristic1",
//         "StartNotify",
//         nullptr,
//         nullptr,
//         G_DBUS_CALL_FLAGS_NONE,
//         -1,
//         nullptr,
//         &error
//     );

//     if (!result) {
//         std::cerr << "Notify failed: " << error->message << "\n";
//         g_error_free(error);
//         g_object_unref(conn);
//         return false;
//     }

//     g_variant_unref(result);
//     g_object_unref(conn);
//     std::cout << "Subscribed to Heart Rate notifications.\n";

//     m_callback = callback;

//     // D-Bus signal handling for real notifications is not implemented here
//     return true;
// }

// void BleClient::onBleNotification(const uint8_t* data, size_t len) {
//     if (len < 2) return;
//     uint8_t bpm = data[1]; // Simplified, flags not handled
//     if (m_callback) {
//         m_callback(bpm);
//     }
// }



#include "ble.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <gio/gio.h>

BleClient::BleClient(const std::string& name) : targetName(name) {
    if (!initBluez()) {
        std::cerr << "Failed to initialize BlueZ." << std::endl;
    }
}

BleClient::~BleClient() {}

bool BleClient::initBluez() {
    adapterPath = findAdapter();
    return !adapterPath.empty();
}

std::string BleClient::findAdapter() {
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

    if (!result) return "";

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
                g_variant_iter_free(properties);
                g_variant_iter_free(interfaces);
                g_variant_iter_free(iter);
                g_variant_unref(result);
                g_object_unref(conn);
                return path;
            }
        }
        g_variant_iter_free(interfaces);
    }

    g_variant_iter_free(iter);
    g_variant_unref(result);
    g_object_unref(conn);
    return "";
}

void BleClient::startScan(DeviceFoundCallback onDeviceFound) {
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!conn) return;

    GError* error = nullptr;
    g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        adapterPath.c_str(),
        "org.bluez.Adapter1",
        "StartDiscovery",
        nullptr,
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error
    );

    if (error) {
        std::cerr << "Failed to start discovery: " << error->message << std::endl;
        g_error_free(error);
    }

    std::cout << "Scanning for BLE devices for 10 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        adapterPath.c_str(),
        "org.bluez.Adapter1",
        "StopDiscovery",
        nullptr,
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        nullptr
    );

    g_object_unref(conn);
}

bool BleClient::connectToDevice(const std::string& address) {
    std::string devicePath = adapterPath + "/dev_" + address;
    for (auto& ch : devicePath) if (ch == ':') ch = '_';

    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!conn) return false;

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
        std::cerr << "Connection failed: " << error->message << std::endl;
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }

    g_object_unref(conn);
    return true;
}
bool BleClient::enableHeartRateNotifications(const std::string& deviceMac) {
    // Prepare the D-Bus object paths
    std::string devicePath = adapterPath + "/dev_" + deviceMac;
    for (auto& ch : devicePath) if (ch == ':') ch = '_';

    std::string charPath = devicePath + "/service000a/char000b";

    std::cout << "Notification path:\n" << charPath << "\n";

    // Get the system D-Bus connection
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!conn) {
        std::cerr << "❌ Failed to get system bus.\n";
        return false;
    }

    // Start notifications (BlueZ handles CCC descriptor internally)
    GError* error = nullptr;
    g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        charPath.c_str(),
        "org.bluez.GattCharacteristic1",
        "StartNotify",
        nullptr,
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error
    );

    if (error) {
        std::cerr << "❌ Failed to enable notification: " << error->message << std::endl;
        g_error_free(error);
        g_object_unref(conn);
        return false;
    }

    // Subscribe to Value property changes (notifications)
    g_dbus_connection_signal_subscribe(
        conn,
        "org.bluez",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        charPath.c_str(),
        "org.bluez.GattCharacteristic1",
        G_DBUS_SIGNAL_FLAGS_NONE,
        [](GDBusConnection*, const gchar*, const gchar*, const gchar*, const gchar*, GVariant* parameters, gpointer) {
            GVariantIter* props = nullptr;
            const gchar* iface = nullptr;

            g_variant_get(parameters, "(&sa{sv}as)", &iface, &props, nullptr);

            const gchar* key = nullptr;
            GVariant* value = nullptr;
            while (g_variant_iter_loop(props, "{sv}", &key, &value)) {
                if (std::string(key) == "Value") {
                    gsize len = 0;
                    const guint8* data = (const guint8*)g_variant_get_fixed_array(value, &len, sizeof(guint8));
                    if (len >= 2) {
                        uint8_t heartRate = data[1]; // 2nd byte typically contains HR
                        std::cout << "❤️  Heart Rate: " << (int)heartRate << " bpm" << std::endl;
                    }
                }
            }
        },
        nullptr,
        nullptr
    );

    std::cout << "✅ Notifications enabled on Heart Rate Measurement.\n";

    // Run GLib main loop to receive signals
    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_object_unref(conn);
    return true;
}
