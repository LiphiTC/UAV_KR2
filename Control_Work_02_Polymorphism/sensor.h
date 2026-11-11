#pragma once

#include <QString>
#include <vector>

class Sensor {
public:
    virtual double read() = 0;
    virtual bool is_ready() const = 0;
    virtual QString name() const = 0;
    virtual ~Sensor() = default;
};

class Gyroscope final : public Sensor {
public:
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    void set_ready(bool ready);
private:
    bool ready_ = true;
};

class Accelerometer final : public Sensor {
public:
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    void set_ready(bool ready);
private:
    bool ready_ = true;
};

class Barometer final : public Sensor {
public:
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    void set_ready(bool ready);
private:
    bool ready_ = true;
};

bool check_sensors(const std::vector<Sensor*>& sensors);
