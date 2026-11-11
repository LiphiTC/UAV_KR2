#pragma once

#include <QString>

struct Position {
    double latitude;
    double longitude;
    double altitude;
};

bool valid_position(const Position& position);

class NavigationSystem {
public:
    virtual Position get_position() const = 0;
    virtual bool is_valid() const = 0;
    virtual QString system_name() const = 0;
    virtual ~NavigationSystem() = default;
};

class GPSNavigation final : public NavigationSystem {
public:
    explicit GPSNavigation(Position position = {55.75, 37.61, 150},
                           int satellites = 8, bool valid = true);
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;
    void set_position(Position position);
    void set_satellites(int satellites);
    int satellites() const;
    void set_valid(bool valid);
private:
    Position position_;
    int satellites_;
    bool valid_;
};

class VisualNavigation final : public NavigationSystem {
public:
    explicit VisualNavigation(Position position = {55.75, 37.61, 150},
                              bool target_tracked = true);
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;
    void set_position(Position position);
    void set_target_tracked(bool tracked);
private:
    Position position_;
    bool target_tracked_;
};

void print_navigation_status(const NavigationSystem& navigation);
