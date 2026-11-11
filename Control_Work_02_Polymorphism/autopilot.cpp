#include "autopilot.h"
#include <QDebug>
#include <cmath>

Autopilot::Autopilot(UAV* vehicle, const std::vector<Sensor*>& sensors,
                     NavigationSystem* navigation, FlightController* controller)
    : vehicle_(vehicle), sensors_(sensors), navigation_(navigation), controller_(controller) {}

const CycleResult& Autopilot::last_result() const { return result_; }

void Autopilot::fail(const QString& reason)
{
    result_.message = reason;
    qDebug().noquote() << reason;
    qDebug().noquote() << "Autopilot cycle aborted";
}

void Autopilot::run_cycle()
{
    result_ = {};
    qDebug().noquote() << "Autopilot cycle started";
    if (!vehicle_) { fail("Error: vehicle is not set"); return; }
    if (!check_sensors(sensors_)) { fail("Reason: sensor failure"); return; }
    if (!navigation_) { fail("Error: navigation is not set"); return; }
    if (!navigation_->is_valid()) { fail("Navigation data invalid"); return; }
    const Position position = navigation_->get_position();
    if (!valid_position(position)) { fail("Error: invalid navigation position"); return; }
    if (!controller_) { fail("Error: flight controller is not set"); return; }
    result_.altitude = position.altitude;
    result_.error = target_altitude - position.altitude;
    result_.control = controller_->calculate_control(result_.error, time_step);
    if (!std::isfinite(result_.control)) { fail("Error: invalid controller output"); return; }
    result_.success = true;
    result_.message = "Autopilot cycle completed";
    qDebug().noquote() << "Navigation:" << navigation_->system_name();
    qDebug().noquote() << "Current altitude:" << result_.altitude << "m";
    qDebug().noquote() << "Target altitude:" << target_altitude << "m";
    qDebug().noquote() << "Altitude error:" << result_.error << "m";
    qDebug().noquote() << "Controller output:" << result_.control;
    qDebug().noquote() << result_.message;
}
