#include "BleService.hpp"
#include "config/LogAdapt.hpp"
#include <nlohmann/json.hpp>
#include <regex>

#include "websocket/GraphWebSocket.hpp"

bool BleService::initBluez() {
    adapterPath = findAdapter();
    return !adapterPath.empty();
}

std::string BleService::findAdapter() {
    // Connect to the system bus
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!gDBusConn) return "";

    GVariant* result = g_dbus_connection_call_sync(
        gDBusConn,
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
        g_object_unref(gDBusConn);
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
                g_object_unref(gDBusConn);
                return path;
            }
        }
    }
    g_variant_unref(result);
    g_object_unref(gDBusConn);
    return "";
}


bool BleService::start() {
    if (running) return true;
    LOGI("Starting BLE Service.");
    running = true;
    if (!initBluez()) {
        LOGE("Failed to initialize BlueZ.");
        return false;
    }
    return true;
}

void BleService::stop() {
    LOGI("BLE Service stopped");
    running = false;
    //TODO: Clear the connection and disconnect
    if (gDBusConn) {
        g_dbus_connection_call_sync(
            gDBusConn, "org.bluez", adapterPath.c_str(), "org.bluez.Adapter1",
            "StopDiscovery", nullptr, nullptr,
            G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr
        );
    }
    // Unsubscribe from all signals
    // for (auto id : signalSubscriptionIds) {
    //     g_dbus_connection_signal_unsubscribe(conn, id);
    // }
    // signalSubscriptionIds.clear();
    // Unref the connection

    if (gDBusConn) {
        g_object_unref(gDBusConn);
        gDBusConn = nullptr;
    }
    //TODO: other clearing
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

std::vector<BleDeviceInfo> BleService::scanDevices(int timeoutSeconds) {
    std::vector<BleDeviceInfo> devices;
    GError* error = nullptr;

    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!gDBusConn) {
        std::cerr << "❌ Failed to connect to system bus: " << error->message << "\n";
        g_error_free(error);
        return devices;
    }

    g_dbus_connection_call_sync(
        gDBusConn, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
        "StartDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (error) {
        std::cerr << "❌ StartDiscovery failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(gDBusConn);
        return devices;
    }
    std::this_thread::sleep_for(std::chrono::seconds(timeoutSeconds));
    GVariant* reply = g_dbus_connection_call_sync(
        gDBusConn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (!reply) {
        std::cerr << "❌ GetManagedObjects failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(gDBusConn);
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
        gDBusConn, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1",
        "StopDiscovery", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, nullptr);

    g_object_unref(gDBusConn);
    return devices;
}

std::string parseMacFlexible(const std::string& input) {
    try {
        // Try to parse as JSON
        nlohmann::json j = nlohmann::json::parse(input);
        if (j.is_string()) {
            return j.get<std::string>();
        } else if (j.is_object() && j.contains("mac") && j["mac"].is_string()) {
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
    return input;
}

bool BleService::connectToDevice(const std::string& jsonmac) {
    std::string mac =  parseMacFlexible(jsonmac);
    std::string path = "/org/bluez/hci0/dev_" + mac;
    for (auto& ch : path) if (ch == ':') ch = '_';

    GError* error = nullptr;
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!gDBusConn) return false;
    LOGI("Connecting to device at path: {}", path);
    g_dbus_connection_call_sync(
        gDBusConn, "org.bluez", 
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
        g_object_unref(gDBusConn);
        return false;
    }
    LOGI("connected, returning");

    currentConnectedDevices.emplace_back();
    g_object_unref(gDBusConn);

    return true;
}

/* ------------------------------
Notifications related methods
------------------------------*/
std::vector<BleServiceInfo> BleService::getServicesAndCharacteristics(const std::string& mac) {

    std::vector<BleServiceInfo> services;
    std::string basePath = "/org/bluez/hci0/dev_" + mac;
    for (auto& ch : basePath) if (ch == ':') ch = '_';

    GError* error = nullptr;
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!gDBusConn) return services;

    GVariant* reply = g_dbus_connection_call_sync(
        gDBusConn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1,
        nullptr, &error);

    if (!reply) {
        LOGE("❌ GetManagedObjects failed: {}", error->message);
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
                serviceMap[path] = BleServiceInfo{
                    .name = uuidToName.count(uuid) ? uuidToName[uuid] : "Unknown",
                    .path = path,
                    .uuid = uuid,
                    .characteristics = {}
                };
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
                    cInfo.name = uuidToName.count(uuid) ? uuidToName[uuid] : "Unknown";
                    cInfo.uuid = uuid;
                    cInfo.path = objectPath; // Use object path as value
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
    g_object_unref(gDBusConn);

    for (auto& [_, svc] : serviceMap) {
        services.push_back(std::move(svc));
    }

    return services;
}

bool BleService::enableNotification(const std::string& path) {

    bool ret = startLoopFor(path);
    if (!ret) {
        LOGE("Failed to start loop for characteristic: {}", path);
        return false;
    }

    numOfActiveNotifications++;
    LOGI("Active notifications count: {}", numOfActiveNotifications);
    return true;
}


//disableHeartRateNotifications
bool BleService::disableNotification(const std::string& path) {
    if (path.empty()) {
        std::cerr << "❌ Characteristic path is empty.\n";
        return false;
    }

    stopLoopFor(path);

    bool ret = false;
    if (ret) {
        LOGE("Failed to start loop for characteristic: {}", path);
        return false;
    }
    
    numOfActiveNotifications--;
    if (numOfActiveNotifications < 0) {
        numOfActiveNotifications = 0; // Ensure it doesn't go negative
    }
    LOGI("Active notifications count: {}", numOfActiveNotifications);

    return true; 
}


std::string BleService::extractUuidPrefix(const std::string& uuid) {
    // Take substring before first dash
    auto dashPos = uuid.find('-');
    std::string firstPart = uuid.substr(0, dashPos);

    // Strip leading zeros
    auto nonZero = firstPart.find_first_not_of('0');
    return (nonZero == std::string::npos) ? "0" : firstPart.substr(nonZero);
}

//return number of floating point values
std::string BleService::toggleNotify(const std::string& body){
    LOGI("Toggling notify for  {}",body);
    auto json_data = nlohmann::json::parse(body);
    auto newJson = nlohmann::json::object();

    if (!json_data.contains("mac") || !json_data.contains("uuid")) {
        LOGE("Invalid request: 'mac' and 'uuid' are required.");
        newJson["status"] = "failed";
        newJson["message"] = "Invalid request: 'mac' and 'uuid' are required.";
        return newJson.dump();
    }
  
    std::string path = json_data["path"];
    std::string uuid = json_data["uuid"];
    std::string name = json_data["charName"];
    std::string extractedUuid = extractUuidPrefix(uuid);
    bool enable = json_data["enable"];
    if (enable) {
        LOGI("Enabling notification");

        // Here we want to inform front end, about number of values available
        uint8_t retData[10]; 
        size_t readLength = 0;
        //Custom characteristic (can be any amount)
        LOGI("Extracted UUID: {}",extractedUuid);
        if (extractedUuid == "f00d"){
            readLength = readCharacteristics(path, retData);
        }
        // Always one adc heart rate value
        else if (extractedUuid == "2a37"){
            readLength = 1;
            //TODO: needs better idea
            retData[0] = 1;
        }
        // pulse oximeter, always two, pulse and spo2
        else if (extractedUuid == "2a5f"){
            readLength = 1;
            retData[0] = 2;
        }
        
        LOGI("Read length {} from char {}",readLength, path);
        int ret = enableNotification(path);
        if (!ret) {
            // firstNotificationReceived.store(false);
            LOGE("Failed to enable notification for path: {}", path);
            newJson["status"] = "failed";
            newJson["message"] = "Failed to enable notification.";
            return newJson.dump();
        }

        if (readLength == 0){
            LOGE("Could not read length of custom characteristic");
        }
        charReg.upsert(path, extractedUuid, name, retData[0]);
        std::vector<std::string> temp = charReg.listNames();
        LOGI("first:{}",temp[0]);
        LOGI("Read length received, number of active notifications: {}", numOfActiveNotifications);
        newJson["number_of_values"] = retData[0];

    } else {
        LOGI("Disabling notification");
        disableNotification(path);
        charReg.eraseByPath(path);
    }
    LOGI("Returning response for toggle notify");
    newJson["status"] = "success";
    newJson["message"] = enable ? "Notification enabled" : "Notification disabled";
    return newJson.dump();
}


void BleService::onPropertiesChanged(GDBusConnection*,
                                     const gchar* sender_name,
                                     const gchar* object_path, // characteristic path
                                     const gchar* interface_name,
                                     const gchar* signal_name,
                                     GVariant* parameters,
                                     gpointer user_data)
{
    auto* self = static_cast<BleService*>(user_data);
    if (!self) return;

    // LOGI("PropertiesChanged signal received from: {}", object_path);
    const gchar* iface = nullptr;
    GVariant* changedProps = nullptr;
    GVariant* invalidatedProps = nullptr;
    g_variant_get(parameters, "(&s@a{sv}@as)", &iface, &changedProps, &invalidatedProps);
    if (!changedProps) return;

    GVariantIter iter;
    gchar* key;
    GVariant* value;
    g_variant_iter_init(&iter, changedProps);
    gsize len = 0;
    while (g_variant_iter_next(&iter, "{sv}", &key, &value)) {
        if (std::string(key) == "Value") {
            const guint8* data = (const guint8*)g_variant_get_fixed_array(value, &len, sizeof(guint8));
            LOGI("data length: {}", static_cast<int>(len));

            self->processData(object_path, data, len);
            if (len == 0) {
                LOGW("data length Zero, returning");
                continue;
            }
        }
        g_free(key);
        g_variant_unref(value);
    }
    g_variant_unref(changedProps);
    if (invalidatedProps) g_variant_unref(invalidatedProps);
}


/* -------------------------
Data process related methods
 -------------------------*/

void BleService::processData(const std::string& path, const guint8* data, gsize len) {
  //  std::lock_guard<std::mutex> lk(m_mx);
    if (path.empty() || !data || len == 0) {
        LOGE("Invalid data received for path: {}", path);
        return;
    }
    //if websocket is not connected, just return
    if (m_graphRunning.load() == false) {
        return;
    }
    //Add a timeout to avoid infinite loop
    static int timeout = 300; // 3 seconds max wait
    while(m_graphValueChanged.load() && timeout > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timeout--;
    }
    if (timeout <= 0) {
        LOGW("Timeout waiting for graph data to be processed, overwriting data");
    }

    std::string key;
    std::vector<float> convertedData;  
    auto all = charReg.listAll();
    for (const auto& meta : all) {
        LOGI("Registered path:  {}",meta.uuid);
    }

    if (auto metaOpt = charReg.getByPath(path)) {
        const auto& meta = *metaOpt;

        if (meta.uuid == "f00d"){
            convertedData = frameBuilder_.makeCustom5(data,len);
        }
        // Always one adc heart rate value
        else if (meta.uuid == "2a37"){
            convertedData = frameBuilder_.makeHeart(data, len);
        }
        // pulse oximeter, always two, pulse and spo2
        else if (meta.uuid == "2a5f"){
            convertedData = frameBuilder_.makePoX(data, len);
        }
        for (int i = 1; i < convertedData.size()+1; ++i) {
            key = meta.name +"."+ std::to_string(i);
            CurrentGraphValue convertedDataPoint;
            convertedDataPoint.value = convertedData[i-1];
            convertedDataPoint.updated = true;
            m_graphData[key] = convertedDataPoint;
            LOGI("key: {}",key);
            LOGI("convertedData[i]: {}",convertedData[i-1]);
        }
        m_graphValueChanged.store(true);
    } else {
        LOGE("Path not found");
    }
}

/* -----------------------------------------
Related to threads starting of notifications
 ------------------------------------------*/
bool BleService::startLoopFor(const std::string& key) {

    // Prepare GLib objects first
    GMainContext* ctx = g_main_context_new();
    if (!ctx) return false;

    GMainLoop* loop = g_main_loop_new(ctx, FALSE);
    if (!loop) {
        g_main_context_unref(ctx);
        return false;
    }

    // Start thread now so we know it’s good before inserting
    std::thread th;
    try {
        LOGI("starting loop for key: {}", key );
        th = std::thread([ctx,loop,key, this]() { 
            g_main_context_push_thread_default(ctx);

            GError* error = nullptr;
            gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
            if (!gDBusConn) {
                LOGE("❌ Failed to connect to system bus: ", error->message);
                g_error_free(error);
                return false;
            }

            GVariant* reply = g_dbus_connection_call_sync(
                gDBusConn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
                "GetManagedObjects", nullptr, nullptr,
                G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error
            );

            if (!reply) {
                std::cerr << "❌ GetManagedObjects failed: " << error->message << "\n";
                g_error_free(error);
                g_object_unref(gDBusConn);
                return false;
            }

            GVariantIter* objIter = nullptr;
            g_variant_get(reply, "(a{oa{sa{sv}}})", &objIter);

            LOGI("Starting notifications for characteristic: {}", key);

            if (key.empty()) {
                std::cerr << "Characteristic path is empty.\n";
                g_object_unref(gDBusConn);
                return false;
            }

            g_dbus_connection_call_sync(
                gDBusConn,
                "org.bluez", key.c_str(),
                "org.bluez.GattCharacteristic1", "StartNotify",
                nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error
            );

            if (error) {
                LOGE("StartNotify failed");
                g_error_free(error);
                g_object_unref(gDBusConn);
                return false;
            }
            LOGI("Notifications enabled for: {} subscribing to properties changed", key);
            g_dbus_connection_signal_subscribe(
                gDBusConn,
                "org.bluez", "org.freedesktop.DBus.Properties", 
                "PropertiesChanged",
                key.c_str(), "org.bluez.GattCharacteristic1",
                G_DBUS_SIGNAL_FLAGS_NONE,
                &BleService::onPropertiesChanged,
                this, nullptr
            );
            g_main_loop_run(loop);  
            g_main_context_pop_thread_default(ctx);
            LOGI("coming out of thread loop");
            return true; 
        });
    } catch (...) {
        g_main_context_unref(ctx);
        return false; // If thread creation fails, clean up and return false
    }

    // Try to publish into the map (move the thread in)
    {
        std::lock_guard<std::mutex> lk(m_mx);
        auto [it, inserted] = m_loops.try_emplace(
            key, LoopWorker{ctx, loop, std::move(th)}
        );
        if (!inserted) {
            // Key already exists → roll back the new worker
            g_main_loop_quit(loop);
            // Unlock before joining? No lock held here besides lk; ok to join now.
            // Wait for loop to stop
            std::thread tmp(std::move(th)); // th is already moved, but keep pattern consistent
            if (tmp.joinable()) tmp.join();
            g_main_loop_unref(loop);
            g_main_context_unref(ctx);
            return false;
        }
    }
    LOGI("loop was started for key: {}", key);
    return true;
}

void BleService::stopLoopFor(const std::string& key) {
    LOGI("Stopping loop for key: {}", key);
    LoopWorker toStop; // take ownership out of the map to stop outside the lock
    {
        std::lock_guard<std::mutex> lk(m_mx);
        auto it = m_loops.find(key);
        if (it == m_loops.end()) return;

        //GMainContext* ctx = it->second.ctx;
        // bool ctx_acquired = g_main_context_acquire(it->second.ctx);
        // if (!ctx_acquired) {
        //     LOGE("Failed to acquire context for key: {}", key);
        //     return;
        // }
        // g_main_context_release(it->second.ctx);
        // Signal loop to quit
        if (it->second.loop) g_main_loop_quit(it->second.loop);

        // Move the worker out so we can join without holding the lock
        toStop = std::move(it->second);
        m_loops.erase(it);
    }
    LOGI("Loop for key: {} is signaled to stop", key);
    // Join the thread
    if (toStop.th.joinable()) toStop.th.join();
    LOGI("Loop for key: {} has stopped", key);
    // Free GLib resources
    destroyWorker(toStop);
}

void BleService::stopAllLoops() {
    // Move everything out to join without holding the lock
    std::unordered_map<std::string, LoopWorker> toStopAll;
    {
        std::lock_guard<std::mutex> lk(m_mx);
        for (auto& [k, w] : m_loops) {
            if (w.loop) g_main_loop_quit(w.loop);
        }
        toStopAll = std::move(m_loops);
        m_loops.clear();
    }

    // Join and free
    for (auto& [k, w] : toStopAll) {
        if (w.th.joinable()) w.th.join();
        destroyWorker(w);
    }
}

bool BleService::hasLoop(const std::string& key) const {
    std::lock_guard<std::mutex> lk(m_mx);
    return m_loops.find(key) != m_loops.end();
}

size_t BleService::loopCount() const {
    std::lock_guard<std::mutex> lk(m_mx);
    return m_loops.size();
}


//----------------------------------
size_t BleService::readCharacteristics(const std::string& charPath, guint8* retData) {
    GError* err = nullptr;
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &err);
    if (!gDBusConn) {
        std::cerr << "DBus connection is null (initialize it once with g_bus_get_sync and keep it alive).\n";
        return {};
    }
    static const char* BLUEZ_BUS       = "org.bluez";
    static const char* GATT_CHAR_IFACE = "org.bluez.GattCharacteristic1";

    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    /* Finish building variant, i.e. the variant does not contain any value. */
    GVariant *v1 = g_variant_builder_end(builder);
    //This has a ReadValue and a WriteValue interface.
    GVariant* ret = g_dbus_connection_call_sync(
        gDBusConn,
        BLUEZ_BUS,
        charPath.c_str(),
        GATT_CHAR_IFACE,
        "ReadValue",
        g_variant_new_tuple(&v1, 1),  // Empty variant.
        G_VARIANT_TYPE("(ay)"),               // expect ay directly
        G_DBUS_CALL_FLAGS_NONE,
        5000,
        nullptr,
        &err
    );
    // ret has type "(ay)"
    GVariant* arr = nullptr;
    g_variant_get(ret, "(@ay)", &arr);  // borrow child as a new GVariant you must unref

    gsize len = 0;
    const guint8* data = static_cast<const guint8*>(
        g_variant_get_fixed_array(arr, &len, sizeof(guint8))
    );

    if (len > 0) {
        memcpy(retData,data,len);
        for (int i = 0; i< len;i++){
            printf("%d,",data[i]);
        }
    }

    g_object_unref(gDBusConn);
    g_variant_unref(arr);
    g_variant_unref(ret);
    return len;
}


std::string BleService::readService(const std::string& mac, const std::string& uuid) {
    LOGI( "✅ Read notify for  {}-{}",mac,uuid);

  
    // TODO: implement the actual read operation
    return macToDevicePath(mac) + "/" + uuid; // Return the path as a string
}


bool BleService::writeService(const std::string& body){
    LOGI( "✅ Write notify for  {}",body);
    // for (const auto& uuid : serviceUUIDs) {
    //     std::cout << "  - " << uuid << "\n";
    // }
    // might store them for later use, e.g., streaming
    return true;
}

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


std::string BleService::macToDevicePath(const std::string& mac) {
    std::string path = "/org/bluez/hci0/dev_" + mac;
    for (char& c : path) if (c == ':') c = '_';
    return path;
}

bool BleService::removeDevice(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(gDBusConn, "org.bluez", 
                                "/org/bluez/hci0", 
                                "org.bluez.Adapter1",
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
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!gDBusConn) {
        LOGW("DBus connection is null.");
        return false;
    }

    GVariant* result = g_dbus_connection_call_sync(gDBusConn, "org.bluez", path.c_str(),
        "org.freedesktop.DBus.Properties", "Get",
        g_variant_new("(ss)", "org.bluez.Device1", "Connected"),
        G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (!result) {
        LOGW("DBus call to check connection failed, device might not be available.");
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
    GVariant* result = g_dbus_connection_call_sync(gDBusConn, "org.bluez", path.c_str(),
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
    g_dbus_connection_call_sync(gDBusConn, "org.bluez", path.c_str(), "org.bluez.Device1",
        "Pair", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error) {
        LOGE("Pair failed: {} ",error->message);
        g_error_free(error);
        return false;
    }
    return true;
}


bool BleService::trustDevice(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(gDBusConn, "org.bluez", path.c_str(), "org.freedesktop.DBus.Properties",
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
    GVariant* reply = g_dbus_connection_call_sync(gDBusConn, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
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


bool BleService::cancelPairing(const std::string& mac) {
    auto path = macToDevicePath(mac);
    GError* error = nullptr;
    g_dbus_connection_call_sync(gDBusConn, "org.bluez", path.c_str(), "org.bluez.Device1",
        "CancelPairing", nullptr, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error) {
        LOGE("CancelPairing failed: {}", error->message);
        g_error_free(error);
        return false;
    }
    return true;
}

//disconnectDevice
bool BleService::disconnectDevice(const std::string& jsonmac) {
    LOGI("Disconnecting device with MAC: {}", jsonmac);

    std::string mac = parseMacFlexible(jsonmac);
    if(!isConnected(mac)){
        LOGW("Device with MAC: {} is not connected.", mac);
        return true; // Already disconnected
    }
    std::string path = macToDevicePath(mac);

    GError* error = nullptr;
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!gDBusConn) return false;

    g_dbus_connection_call_sync(
        gDBusConn, "org.bluez",
        path.c_str(),
        "org.bluez.Device1",
        "Disconnect",
        nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1, nullptr, &error
    );

    if (error) {
        std::cerr << "❌ Disconnect failed: " << error->message << "\n";
        g_error_free(error);
        g_object_unref(gDBusConn);
        return false;
    }

    g_object_unref(gDBusConn);
    return true;
}

bool BleService::cleanupDisconnectedDevices() {
    for (const auto& path : getConnectedDevices()) {
        // No-op: implement logic here for removing based on timestamp if stored
        LOGI("✔️ Device connected: {}",path);
    }
    return true;
    // Extend with timestamp check and call removeDevice(...) if needed
}

bool BleService::shutdown() {
    //Turning off the Bluetooth adapter
    GError* error = nullptr;
    gDBusConn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!gDBusConn) {
        LOGE("❌ Failed to connect to system bus: {}", error->message);
        g_error_free(error);
        return false;
    }
    g_dbus_connection_call_sync(
        gDBusConn, "org.bluez", adapterPath.c_str(), "org.bluez.Adapter1",
        "SetPowered", g_variant_new("(b)", FALSE), nullptr,
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error
    );
    if (error) {
        LOGE("❌ Failed to turn off Bluetooth adapter: {}", error->message);
        g_error_free(error);
        g_object_unref(gDBusConn);
        return false;
    }
    LOGI("Bluetooth adapter turned off successfully.");
    g_object_unref(gDBusConn);
    gDBusConn = nullptr;
    LOGI("BLE Service stopped and cleaned up.");
    return true;
}



// BleService::BleService() {
//     GError* error = nullptr;
//     conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
//     if (!conn) {
//         std::cerr << "❌ Could not connect to system bus: " << error->message << std::endl;
//         g_error_free(error);
//     }
// }


//----------------WebSocket related methods-------------------
void BleService::addGraphSocket(v_int32 graphId, 
    const std::shared_ptr<oatpp::websocket::AsyncWebSocket>& socket,
    const std::shared_ptr<WSComm>& wsComm) {
    std::lock_guard<std::mutex> lock(m_graphMutex);
    m_graphClients[graphId] = socket;
    m_graphById[graphId] = wsComm;

    if (!m_graphRunning) {
        m_graphRunning = true;
        m_graphThread = std::thread([this]() { 
            LOGI("Starting graph streaming thread.");
            streamGraph(); 
            LOGI("Graph streaming thread finished.");
        });
    }
}

void BleService::leaveGraph(v_int32 userId) {
    {
        std::lock_guard<std::mutex> lock(m_graphMutex);
        m_graphClients.erase(userId);
        m_graphById.erase(userId);
        LOGI("Graph Weboscket leaving");
        if (m_graphClients.empty()) {
            LOGI("m graph running is stopping and notifying");
            m_graphRunning = false;
            m_cv.notify_all(); //  wake the thread immediately
        }
    }

    if (m_graphThread.joinable()) {
        m_graphThread.join();
    }
}

void BleService::streamGraph() {
    using namespace std::chrono;
    std::unique_lock<std::mutex> lock(m_graphMutex);
    float value = 23.5f;
    int counter = 0;
    int counter2 = 0;
    int counter3 = 0;
    //auto bleService = std::dynamic_pointer_cast<BleService>(USE_SRVC("ble"));
    while (m_graphRunning) {
        if (simulationEnable.load()){
            // Generate simulated sensor data using BleSimulation
            std::string jsonData = bleSimulation.generateSimulationData(counter, counter2, counter3, value);
            auto newJson = nlohmann::json::parse(jsonData);

            // Broadcast simulated payload to connected graph clients
            for (auto& pair : m_graphById) {
                pair.second->sendMessage(newJson.dump().c_str());
            }

            // throttle simulation FPS
            int fps = m_webSocketMsgFps;
            if (m_webSocketMsgFps.load() > 0) fps = m_webSocketMsgFps.load();
            if (fps < 1) fps = 1;
            if (fps > 100) fps = 100; // sane upper bound
            auto sleepDuration = std::chrono::milliseconds(1000 / fps);
            // release the main graph lock while waiting so other threads (add/leave) aren't blocked
            lock.unlock();
            // wait for either sleepDuration or until streaming stops (allow early wakeup via notify)
            std::mutex sleeperMutex;
            std::unique_lock<std::mutex> sleeperLock(sleeperMutex);
            m_cv.wait_for(sleeperLock, sleepDuration, [this]{ return !m_graphRunning.load(); });
            // reacquire the main lock and continue
            lock.lock();

            continue;
        }

        while(!m_graphValueChanged.load() && m_graphRunning) {
            m_cv.wait_for(lock, std::chrono::milliseconds(1), 
                [this]{
                    return !m_graphRunning;
                }       
            );
        }

        auto newJson = nlohmann::json::object();
        
        for(auto& pair : m_graphById) {
            LOGI("sendMessage() - Sending message to userId={}", pair.first);
            LOGI("charReg.size() - {}", charReg.size());
            for(int i = 0; i<charReg.size();i++){
                for(auto& graphData : m_graphData) {
                    //TODO: make it faster, by removing the non used keys in
                    //m_graphData
                    if (!graphData.second.updated) {
                        continue; // Skip if value hasn't updated
                    }
                    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                    newJson["time"] = std::to_string(now);
                    newJson["id"] = graphData.first; 
                    newJson["value"] = graphData.second.value;
                    graphData.second.updated = false; // Reset change flag 
                    pair.second->sendMessage(newJson.dump().c_str());//   json.c_str());
                    LOGI("streamGraph() - Sent message: {}", newJson.dump());
                }
            }
        }
        m_graphValueChanged.store(false);
    
        m_cv.wait_for(lock, std::chrono::milliseconds(1), [this] {
            return !m_graphRunning;
        });
    }
    LOGI("streamGraph() - Stopping graph streaming thread.");
    m_graphRunning = false;
}

bool BleService::isEmpty() const {
    std::lock_guard<std::mutex> lockGraphs(m_graphMutex);
    return m_graphClients.empty();
}

bool BleService::webSocketMsgSetFps(std::string body){
    auto json_data = nlohmann::json::parse(body);
    if (!json_data.contains("wsfps")) {
        LOGE("Invalid request: 'wsfps' key is missing.");
        return false;
    }
    if (!json_data["wsfps"].is_number()) {
        LOGE("Invalid request: 'simulation' must be a number.");
        return false;
    }
    m_webSocketMsgFps.store(json_data["wsfps"]);
    LOGI("Setting WebSocket message FPS to {}", m_webSocketMsgFps.load());
    return true;
}

bool BleService::setSimulation(std::string body){
    auto json_data = nlohmann::json::parse(body);
    if (!json_data.contains("simulation")) {
        LOGE("Invalid request: 'simulation' key is missing.");
        return false;
    }
    if (!json_data["simulation"].is_boolean()) {
        LOGE("Invalid request: 'simulation' must be a boolean.");
        return false;
    }
    bool simulationStatus = json_data["simulation"];
    LOGI("Setting simulation mode to {}", simulationStatus);
    
    if (simulationStatus){
        simulationEnable.store(true);
        bleSimulation.enableSimulation();
    }
    else{
        simulationEnable.store(false);
        bleSimulation.disableSimulation();
    }
    return true;
}