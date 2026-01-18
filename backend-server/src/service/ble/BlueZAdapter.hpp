#ifndef BLUEZ_ADAPTER_HPP
#define BLUEZ_ADAPTER_HPP

#include <string>
#include <gio/gio.h>

/// Manages BlueZ adapter operations via D-Bus.
/// Handles adapter initialization, discovery, and basic adapter management.
class BlueZAdapter {
public:
    BlueZAdapter();
    ~BlueZAdapter();

    // Initialize BlueZ connection and find adapter
    bool initialize();

    // Cleanup and disconnect
    void cleanup();

    // Get the adapter path
    std::string getAdapterPath() const { return adapterPath_; }

    // Get the D-Bus connection
    GDBusConnection* getConnection() const { return gDBusConn_; }

    // Start device discovery
    bool startDiscovery();

    // Stop device discovery
    bool stopDiscovery();

private:
    GDBusConnection* gDBusConn_ = nullptr;
    std::string adapterPath_;

    // Find the first available BlueZ adapter
    std::string findAdapter();
};

#endif // BLUEZ_ADAPTER_HPP
