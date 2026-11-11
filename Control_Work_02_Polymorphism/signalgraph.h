#pragma once

#include <QPointF>
#include <QVector>
#include <QWidget>
#include <limits>

// QPainter, как BatteryGraph из Practical_02_Task_2; шкала зависит от данных.
class SignalGraph : public QWidget {
public:
    SignalGraph(const QString& title, const QColor& color, QWidget* parent = nullptr);
    void add_sample(double time, double value);
    void clear();
    void set_target(double value);
    const QVector<QPointF>& history() const;
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString title_;
    QColor color_;
    QVector<QPointF> samples_;
    double target_ = std::numeric_limits<double>::quiet_NaN();
};
