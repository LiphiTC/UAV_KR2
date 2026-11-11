#pragma once

#include "point3d.h"
#include <QString>
#include <QVector>

class UAV {
public:
    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;
    // Учебная траектория для воспроизведения испытания в 3D.
    virtual QVector<Point3D> test_route() const = 0;
    virtual QString image_resource() const = 0;
    virtual ~UAV() = default;
};

class Quadcopter final : public UAV {
public:
    void takeoff() override;
    void land() override;
    QString type() const override;
    QVector<Point3D> test_route() const override;
    QString image_resource() const override;
};

class FixedWingUAV final : public UAV {
public:
    void takeoff() override;
    void land() override;
    QString type() const override;
    QVector<Point3D> test_route() const override;
    QString image_resource() const override;
};

void run_flight_test(UAV& vehicle);
