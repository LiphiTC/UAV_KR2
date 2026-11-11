#include "signalgraph.h"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

SignalGraph::SignalGraph(const QString& title, const QColor& color, QWidget* parent)
    : QWidget(parent), title_(title), color_(color)
{
    setMinimumSize(290, 200);
    setAccessibleName(title);
}
void SignalGraph::add_sample(double time, double value)
{
    if (!std::isfinite(time) || !std::isfinite(value) || time < 0
        || (!samples_.isEmpty() && time < samples_.last().x()))
        return;
    samples_.append({time, value});
    if (samples_.size() > 600)
        samples_.removeFirst();
    update();
}
void SignalGraph::clear() { samples_.clear(); update(); }
void SignalGraph::set_target(double value) { target_ = value; update(); }
const QVector<QPointF>& SignalGraph::history() const { return samples_; }

void SignalGraph::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().base());
    const QColor text = palette().text().color();
    painter.setPen(text);
    painter.drawText(QRect(0, 4, width(), 24), Qt::AlignCenter, title_);
    const QRectF plot(65, 36, width() - 85, height() - 82);
    const double start = samples_.isEmpty() ? 0 : samples_.first().x();
    const double end = std::max(start + 1, samples_.isEmpty() ? 0 : samples_.last().x());
    double low = 0, high = 1;
    for (const QPointF& sample : samples_) {
        low = std::min(low, sample.y());
        high = std::max(high, sample.y());
    }
    if (std::isfinite(target_)) {
        low = std::min(low, target_);
        high = std::max(high, target_);
    }
    const double margin = (high - low) * 0.1;
    high += margin;
    if (low < 0) low -= margin;
    auto map = [&](QPointF p) {
        return QPointF(plot.left() + (p.x() - start) / (end - start) * plot.width(),
                       plot.bottom() - (p.y() - low) / (high - low) * plot.height());
    };
    for (int i = 0; i <= 4; ++i) {
        const double value = low + (high - low) * i / 4;
        const double y = map({start, value}).y();
        painter.setPen(palette().mid().color());
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        painter.setPen(text);
        painter.drawText(QRectF(0, y - 10, 59, 20), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(value, 'g', 4));
        const double t = start + (end - start) * i / 4;
        painter.drawText(QRectF(map({t, low}).x() - 25, plot.bottom() + 3, 50, 20),
                         Qt::AlignCenter, QString::number(t, 'f', 1));
    }
    painter.drawText(QRect(0, height() - 24, width(), 20), Qt::AlignCenter, "Время модели, с");
    if (std::isfinite(target_)) {
        painter.setPen(QPen(QColor("#d07618"), 2, Qt::DashLine));
        painter.drawLine(map({start, target_}), map({end, target_}));
    }
    if (samples_.isEmpty()) {
        painter.setPen(text);
        painter.drawText(plot, Qt::AlignCenter, "Нет измерений");
        return;
    }
    painter.setClipRect(plot.adjusted(-3, -3, 3, 3));
    QPainterPath path(map(samples_.first()));
    for (int i = 1; i < samples_.size(); ++i)
        path.lineTo(map(samples_[i]));
    painter.setPen(QPen(color_, 2));
    painter.drawPath(path);
    painter.setBrush(color_);
    painter.drawEllipse(map(samples_.last()), 3, 3);
}
