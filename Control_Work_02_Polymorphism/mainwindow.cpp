#include "mainwindow.h"
#include "flightaudio.h"
#include "scene3d.h"
#include "signalgraph.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace {
QPushButton* button(const QString& text, const char* name, QBoxLayout* layout)
{
    auto* result = new QPushButton(text);
    result->setObjectName(name);
    layout->addWidget(result);
    return result;
}
QCheckBox* checkbox(const QString& text, const char* name)
{
    auto* result = new QCheckBox(text);
    result->setObjectName(name);
    result->setChecked(true);
    return result;
}
QDoubleSpinBox* coordinate(const char* name, double min, double max, int decimals)
{
    auto* input = new QDoubleSpinBox;
    input->setObjectName(name);
    input->setRange(min, max);
    input->setDecimals(decimals);
    return input;
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Контрольная работа №2 — Вариант 1 — Полиморфизм БПЛА");
    setMinimumSize(1000, 720);
    resize(1380, 920);
    auto* root = new QWidget;
    auto* layout = new QVBoxLayout(root);
    auto* heading = new QLabel("<b>Контрольная работа №2 · Вариант 1</b> — БПЛА, сенсоры и навигация");
    layout->addWidget(heading);
    auto* split = new QSplitter;
    split->setChildrenCollapsible(false);
    layout->addWidget(split, 1);
    auto* panel = new QWidget;
    auto* left = new QVBoxLayout(panel);
    auto* tabs = new QTabWidget;
    tabs->setObjectName("taskTabs");
    left->addWidget(tabs);
    auto* flight_page = new QWidget;
    auto* flight_layout = new QVBoxLayout(flight_page);
    flight_layout->addWidget(new QLabel("Полиморфное испытание: взлёт → посадка"));
    vehicle_input_ = new QComboBox;
    vehicle_input_->setObjectName("vehicleInput");
    vehicle_input_->addItems({quadcopter_.type(), fixedwing_.type()});
    flight_layout->addWidget(vehicle_input_);
    image_ = new QLabel;
    image_->setAlignment(Qt::AlignCenter);
    image_->setMinimumHeight(135);
    flight_layout->addWidget(image_);
    test_button_ = button("Испытать БПЛА в 3D", "flightTestButton", flight_layout);
    auto* flight_note = new QLabel("Квадрокоптер: вертикальный взлёт и посадка.\nСамолёт: разбег и планирующая посадка.\nВ 3D воспроизводится учебная траектория испытания.");
    flight_note->setWordWrap(true);
    flight_layout->addWidget(flight_note);
    flight_layout->addStretch();
    tabs->addTab(flight_page, "1. БПЛА");

    auto* sensor_page = new QWidget;
    auto* sensor_layout = new QVBoxLayout(sensor_page);
    sensor_layout->addWidget(new QLabel("Снимите флажок для имитации отказа:"));
    ready_inputs_ = {checkbox("Gyroscope готов", "gyroReady"),
                     checkbox("Accelerometer готов", "accelReady"),
                     checkbox("Barometer готов", "baroReady")};
    for (auto* ready : ready_inputs_) sensor_layout->addWidget(ready);
    sensor_table_ = new QTableWidget(3, 3);
    sensor_table_->setObjectName("sensorTable");
    sensor_table_->setHorizontalHeaderLabels({"Сенсор", "Состояние", "Значение"});
    sensor_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    sensor_table_->verticalHeader()->hide();
    sensor_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sensor_layout->addWidget(sensor_table_);
    auto* check_sensors_button = button("Проверить сенсоры", "checkSensorsButton", sensor_layout);
    tabs->addTab(sensor_page, "2. Сенсоры");

    auto* navigation_page = new QWidget;
    auto* navigation_layout = new QVBoxLayout(navigation_page);
    auto* form = new QFormLayout;
    navigation_input_ = new QComboBox;
    navigation_input_->setObjectName("navigationInput");
    navigation_input_->addItems({"GPS", "Visual navigation"});
    satellites_ = new QSpinBox;
    satellites_->setObjectName("satellitesInput");
    satellites_->setRange(0, 32);
    gps_valid_ = checkbox("GPS valid", "gpsValid");
    target_tracked_ = checkbox("Визуальная цель отслеживается", "targetTracked");
    latitude_ = coordinate("latitudeInput", -90, 90, 5);
    longitude_ = coordinate("longitudeInput", -180, 180, 5);
    altitude_ = coordinate("altitudeInput", -1000, 10000, 2);
    altitude_->setSuffix(" м");
    form->addRow("Система:", navigation_input_);
    form->addRow("Спутники GPS (≥ 4):", satellites_);
    form->addRow(gps_valid_);
    form->addRow(target_tracked_);
    form->addRow("Широта:", latitude_);
    form->addRow("Долгота:", longitude_);
    form->addRow("Высота:", altitude_);
    navigation_layout->addLayout(form);
    navigation_status_ = new QLabel;
    navigation_status_->setObjectName("navigationStatus");
    navigation_status_->setWordWrap(true);
    navigation_layout->addWidget(navigation_status_);
    auto* navigation_button = button("Проверить навигацию", "checkNavigationButton", navigation_layout);
    navigation_layout->addStretch();
    tabs->addTab(navigation_page, "3. Навигация");

    auto* auto_page = new QWidget;
    auto* auto_layout = new QVBoxLayout(auto_page);
    auto* explanation = new QLabel("Обязательная интеграция Autopilot\nЦель: 200 м · коэффициент: 0,5 · шаг: 0,1 с\n\nДля эталонного цикла: высота 150 м, ошибка 50 м, управление 25.\nВ анимации: h ← h + управление × 0,1 с.");
    explanation->setWordWrap(true);
    auto_layout->addWidget(explanation);
    controller_connected_ = checkbox("Контроллер подключён", "controllerConnected");
    auto_layout->addWidget(controller_connected_);
    cycle_button_ = button("Выполнить один цикл", "cycleButton", auto_layout);
    auto_button_ = button("Анимация до 200 м", "autopilotButton", auto_layout);
    auto* fault_note = new QLabel("Отказы задаются на вкладках «Сенсоры» и «Навигация». Для проверки координат установите отрицательную высоту. Отказ во время анимации прерывает её.");
    fault_note->setWordWrap(true);
    auto_layout->addWidget(fault_note);
    auto_layout->addStretch();
    tabs->addTab(auto_page, "Интеграция");

    result_ = new QLabel;
    result_->setObjectName("resultLabel");
    result_->setWordWrap(true);
    left->addWidget(result_);
    stop_button_ = button("Остановить анимацию", "stopButton", left);
    auto* reset_button = button("Сбросить: штатный сценарий", "resetButton", left);
    sound_enabled_ = checkbox("Звук полёта и предупреждений", "soundEnabled");
    left->addWidget(sound_enabled_);
    log_ = new QPlainTextEdit;
    log_->setObjectName("eventLog");
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(600);
    left->addWidget(new QLabel("Журнал событий (подробный вывод — qDebug в консоли)"));
    left->addWidget(log_, 1);
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(panel);
    scroll->setMinimumWidth(440);
    split->addWidget(scroll);

    auto* visual_panel = new QWidget;
    auto* right = new QVBoxLayout(visual_panel);
    scene_ = new Scene3D;
    right->addWidget(scene_, 3);
    auto* camera = new QHBoxLayout;
    auto* zoom_in = button("Приблизить", "zoomInButton", camera);
    auto* zoom_out = button("Отдалить", "zoomOutButton", camera);
    auto* focus = button("К БПЛА", "focusButton", camera);
    auto* overview = button("Общий вид", "overviewButton", camera);
    right->addLayout(camera);
    auto* legend = new QLabel("Площадка 600 × 400 м · сетка 50 м · красный маркер: цель 200 м\n3D: локальная учебная сцена; широта/долгота показаны на вкладке навигации.");
    legend->setWordWrap(true);
    right->addWidget(legend);
    auto* plots = new QTabWidget;
    plots->setObjectName("graphTabs");
    altitude_graph_ = new SignalGraph("Высота, м · пунктир: цель 200 м", QColor("#278bc4"));
    altitude_graph_->setObjectName("altitudeGraph");
    altitude_graph_->set_target(200);
    control_graph_ = new SignalGraph("Управление по высоте, м/с (учебная модель)", QColor("#8c56c4"));
    control_graph_->setObjectName("controlGraph");
    plots->addTab(altitude_graph_, "Высота");
    plots->addTab(control_graph_, "Управление");
    const QStringList titles{"Gyroscope, rad/s", "Accelerometer, m/s²", "Barometer, hPa"};
    for (int i = 0; i < 3; ++i) {
        sensor_graphs_[i] = new SignalGraph(titles[i], QColor("#27946f"));
        sensor_graphs_[i]->setObjectName(QString("sensorGraph%1").arg(i));
        plots->addTab(sensor_graphs_[i], sensors_[i]->name());
    }
    right->addWidget(plots, 2);
    split->addWidget(visual_panel);
    split->setSizes({470, 870});
    setCentralWidget(root);
    audio_ = new FlightAudio(this);
    timer_ = new QTimer(this);
    timer_->setObjectName("simulationTimer");
    timer_->setInterval(50);
    connect(timer_, &QTimer::timeout, this, [this] {
        advance_simulation(std::min(0.25, clock_.restart() / 1000.0));
    });
    connect(test_button_, &QPushButton::clicked, this, &MainWindow::start_flight_test);
    connect(check_sensors_button, &QPushButton::clicked, this, &MainWindow::check_sensor_panel);
    connect(navigation_button, &QPushButton::clicked, this, &MainWindow::check_navigation_panel);
    connect(auto_button_, &QPushButton::clicked, this, &MainWindow::start_autopilot);
    connect(cycle_button_, &QPushButton::clicked, this, [this] { run_cycle(); });
    connect(stop_button_, &QPushButton::clicked, this, [this] { stop("Анимация остановлена"); });
    connect(reset_button, &QPushButton::clicked, this, &MainWindow::reset);
    connect(sound_enabled_, &QCheckBox::toggled, audio_, &FlightAudio::set_enabled);
    connect(zoom_in, &QPushButton::clicked, this, [this] { scene_->zoom(0.8f); });
    connect(zoom_out, &QPushButton::clicked, this, [this] { scene_->zoom(1.25f); });
    connect(focus, &QPushButton::clicked, scene_, &Scene3D::focusDrone);
    connect(overview, &QPushButton::clicked, scene_, &Scene3D::resetCamera);
    for (auto* input : ready_inputs_)
        connect(input, &QCheckBox::toggled, this, [this] { sync_models(); refresh_status(); });
    for (auto* input : {gps_valid_, target_tracked_})
        connect(input, &QCheckBox::toggled, this, [this] { sync_models(); refresh_status(); });
    connect(satellites_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this] { sync_models(); refresh_status(); });
    for (auto* input : {latitude_, longitude_, altitude_})
        connect(input, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this] { sync_models(); refresh_status(); });
    connect(navigation_input_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this] { sync_models(); refresh_status(); });
    connect(vehicle_input_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        prepare_scene(selected_vehicle().test_route().first());
        image_->setPixmap(QPixmap(selected_vehicle().image_resource()).scaled(240, 135, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    });
    reset();
}

