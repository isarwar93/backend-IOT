// File: src/service/ble/BleSimulation.cpp
#include "BleSimulation.hpp"

#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

BleSimulation::BleSimulation()
    : pretty_(false) {}

BleSimulation::BleSimulation(bool prettyPrint)
    : pretty_(prettyPrint) {}

void BleSimulation::setPretty(bool enabled) {
    pretty_ = enabled;
}

bool BleSimulation::isPretty() const {
    return pretty_;
}

std::string BleSimulation::indentStr() const {
    return pretty_ ? std::string("  ") : std::string();
}

static uint64_t nowMillis() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

static std::string formatDouble(double v) {
    std::ostringstream oss;
    // reasonable precision; trim trailing zeros
    oss << std::fixed << std::setprecision(6) << v;
    std::string s = oss.str();
    // trim trailing zeros
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s;
}

std::string BleSimulation::escapeJsonString(const std::string &input) {
    std::string out;
    out.reserve(input.size() + 8);
    for (unsigned char c : input) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    // control character -> \u00XX
                    static const char hex[] = "0123456789abcdef";
                    out += "\\u00";
                    out += hex[(c >> 4) & 0xF];
                    out += hex[c & 0xF];
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

std::string BleSimulation::buildAdvertisementJson(
    const std::string &deviceId,
    const std::string &name,
    int rssi,
    const std::map<std::string, std::string> &advData,
    uint64_t timestampMs) const
{
    uint64_t ts = timestampMs ? timestampMs : nowMillis();
    const std::string indent = indentStr();
    const std::string nl = pretty_ ? "\n" : "";
    const std::string sep = pretty_ ? " " : "";

    std::ostringstream o;
    o << "{" << nl;
    if (pretty_) o << indent;
    o << "\"type\":\"advertisement\",\"deviceId\":\"" << escapeJsonString(deviceId) << "\",";
    if (pretty_) o << nl << indent;
    o << "\"name\":\"" << escapeJsonString(name) << "\",";
    if (pretty_) o << nl << indent;
    o << "\"rssi\":" << rssi << ",";
    if (pretty_) o << nl << indent;
    o << "\"adv\":{";
    if (pretty_ && !advData.empty()) o << nl;
    bool first = true;
    for (const auto &kv : advData) {
        if (!first) {
            o << ",";
            if (pretty_) o << nl;
        }
        first = false;
        if (pretty_) o << indent << indent;
        o << "\"" << escapeJsonString(kv.first) << "\":\"" << escapeJsonString(kv.second) << "\"";
    }
    if (pretty_ && !advData.empty()) {
        o << nl;
        o << indent;
    }
    o << "},";
    if (pretty_) o << nl << indent;
    o << "\"timestamp\":" << ts;
    if (pretty_) o << nl;
    o << "}";
    return o.str();
}

std::string BleSimulation::buildDeviceStateJson(
    const std::string &deviceId,
    bool connected,
    int batteryPercent,
    uint64_t timestampMs) const
{
    uint64_t ts = timestampMs ? timestampMs : nowMillis();
    const std::string indent = indentStr();
    const std::string nl = pretty_ ? "\n" : "";

    std::ostringstream o;
    o << "{" << nl;
    if (pretty_) o << indent;
    o << "\"type\":\"device_state\",\"deviceId\":\"" << escapeJsonString(deviceId) << "\",";
    if (pretty_) o << nl << indent;
    o << "\"connected\":" << (connected ? "true" : "false");
    if (batteryPercent >= 0) {
        o << ",";
        if (pretty_) o << nl << indent;
        o << "\"battery\":" << batteryPercent;
    }
    o << ",";
    if (pretty_) o << nl << indent;
    o << "\"timestamp\":" << ts;
    if (pretty_) o << nl;
    o << "}";
    return o.str();
}

std::string BleSimulation::buildSensorReadingJson(
    const std::string &deviceId,
    const std::string &sensorType,
    double value,
    const std::string &unit,
    uint64_t timestampMs) const
{
    uint64_t ts = timestampMs ? timestampMs : nowMillis();
    const std::string indent = indentStr();
    const std::string nl = pretty_ ? "\n" : "";

    std::ostringstream o;
    o << "{" << nl;
    if (pretty_) o << indent;
    o << "\"type\":\"sensor_reading\",\"deviceId\":\"" << escapeJsonString(deviceId) << "\",";
    if (pretty_) o << nl << indent;
    o << "\"sensor\":\"" << escapeJsonString(sensorType) << "\",";
    if (pretty_) o << nl << indent;
    o << "\"value\":" << formatDouble(value);
    if (!unit.empty()) {
        o << ",";
        if (pretty_) o << nl << indent;
        o << "\"unit\":\"" << escapeJsonString(unit) << "\"";
    }
    o << ",";
    if (pretty_) o << nl << indent;
    o << "\"timestamp\":" << ts;
    if (pretty_) o << nl;
    o << "}";
    return o.str();
}

std::string BleSimulation::buildScanReportJson(
    const std::vector<AdvEntry> &entries,
    uint64_t timestampMs) const
{
    uint64_t ts = timestampMs ? timestampMs : nowMillis();
    const std::string indent = indentStr();
    const std::string nl = pretty_ ? "\n" : "";

    std::ostringstream o;
    o << "{" << nl;
    if (pretty_) o << indent;
    o << "\"type\":\"scan_report\",";
    if (pretty_) o << nl << indent;
    o << "\"timestamp\":" << ts << ",";
    if (pretty_) o << nl << indent;
    o << "\"results\":[";
    if (pretty_ && !entries.empty()) o << nl;

    bool firstEntry = true;
    for (const auto &e : entries) {
        if (!firstEntry) {
            o << ",";
            if (pretty_) o << nl;
        }
        firstEntry = false;
        if (pretty_) o << indent << indent;
        // Reuse buildAdvertisementJson but we need advertisement object without outer spaces/newlines.
        // Create adv object manually similar to buildAdvertisementJson but without surrounding braces handling.
        std::ostringstream adv;
        adv << "{" ;
        if (pretty_) adv << nl;
        if (pretty_) adv << indent << indent;
        adv << "\"type\":\"advertisement\",\"deviceId\":\"" << escapeJsonString(e.deviceId) << "\",";
        if (pretty_) adv << nl << indent << indent;
        adv << "\"name\":\"" << escapeJsonString(e.name) << "\",";
        if (pretty_) adv << nl << indent << indent;
        adv << "\"rssi\":" << e.rssi << ",";
        if (pretty_) adv << nl << indent << indent;
        adv << "\"adv\":{";
        if (pretty_ && !e.advData.empty()) adv << nl;
        bool first = true;
        for (const auto &kv : e.advData) {
            if (!first) {
                adv << ",";
                if (pretty_) adv << nl;
            }
            first = false;
            if (pretty_) adv << indent << indent << indent;
            adv << "\"" << escapeJsonString(kv.first) << "\":\"" << escapeJsonString(kv.second) << "\"";
        }
        if (pretty_ && !e.advData.empty()) {
            adv << nl;
            adv << indent << indent;
        }
        adv << "},";
        if (pretty_) adv << nl << indent << indent;
        adv << "\"timestamp\":" << ts;
        if (pretty_) adv << nl << indent << indent;
        adv << "}";
        // insert adv.str() but if pretty we already added indent; ensure proper indentation
        std::string advStr = adv.str();
        if (!pretty_) {
            // compact: remove any newlines/spaces introduced
            std::string compact;
            for (char c : advStr) if (c != '\n' && c != '\r') compact += c;
            advStr.swap(compact);
        }
        o << advStr;
    }

    if (pretty_ && !entries.empty()) {
        o << nl << indent;
    }
    o << "]";
    if (pretty_) o << nl;
    o << "}";
    return o.str();
}