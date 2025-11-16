// File:src/service/ble/BleSimulation.hpp
#ifndef BLE_SIMULATION_HPP
#define BLE_SIMULATION_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>

/// Lightweight utility class for creating BLE-related simulation JSON messages.
///
/// Implementation is expected in BleSimulation.cpp. This header only declares the
/// interface used to produce JSON strings representing advertisements, sensor
/// readings, device state, and scan reports. JSON is produced as std::string so
/// the project remains independent of any particular JSON library.
class BleSimulation {
public:
    struct AdvEntry {
        std::string deviceId;
        std::string name;
        int rssi = 0;
        std::map<std::string, std::string> advData; // key/value pairs for advertising payload
    };

    BleSimulation();
    explicit BleSimulation(bool prettyPrint);

    // Control pretty printing (indentation/newlines) in produced JSON
    void setPretty(bool enabled);
    bool isPretty() const;

    // Build a JSON string for a single BLE advertisement
    // Example structure:
    // {
    //   "type":"advertisement",
    //   "deviceId":"aa:bb:cc:dd:ee:ff",
    //   "name":"Sensor",
    //   "rssi":-45,
    //   "adv": { "manufactuer": "...", "foo":"bar" },
    //   "timestamp": 1630000000000
    // }
    std::string buildAdvertisementJson(
        const std::string &deviceId,
        const std::string &name,
        int rssi,
        const std::map<std::string, std::string> &advData,
        uint64_t timestampMs = 0) const;

    // Build a JSON string representing device state (connected/disconnected etc.)
    // {
    //   "type":"device_state",
    //   "deviceId":"aa:bb:cc",
    //   "connected":true,
    //   "battery":95,
    //   "timestamp": 1630000000000
    // }
    std::string buildDeviceStateJson(
        const std::string &deviceId,
        bool connected,
        int batteryPercent = -1,
        uint64_t timestampMs = 0) const;

    // Build a JSON string for a sensor reading produced by a device
    // {
    //   "type":"sensor_reading",
    //   "deviceId":"aa:bb",
    //   "sensor":"temperature",
    //   "value":23.5,
    //   "unit":"C",
    //   "timestamp": 1630000000000
    // }
    std::string buildSensorReadingJson(
        const std::string &deviceId,
        const std::string &sensorType,
        double value,
        const std::string &unit = "",
        uint64_t timestampMs = 0) const;

    // Build a JSON string representing the result of a BLE scan (array of advertisements)
    // {
    //   "type":"scan_report",
    //   "timestamp":1630000000000,
    //   "results":[ { advertisement }, { advertisement }, ... ]
    // }
    std::string buildScanReportJson(
        const std::vector<AdvEntry> &entries,
        uint64_t timestampMs = 0) const;

    // Escape JSON special characters in a string value (static helper).
    // Implementation provided in .cpp.
    static std::string escapeJsonString(const std::string &input);

private:
    bool pretty_;
    std::string indentStr() const;
};


#endif //!BLE_SIMULATION_HPP