MainWindow::~MainWindow() { timer_->stop(); audio_->stop(); }

UAV& MainWindow::selected_vehicle()
{
    const std::array<UAV*, 2> vehicles{&quadcopter_, &fixedwing_};
    return *vehicles.at(vehicle_input_->currentIndex());
}
NavigationSystem& MainWindow::selected_navigation()
{
    const std::array<NavigationSystem*, 2> systems{&gps_, &visual_};
    return *systems.at(navigation_input_->currentIndex());
}
void MainWindow::sync_models()
{
    gyroscope_.set_ready(ready_inputs_[0]->isChecked());
    accelerometer_.set_ready(ready_inputs_[1]->isChecked());
    barometer_.set_ready(ready_inputs_[2]->isChecked());
    gps_.set_satellites(satellites_->value());
    gps_.set_valid(gps_valid_->isChecked());
    visual_.set_target_tracked(target_tracked_->isChecked());
    const Position p{latitude_->value(), longitude_->value(), altitude_->value()};
    gps_.set_position(p);
    visual_.set_position(p);
}
void MainWindow::refresh_status()
{
    const QStringList units{"rad/s", "m/s²", "hPa"};
    for (int i = 0; i < 3; ++i) {
        Sensor& sensor = *sensors_[i];
        const bool ready = sensor.is_ready();
        const QStringList cells{sensor.name(), ready ? "Ready" : "Not ready",
            ready ? QString::number(sensor.read()) + " " + units[i] : "—"};
        for (int j = 0; j < 3; ++j) {
            auto* item = new QTableWidgetItem(cells[j]);
            item->setForeground(QColor(ready ? "#21835c" : "#c93434"));
            sensor_table_->setItem(i, j, item);
        }
    }
    const NavigationSystem& nav = selected_navigation();
    const bool valid = nav.is_valid() && valid_position(nav.get_position());
    navigation_status_->setText(QString("%1: %2\nGPS: %3 · Visual: %4")
        .arg(nav.system_name(), valid ? "Valid" : "Invalid",
             gps_.is_valid() ? "Valid" : "Invalid", visual_.is_valid() ? "Valid" : "Invalid"));
    navigation_status_->setStyleSheet(valid ? "color: #21835c" : "color: #c93434");
}
void MainWindow::record_sensors()
{
    for (int i = 0; i < 3; ++i)
        if (sensors_[i]->is_ready()) sensor_graphs_[i]->add_sample(time_, sensors_[i]->read());
}
void MainWindow::log(const QString& message, bool error)
{
    log_->appendPlainText(QDateTime::currentDateTime().toString("HH:mm:ss") + "  " + message);
    result_->setText(message);
    result_->setStyleSheet(error ? "color: #c93434; font-weight: bold" : "color: #21835c");
    if (error) audio_->alert();
}
void MainWindow::check_sensor_panel()
{
    sync_models();
    const bool ready = check_sensors(sensors_);
    record_sensors();
    refresh_status();
    log(ready ? "All sensors are ready" : "sensor failure", !ready);
}
void MainWindow::check_navigation_panel()
{
    sync_models();
    NavigationSystem& nav = selected_navigation();
    print_navigation_status(nav);
    refresh_status();
    const bool valid = nav.is_valid() && valid_position(nav.get_position());
    const Position p = nav.get_position();
    log(QString("%1: %2 · %3, %4, %5 м").arg(nav.system_name(), valid ? "Valid" : "Invalid")
        .arg(p.latitude).arg(p.longitude).arg(p.altitude), !valid);
    if (valid && mode_ == Mode::Idle) {
        prepare_scene({300, 200, p.altitude});
        altitude_graph_->add_sample(time_, p.altitude);
    }
}
void MainWindow::prepare_scene(const Point3D& position)
{
    if (!scene_vehicle_.isEmpty()) scene_->removeDrone(scene_vehicle_);
    scene_vehicle_ = selected_vehicle().type();
    scene_->setDrone(scene_vehicle_, {position}, QColor("#45b8df"));
    scene_->setDroneImage(scene_vehicle_, selected_vehicle().image_resource());
    scene_->setSelected(scene_vehicle_);
    scene_->setTarget(position.x, position.y, 200);
}
void MainWindow::clear_graphs()
{
    time_ = 0;
    accumulator_ = 0;
    altitude_graph_->clear();
    control_graph_->clear();
    for (auto* graph : sensor_graphs_) graph->clear();
}
void MainWindow::start_flight_test()
{
    run_flight_test(selected_vehicle());
    clear_graphs();
    flight_route_ = selected_vehicle().test_route();
    prepare_scene(flight_route_.first());
    altitude_graph_->add_sample(0, flight_route_.first().z);
    mode_ = Mode::FlightTest;
    audio_->start();
    clock_.start();
    timer_->start();
    update_controls();
    log("Испытание " + selected_vehicle().type() + ": воспроизведение взлёта и посадки");
}
bool MainWindow::run_cycle()
{
    sync_models();
    Autopilot autopilot(&selected_vehicle(), sensors_, &selected_navigation(),
                        controller_connected_->isChecked() ? &controller_ : nullptr);
    autopilot.run_cycle();
    const CycleResult result = autopilot.last_result();
    refresh_status();
    if (!result.success) {
        stop("Autopilot cycle aborted");
        log(result.message, true);
        return false;
    }
    altitude_graph_->add_sample(time_, result.altitude);
    control_graph_->add_sample(time_, result.control);
    record_sensors();
    log(QString("%1 · h=%2 м · ошибка=%3 м · управление=%4")
        .arg(selected_navigation().system_name()).arg(result.altitude, 0, 'f', 2)
        .arg(result.error, 0, 'f', 2).arg(result.control, 0, 'f', 2));
    if (mode_ == Mode::Autopilot) {
        const double next = result.altitude + result.control * Autopilot::time_step;
        altitude_->setValue(next);
        time_ += Autopilot::time_step;
        scene_->appendRoutePoint(scene_vehicle_, {300, 200, altitude_->value()});
        altitude_graph_->add_sample(time_, altitude_->value());
        if (std::abs(200 - altitude_->value()) <= 0.1)
            stop("Целевая высота достигнута (погрешность ≤ 0,1 м)");
    } else {
        prepare_scene({300, 200, result.altitude});
        time_ += Autopilot::time_step;
    }
    return true;
}
void MainWindow::start_autopilot()
{
    clear_graphs();
    sync_models();
    prepare_scene({300, 200, std::max(0.0, altitude_->value())});
    mode_ = Mode::Autopilot;
    if (!run_cycle() || mode_ == Mode::Idle) return;
    audio_->start();
    clock_.start();
    timer_->start();
    update_controls();
}
void MainWindow::advance_simulation(double seconds)
{
    if (!std::isfinite(seconds) || seconds <= 0 || mode_ == Mode::Idle) return;
    if (mode_ == Mode::FlightTest) {
        time_ = std::min(time_ + seconds, 12.0);
        const double phase = time_ / 12.0 * (flight_route_.size() - 1);
        const int index = std::min(int(phase), int(flight_route_.size()) - 2);
        const Point3D point = flight_route_[index]
            + (flight_route_[index + 1] - flight_route_[index]) * (phase - index);
        scene_->appendRoutePoint(scene_vehicle_, point);
        altitude_graph_->add_sample(time_, point.z);
        if (time_ >= 12) stop("Flight test completed: " + selected_vehicle().type());
    } else {
        accumulator_ += std::min(seconds, 60.0);
        while (mode_ == Mode::Autopilot && accumulator_ + 1e-9 >= Autopilot::time_step) {
            accumulator_ -= Autopilot::time_step;
            if (!run_cycle()) break;
        }
    }
}
void MainWindow::stop(const QString& message)
{
    mode_ = Mode::Idle;
    timer_->stop();
    audio_->stop();
    update_controls();
    log(message);
}
void MainWindow::update_controls()
{
    const bool idle = mode_ == Mode::Idle;
    vehicle_input_->setEnabled(idle);
    navigation_input_->setEnabled(idle);
    for (auto* input : {latitude_, longitude_, altitude_}) input->setEnabled(idle);
    test_button_->setEnabled(idle);
    cycle_button_->setEnabled(idle);
    auto_button_->setEnabled(idle);
    stop_button_->setEnabled(!idle);
}
void MainWindow::reset()
{
    stop("Сброс");
    clear_graphs();
    for (auto* ready : ready_inputs_) ready->setChecked(true);
    gps_valid_->setChecked(true);
    target_tracked_->setChecked(true);
    controller_connected_->setChecked(true);
    satellites_->setValue(8);
    latitude_->setValue(55.75);
    longitude_->setValue(37.61);
    altitude_->setValue(150);
    vehicle_input_->setCurrentIndex(0);
    navigation_input_->setCurrentIndex(0);
    sync_models();
    refresh_status();
    prepare_scene({300, 200, 150});
    scene_->resetCamera();
    image_->setPixmap(QPixmap(selected_vehicle().image_resource()).scaled(240, 135, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    altitude_graph_->add_sample(0, 150);
    record_sensors();
    log("Готово: вариант 1 · сенсоры Ready · GPS Valid · высота 150 м");
}
