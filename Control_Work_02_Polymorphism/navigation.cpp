#include "navigation.h"
#include <QDebug>
#include <cmath>

bool valid_position(const Position& p)
{
    return std::isfinite(p.latitude) && std::isfinite(p.longitude) && std::isfinite(p.altitude)
        && p.latitude >= -90 && p.latitude <= 90
        && p.longitude >= -180 && p.longitude <= 180 && p.altitude >= 0;
}

GPSNavigation::GPSNavigation(Position position, int satellites, bool valid)
    : position_(position), satellites_(satellites), valid_(valid) {}
Position GPSNavigation::get_position() const { return position_; }
bool GPSNavigation::is_valid() const { return satellites_ >= 4 && valid_; }
QString GPSNavigation::system_name() const { return "GPS"; }
void GPSNavigation::set_position(Position position) { position_ = position; }
void GPSNavigation::set_satellites(int satellites) { satellites_ = satellites; }
int GPSNavigation::satellites() const { return satellites_; }
void GPSNavigation::set_valid(bool valid) { valid_ = valid; }

VisualNavigation::VisualNavigation(Position position, bool target_tracked)
    : position_(position), target_tracked_(target_tracked) {}
Position VisualNavigation::get_position() const { return position_; }
bool VisualNavigation::is_valid() const { return target_tracked_; }
QString VisualNavigation::system_name() const { return "Visual navigation"; }
void VisualNavigation::set_position(Position position) { position_ = position; }
void VisualNavigation::set_target_tracked(bool tracked) { target_tracked_ = tracked; }

void print_navigation_status(const NavigationSystem& navigation)
{
    const Position p = navigation.get_position();
    qDebug().noquote() << "Navigation system:" << navigation.system_name();
    qDebug().noquote() << QString("Position: %1, %2, %3").arg(p.latitude).arg(p.longitude).arg(p.altitude);
    qDebug().noquote() << "Status:" << (navigation.is_valid() ? "Valid" : "Invalid");
    if (!valid_position(p))
        qDebug().noquote() << "Error: invalid navigation position";
}
