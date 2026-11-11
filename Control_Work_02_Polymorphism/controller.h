#pragma once

#include <QString>

// Минимальный контроллер для обязательной интеграции, не задание 3 варианта 3.
class FlightController {
public:
    virtual double calculate_control(double error, double dt) = 0;
    virtual QString controller_name() const = 0;
    virtual ~FlightController() = default;
};

class SimpleController final : public FlightController {
public:
    explicit SimpleController(double coefficient = 0.5);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
private:
    double coefficient_;
};
