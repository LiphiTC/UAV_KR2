#include "autopilot.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    qDebug().noquote() << "=== Task 1 / Variant 1 ===";
    Quadcopter quadcopter;
    FixedWingUAV fixedwing;
    for (UAV* vehicle : std::vector<UAV*>{&quadcopter, &fixedwing})
        run_flight_test(*vehicle);

    qDebug().noquote() << "=== Task 2 / Variant 1 ===";
    Gyroscope gyroscope;
    Accelerometer accelerometer;
    Barometer barometer;
    const std::vector<Sensor*> sensors{&gyroscope, &accelerometer, &barometer};
    qDebug().noquote() << "Gyroscope:" << gyroscope.read() << "rad/s";
    qDebug().noquote() << "Accelerometer:" << accelerometer.read() << "m/s2";
    qDebug().noquote() << "Barometer:" << barometer.read() << "hPa";
    check_sensors(sensors);
    accelerometer.set_ready(false);
    check_sensors(sensors);
    accelerometer.set_ready(true);

    qDebug().noquote() << "=== Task 3 / Variant 1 ===";
    GPSNavigation gps;
    VisualNavigation visual;
    print_navigation_status(gps);
    gps.set_satellites(3);
    print_navigation_status(gps);
    gps.set_satellites(8);
    gps.set_valid(false);
    print_navigation_status(gps);
    gps.set_valid(true);
    print_navigation_status(visual);
    visual.set_target_tracked(false);
    print_navigation_status(visual);
    visual.set_target_tracked(true);

    qDebug().noquote() << "=== Mandatory integration / five scenarios ===";
    SimpleController controller;
    Autopilot autopilot(&quadcopter, sensors, &gps, &controller);
    qDebug().noquote() << "1. Normal operation";
    autopilot.run_cycle();
    qDebug().noquote() << "2. Sensor not ready";
    gyroscope.set_ready(false);
    autopilot.run_cycle();
    gyroscope.set_ready(true);
    qDebug().noquote() << "3. Navigation invalid";
    gps.set_valid(false);
    autopilot.run_cycle();
    gps.set_valid(true);
    qDebug().noquote() << "4. Missing flight controller";
    Autopilot(&quadcopter, sensors, &gps, nullptr).run_cycle();
    qDebug().noquote() << "5. Negative altitude";
    gps.set_position({55.75, 37.61, -1});
    autopilot.run_cycle();
    return 0;
}
