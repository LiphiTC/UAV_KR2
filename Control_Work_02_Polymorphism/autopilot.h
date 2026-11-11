#pragma once

#include "controller.h"
#include "navigation.h"
#include "sensor.h"
#include "uav.h"

struct CycleResult {
    bool success = false;
    double altitude = 0;
    double error = 0;
    double control = 0;
    QString message;
};

class Autopilot {
public:
    // Не владеет модулями: они должны жить дольше Autopilot.
    Autopilot(UAV* vehicle, const std::vector<Sensor*>& sensors,
              NavigationSystem* navigation, FlightController* controller);
    void run_cycle();
    const CycleResult& last_result() const;
    static constexpr double target_altitude = 200.0;
    static constexpr double time_step = 0.1;
private:
    void fail(const QString& reason);
    UAV* vehicle_;
    std::vector<Sensor*> sensors_;
    NavigationSystem* navigation_;
    FlightController* controller_;
    CycleResult result_;
};
