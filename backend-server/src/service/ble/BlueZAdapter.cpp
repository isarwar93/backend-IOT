// File: src/service/ble/BlueZAdapter.cpp
#include "BlueZAdapter.hpp"
#include "config/LogAdapt.hpp"

BlueZAdapter::BlueZAdapter()
    : gDBusConn_(nullptr) {}

BlueZAdapter::~BlueZAdapter() {
    cleanup();
}

bool BlueZAdapter::initialize() {
    adapterPath_ = findAdapter();
    return !adapterPath_.empty();
}

void BlueZAdapter::cleanup() {
    if (gDBusConn_) {
        g_object_unref(gDBusConn_);
        gDBusConn_ = nullptr;
    }
}

std::string BlueZAdapter::findAdapter() {
    // Connect to the system bus
    gDBusConn_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr);
    if (!gDBusConn_) {
        LOGE("Failed to connect to system bus");
        return "";
    }

    GVariant* result = g_dbus_connection_call_sync(
        gDBusConn_,
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
        LOGE("GetManagedObjects failed");
        g_object_unref(gDBusConn_);
        gDBusConn_ = nullptr;
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
                LOGI("Found BlueZ adapter at: {}", path);
                // Do NOT free iter, interfaces, or properties
                g_variant_unref(result);
                return path;
            }
        }
    }
    
    g_variant_unref(result);
    g_object_unref(gDBusConn_);
    gDBusConn_ = nullptr;
    LOGW("No BlueZ adapter found");
    return "";
}

bool BlueZAdapter::startDiscovery() {
    if (adapterPath_.empty() || !gDBusConn_) {
        LOGE("Adapter not initialized or D-Bus connection not available");
        return false;
    }

    GError* error = nullptr;
    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", adapterPath_.c_str(), "org.bluez.Adapter1",
        "StartDiscovery", nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (error) {
        LOGE("StartDiscovery failed: {}", error->message);
        g_error_free(error);
        return false;
    }
    
    LOGI("Device discovery started");
    return true;
}

bool BlueZAdapter::stopDiscovery() {
    if (adapterPath_.empty() || !gDBusConn_) {
        LOGE("Adapter not initialized or D-Bus connection not available");
        return false;
    }

    GError* error = nullptr;
    g_dbus_connection_call_sync(
        gDBusConn_, "org.bluez", adapterPath_.c_str(), "org.bluez.Adapter1",
        "StopDiscovery", nullptr, nullptr,
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (error) {
        LOGE("StopDiscovery failed: {}", error->message);
        g_error_free(error);
        return false;
    }

    LOGI("Device discovery stopped");
    return true;
}
