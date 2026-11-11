#include "uav.h"
#include <QDebug>

void Quadcopter::takeoff() { qDebug().noquote() << "Quadcopter: vertical takeoff"; }
void Quadcopter::land() { qDebug().noquote() << "Quadcopter: vertical landing"; }
QString Quadcopter::type() const { return "Quadcopter"; }
QString Quadcopter::image_resource() const { return ":/assets/drone.jpg"; }
QVector<Point3D> Quadcopter::test_route() const
{
    return {{100, 200, 0}, {100, 200, 120}, {100, 200, 120}, {100, 200, 0}};
}

void FixedWingUAV::takeoff() { qDebug().noquote() << "Fixed-wing UAV: runway takeoff"; }
void FixedWingUAV::land() { qDebug().noquote() << "Fixed-wing UAV: gliding landing"; }
QString FixedWingUAV::type() const { return "Fixed-wing UAV"; }
QString FixedWingUAV::image_resource() const { return ":/assets/fixedwing.svg"; }
QVector<Point3D> FixedWingUAV::test_route() const
{
    return {{30, 200, 0}, {130, 200, 0}, {270, 200, 120}, {420, 200, 120}, {570, 200, 0}};
}

void run_flight_test(UAV& vehicle)
{
    qDebug().noquote() << "Flight test:" << vehicle.type();
    vehicle.takeoff();
    vehicle.land();
    qDebug().noquote() << "Flight test completed";
}
