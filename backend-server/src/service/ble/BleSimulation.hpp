// File:src/service/ble/BleSimulation.hpp
#ifndef BLE_SIMULATION_HPP
#define BLE_SIMULATION_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <chrono>
#include <nlohmann/json.hpp>

/// Simulation utility class for generating BLE-related sensor data.
/// Provides methods to enable/disable simulation mode and generate realistic
/// sensor readings with statistical information.
class BleSimulation {
public:
    BleSimulation();
    explicit BleSimulation(bool prettyPrint);

    // Simulation state management
    void enableSimulation();
    void disableSimulation();
    bool isSimulationEnabled() const;

    // Generate simulated sensor data with counters and timestamps
    // Returns a JSON string with the simulated "websocket_data" object
    // containing timestamp and sensor measurements (ecg, heart_rate, respiration_rate, etc.)
    std::string generateSimulationData(
        int &counter,
        int &counter2,
        int &counter3,
        float baseValue = 23.5f);

    // Helper to reset internal counters and timers for simulation
    void resetSimulationState();

private:
    bool pretty_;
    bool simulationEnabled_ = false;
    
    // Simulation state tracking
    std::chrono::system_clock::time_point lastBpTempSendTime_;
    
    // Helper to create individual sensor readings with stats
    nlohmann::json makeSensorReading(float value) const;
};


#endif //!BLE_SIMULATION_HPP