// File: src/service/ble/BleSimulation.cpp
#include "BleSimulation.hpp"

#include <sstream>
#include <chrono>
#include <cmath>

BleSimulation::BleSimulation()
    : pretty_(false) {}

BleSimulation::BleSimulation(bool prettyPrint)
    : pretty_(prettyPrint) {}

/* ========================
   Simulation State Management
   ======================== */

void BleSimulation::enableSimulation() {
    simulationEnabled_ = true;
    resetSimulationState();
}

void BleSimulation::disableSimulation() {
    simulationEnabled_ = false;
}

bool BleSimulation::isSimulationEnabled() const {
    return simulationEnabled_;
}

void BleSimulation::resetSimulationState() {
    using namespace std::chrono;
    lastBpTempSendTime_ = system_clock::now();
}

/* ========================
   Simulation Data Generation
   ======================== */

nlohmann::json BleSimulation::makeSensorReading(float value) const {
    std::vector<float> past = { value, value * 0.98f, value * 1.02f, value - 0.5f, value + 0.5f };
    float sum = 0.f;
    float maxv = past.empty() ? 0.f : past[0];
    float minv = past.empty() ? 0.f : past[0];
    for (float p : past) {
        sum += p;
        if (p > maxv) maxv = p;
        if (p < minv) minv = p;
    }
    nlohmann::json s;
    s["current"] = value;
    s["past_values"] = past;
    s["max"] = maxv;
    s["min"] = minv;
    s["avg"] = past.empty() ? 0.f : (sum / static_cast<float>(past.size()));
    return s;
}

std::string BleSimulation::generateSimulationData(
    int &counter,
    int &counter2,
    int &counter3,
    float baseValue)
{
    using namespace std::chrono;
    
    // Update counters
    counter = (counter + 1) % 1000;
    counter2 = (counter2 + counter) % 9000;
    counter3 = (counter3 + counter2) % 80000;
    
    // Build the websocket data object
    nlohmann::json ws;
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    ws["timestamp"] = std::to_string(now);
    
    // Generate sensor readings using mathematical functions
    nlohmann::json sensors;
    sensors["ecg"] = makeSensorReading(counter + baseValue * 67 + (std::rand() % 100) * 0.1f);
    sensors["heart_rate"] = makeSensorReading(cos((counter + 34) * 0.1f) * 10.0f + 70.0f);
    sensors["respiration_rate"] = makeSensorReading(sin((counter - 2) * 0.1f) * 5.0f + 16.0f);
    
    // Check if 3 seconds have passed for blood_pressure and body_temperature
    auto currentTime = system_clock::now();
    auto elapsedSeconds = duration_cast<seconds>(currentTime - lastBpTempSendTime_).count();
    
    if (elapsedSeconds >= 3) {
        sensors["blood_pressure"] = makeSensorReading(120.0f + (counter + baseValue - 20.0f) * 0.5f);
        sensors["body_temperature"] = makeSensorReading(36.5f + (counter + baseValue - 20.0f) * 0.01f);
        lastBpTempSendTime_ = currentTime;
    }
    
    ws["sensors"] = sensors;
    
    // Build the full JSON response with websocket_data wrapper
    nlohmann::json response;
    response["websocket_data"] = ws;
    return response.dump();
}