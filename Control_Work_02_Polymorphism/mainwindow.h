#pragma once

#include "autopilot.h"
#include <QElapsedTimer>
#include <QMainWindow>
#include <array>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTimer;
class Scene3D;
class SignalGraph;
class FlightAudio;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    // Детерминированное продвижение времени для проверки анимации.
    void advance_simulation(double seconds);
private:
    enum class Mode { Idle, FlightTest, Autopilot };
    UAV& selected_vehicle();
    NavigationSystem& selected_navigation();
    void sync_models();
    void refresh_status();
    void record_sensors();
    void check_sensor_panel();
    void check_navigation_panel();
    void start_flight_test();
    void start_autopilot();
    bool run_cycle();
    void stop(const QString& message);
    void reset();
    void prepare_scene(const Point3D& position);
    void update_controls();
    void log(const QString& message, bool error = false);
    void clear_graphs();

    Quadcopter quadcopter_;
    FixedWingUAV fixedwing_;
    Gyroscope gyroscope_;
    Accelerometer accelerometer_;
    Barometer barometer_;
    std::vector<Sensor*> sensors_{&gyroscope_, &accelerometer_, &barometer_};
    GPSNavigation gps_;
    VisualNavigation visual_;
    SimpleController controller_;
    Mode mode_ = Mode::Idle;
    double time_ = 0;
    double accumulator_ = 0;
    QVector<Point3D> flight_route_;
    QString scene_vehicle_;

    QComboBox* vehicle_input_;
    QComboBox* navigation_input_;
    std::array<QCheckBox*, 3> ready_inputs_;
    QCheckBox* gps_valid_;
    QCheckBox* target_tracked_;
    QCheckBox* controller_connected_;
    QCheckBox* sound_enabled_;
    QSpinBox* satellites_;
    QDoubleSpinBox* latitude_;
    QDoubleSpinBox* longitude_;
    QDoubleSpinBox* altitude_;
    QLabel* image_;
    QLabel* navigation_status_;
    QLabel* result_;
    QTableWidget* sensor_table_;
    QPlainTextEdit* log_;
    QPushButton* test_button_;
    QPushButton* cycle_button_;
    QPushButton* auto_button_;
    QPushButton* stop_button_;
    Scene3D* scene_;
    SignalGraph* altitude_graph_;
    SignalGraph* control_graph_;
    std::array<SignalGraph*, 3> sensor_graphs_;
    QTimer* timer_;
    QElapsedTimer clock_;
    FlightAudio* audio_;
};
