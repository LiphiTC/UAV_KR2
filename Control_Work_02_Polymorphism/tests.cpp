#include "autopilot.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

namespace {
QStringList messages;
void capture(QtMsgType, const QMessageLogContext&, const QString& message) { messages << message; }
void check(bool condition, const char* message)
{
    if (!condition) { qFatal("FAILED: %s", message); }
}
class ProbeSensor final : public Sensor {
public:
    double read() override { ++reads_; return value_; }
    bool is_ready() const override { return ready_; }
    QString name() const override { return "Probe"; }
    void set_ready(bool ready) { ready_ = ready; }
    void set_value(double value) { value_ = value; }
    int reads() const { return reads_; }
private:
    bool ready_ = false;
    double value_ = 1;
    int reads_ = 0;
};
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    static_assert(std::is_abstract_v<UAV> && std::has_virtual_destructor_v<UAV>);
    static_assert(std::is_abstract_v<Sensor> && std::has_virtual_destructor_v<Sensor>);
    static_assert(std::is_abstract_v<NavigationSystem> && std::has_virtual_destructor_v<NavigationSystem>);
    static_assert(std::is_abstract_v<FlightController> && std::has_virtual_destructor_v<FlightController>);

    Quadcopter quad;
    FixedWingUAV wing;
    auto previous = qInstallMessageHandler(capture);
    run_flight_test(quad);
    run_flight_test(wing);
    qInstallMessageHandler(previous);
    check(messages.contains("Quadcopter: vertical takeoff"), "quad takeoff output");
    check(messages.contains("Quadcopter: vertical landing"), "quad landing output");
    check(messages.contains("Fixed-wing UAV: runway takeoff"), "wing takeoff output");
    check(messages.contains("Fixed-wing UAV: gliding landing"), "wing landing output");
    check(messages.count("Flight test completed") == 2, "both tests completed");

    Gyroscope gyro;
    Accelerometer accel;
    Barometer baro;
    std::vector<Sensor*> sensors{&gyro, &accel, &baro};
    check(gyro.read() == 1.25 && accel.read() == 9.81 && baro.read() == 1012.8, "reference sensor values");
    check(check_sensors(sensors), "all sensors ready");
    gyro.set_ready(false);
    check(!check_sensors(sensors) && std::isnan(gyro.read()), "unready gyro fails safely");
    gyro.set_ready(true);
    accel.set_ready(false);
    check(!check_sensors(sensors), "unready accelerometer");
    accel.set_ready(true);
    baro.set_ready(false);
    check(!check_sensors(sensors), "unready barometer");
    baro.set_ready(true);
    check(!check_sensors({nullptr}) && !check_sensors({}), "null and empty sensor collections");
    ProbeSensor probe;
    check(!check_sensors({&probe}) && probe.reads() == 0, "never read an unready sensor");
    probe.set_ready(true);
    probe.set_value(std::numeric_limits<double>::quiet_NaN());
    check(!check_sensors({&probe}), "nonfinite sensor reading");

    GPSNavigation gps;
    VisualNavigation visual;
    check(gps.is_valid(), "default GPS valid");
    gps.set_satellites(3);
    check(!gps.is_valid(), "three satellites invalid");
    gps.set_satellites(4);
    check(gps.is_valid(), "four satellites valid");
    gps.set_valid(false);
    check(!gps.is_valid(), "GPS valid flag required");
    gps.set_valid(true);
    check(visual.is_valid(), "tracked target valid");
    visual.set_target_tracked(false);
    check(!visual.is_valid(), "lost target invalid");
    visual.set_target_tracked(true);
    messages.clear();
    previous = qInstallMessageHandler(capture);
    print_navigation_status(gps);
    qInstallMessageHandler(previous);
    check(messages == QStringList{"Navigation system: GPS", "Position: 55.75, 37.61, 150", "Status: Valid"},
          "reference navigation output");

    SimpleController controller;
    Autopilot autopilot(&quad, sensors, &gps, &controller);
    autopilot.run_cycle();
    check(autopilot.last_result().success && autopilot.last_result().altitude == 150
          && autopilot.last_result().error == 50 && autopilot.last_result().control == 25,
          "scenario 1: reference cycle");
    gyro.set_ready(false);
    autopilot.run_cycle();
    check(!autopilot.last_result().success && autopilot.last_result().message == "Reason: sensor failure"
          && autopilot.last_result().control == 0, "scenario 2: failure clears stale control");
    gyro.set_ready(true);
    gps.set_valid(false);
    autopilot.run_cycle();
    check(!autopilot.last_result().success && autopilot.last_result().message == "Navigation data invalid",
          "scenario 3: invalid navigation");
    gps.set_valid(true);
    Autopilot missing_controller(&quad, sensors, &gps, nullptr);
    missing_controller.run_cycle();
    check(missing_controller.last_result().message == "Error: flight controller is not set", "scenario 4");
    gps.set_position({55.75, 37.61, -1});
    autopilot.run_cycle();
    check(autopilot.last_result().message == "Error: invalid navigation position", "scenario 5");
    for (Position invalid : {Position{91, 0, 150}, Position{0, 181, 150},
         Position{0, 0, std::numeric_limits<double>::infinity()},
         Position{std::numeric_limits<double>::quiet_NaN(), 0, 150}}) {
        gps.set_position(invalid);
        autopilot.run_cycle();
        check(!autopilot.last_result().success, "invalid coordinate rejected");
    }
    gps.set_position({55.75, 37.61, 150});
    Autopilot no_vehicle(nullptr, sensors, &gps, &controller);
    no_vehicle.run_cycle();
    check(no_vehicle.last_result().message == "Error: vehicle is not set", "null vehicle");
    Autopilot no_navigation(&quad, sensors, nullptr, &controller);
    no_navigation.run_cycle();
    check(no_navigation.last_result().message == "Error: navigation is not set", "null navigation");
    Autopilot null_sensor(&quad, {nullptr}, &gps, &controller);
    null_sensor.run_cycle();
    check(!null_sensor.last_result().success, "null sensor aborts cycle");
    Autopilot interchangeable(&wing, sensors, &visual, &controller);
    interchangeable.run_cycle();
    check(interchangeable.last_result().success, "wing and visual navigation use same cycle");
    check(std::isnan(controller.calculate_control(50, 0)), "invalid dt");
    qDebug().noquote() << "All model checks passed";
    return EXIT_SUCCESS;
}
