#include "sensor.h"
#include <QDebug>
#include <cmath>
#include <limits>

namespace {
double read_value(bool ready, double value, const QString& name)
{
    if (ready)
        return value;
    qDebug().noquote() << "Error:" << name << "is not ready";
    return std::numeric_limits<double>::quiet_NaN();
}
}

double Gyroscope::read() { return read_value(ready_, 1.25, name()); }
bool Gyroscope::is_ready() const { return ready_; }
QString Gyroscope::name() const { return "Gyroscope"; }
void Gyroscope::set_ready(bool ready) { ready_ = ready; }

double Accelerometer::read() { return read_value(ready_, 9.81, name()); }
bool Accelerometer::is_ready() const { return ready_; }
QString Accelerometer::name() const { return "Accelerometer"; }
void Accelerometer::set_ready(bool ready) { ready_ = ready; }

double Barometer::read() { return read_value(ready_, 1012.8, name()); }
bool Barometer::is_ready() const { return ready_; }
QString Barometer::name() const { return "Barometer"; }
void Barometer::set_ready(bool ready) { ready_ = ready; }

bool check_sensors(const std::vector<Sensor*>& sensors)
{
    qDebug().noquote() << "Checking sensors...";
    bool ready = !sensors.empty();
    if (sensors.empty())
        qDebug().noquote() << "Error: sensors are not set";
    for (Sensor* sensor : sensors) {
        if (!sensor) {
            qDebug().noquote() << "Error: sensor is not set";
            ready = false;
        } else if (!sensor->is_ready()) {
            qDebug().noquote() << sensor->name() + ": Not ready";
            ready = false;
        } else {
            const double value = sensor->read();
            qDebug().noquote() << QString("%1: Ready, value=%2").arg(sensor->name()).arg(value);
            if (!std::isfinite(value)) {
                qDebug().noquote() << "Error: invalid sensor reading";
                ready = false;
            }
        }
    }
    qDebug().noquote() << (ready ? "All sensors are ready" : "sensor failure");
    return ready;
}
