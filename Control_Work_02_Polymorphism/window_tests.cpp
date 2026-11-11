#include "mainwindow.h"
#include "signalgraph.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTest>
#include <QTimer>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QFrameGraphNode>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QRenderCapture>
#include <cmath>

namespace {
void check(bool condition, const char* message) { if (!condition) qFatal("FAILED: %s", message); }
template<typename T> T* child(MainWindow& window, const char* name)
{
    auto* widget = window.findChild<T*>(name);
    check(widget != nullptr, name);
    return widget;
}
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    check(!QImage(":/assets/drone.jpg").isNull(), "embedded quad image");
    check(!QImage(":/assets/fixedwing.svg").isNull(), "embedded wing image");
    check(QFile::exists(":/assets/drone_sound.mp3") && QFile::exists(":/assets/critical.mp3"), "audio resources");
    MainWindow window;
    auto click = [&](const char* name) { child<QPushButton>(window, name)->click(); };
    auto* timer = child<QTimer>(window, "simulationTimer");
    auto* sound = child<QCheckBox>(window, "soundEnabled");
    sound->setChecked(false);
    auto* status = child<QLabel>(window, "resultLabel");
    auto* altitude = child<QDoubleSpinBox>(window, "altitudeInput");
    auto* graph = static_cast<SignalGraph*>(child<QWidget>(window, "altitudeGraph"));
    auto* controls = static_cast<SignalGraph*>(child<QWidget>(window, "controlGraph"));
    click("cycleButton");
    check(status->text().contains("ошибка=50.00") && status->text().contains("управление=25.00"), "reference GUI cycle");
    check(altitude->value() == 150 && controls->history().last().y() == 25, "single cycle does not integrate altitude");
    child<QCheckBox>(window, "gyroReady")->setChecked(false);
    click("cycleButton");
    check(status->text() == "Reason: sensor failure", "sensor failure GUI");
    click("resetButton");
    child<QSpinBox>(window, "satellitesInput")->setValue(3);
    click("cycleButton");
    check(status->text() == "Navigation data invalid", "invalid GPS GUI");
    child<QComboBox>(window, "navigationInput")->setCurrentIndex(1);
    click("cycleButton");
    check(status->text().contains("Visual navigation"), "manual switch to visual navigation");
    child<QCheckBox>(window, "targetTracked")->setChecked(false);
    click("cycleButton");
    check(status->text() == "Navigation data invalid", "lost target GUI");
    click("resetButton");
    child<QCheckBox>(window, "controllerConnected")->setChecked(false);
    click("cycleButton");
    check(status->text() == "Error: flight controller is not set", "missing controller GUI");
    click("resetButton");
    altitude->setValue(-1);
    click("autopilotButton");
    check(status->text() == "Error: invalid navigation position" && !timer->isActive(), "negative altitude blocks animation");

    click("resetButton");
    click("autopilotButton");
    timer->stop();
    check(altitude->value() == 152.5, "first integrated altitude");
    window.advance_simulation(0.1);
    check(std::abs(altitude->value() - 154.88) < 0.01, "second integrated altitude");
    check(!child<QComboBox>(window, "vehicleInput")->isEnabled(), "vehicle locked during animation");
    child<QCheckBox>(window, "baroReady")->setChecked(false);
    const double before_failure = altitude->value();
    window.advance_simulation(1);
    check(altitude->value() == before_failure && status->text() == "Reason: sensor failure", "in-flight failure stops movement");
    check(child<QComboBox>(window, "vehicleInput")->isEnabled(), "controls unlocked after failure");
    click("resetButton");
    click("autopilotButton");
    timer->stop();
    window.advance_simulation(30);
    check(std::abs(altitude->value() - 200) <= 0.10001 && status->text().contains("Целевая высота"), "converges to target");
    check(!child<QPushButton>(window, "stopButton")->isEnabled(), "automatic stop at target");

    click("resetButton");
    click("flightTestButton");
    timer->stop();
    window.advance_simulation(2);
    check(std::abs(graph->history().last().y() - 60) < 0.01, "vertical takeoff midpoint");
    window.advance_simulation(10);
    check(graph->history().last().y() == 0 && status->text().contains("Flight test completed"), "quad lands");
    child<QComboBox>(window, "vehicleInput")->setCurrentIndex(1);
    click("flightTestButton");
    timer->stop();
    window.advance_simulation(1.5);
    check(graph->history().last().y() == 0, "fixed wing ground run");
    window.advance_simulation(3);
    check(std::abs(graph->history().last().y() - 60) < 0.01, "fixed wing climb");
    window.advance_simulation(7.5);
    check(graph->history().last().y() == 0 && !child<QPushButton>(window, "stopButton")->isEnabled(), "fixed wing gliding landing");

    SignalGraph bounded("test", Qt::blue);
    for (int i = 0; i < 1000; ++i) bounded.add_sample(i, -i);
    check(bounded.history().size() == 600, "bounded graph history");
    bounded.add_sample(-1, 0);
    check(bounded.history().last().x() == 999, "out of order graph sample rejected");

    // При KR2_SCREENSHOT выполняется также настоящий рендер на рабочем столе.
    const QString screenshot = qEnvironmentVariable("KR2_SCREENSHOT");
    if (!screenshot.isEmpty()) {
        click("resetButton");
        click("autopilotButton");
        timer->stop();
        window.advance_simulation(1.5);
        child<QTabWidget>(window, "taskTabs")->setCurrentIndex(3);
        window.showMaximized();
        Qt3DExtras::Qt3DWindow* view = nullptr;
        for (auto* candidate : QApplication::allWindows())
            if (auto* scene = qobject_cast<Qt3DExtras::Qt3DWindow*>(candidate)) view = scene;
        check(view, "3D view exists");
        Qt3DRender::QRenderCapture* capture = nullptr;
        for (auto* node : view->activeFrameGraph()->findChildren<Qt3DRender::QFrameGraphNode*>()) {
            if (!qobject_cast<Qt3DRender::QNoDraw*>(node)
                && node->findChildren<Qt3DRender::QFrameGraphNode*>(QString(), Qt::FindDirectChildrenOnly).isEmpty()) {
                capture = new Qt3DRender::QRenderCapture(node);
                break;
            }
        }
        check(capture, "capture branch");
        QTest::qWait(1500);
        auto* reply = capture->requestCapture();
        check(QTest::qWaitFor([reply] { return reply->isComplete(); }, 10000), "3D framebuffer rendered");
        check(reply->image().save(screenshot + ".scene.png"), "save scene");
        check(window.grab().save(screenshot + ".window.png"), "save interface");
        check(graph->grab().save(screenshot + ".graph.png"), "save graph");
        delete reply;
        delete capture;
        window.close();
    }
    qDebug().noquote() << "All window checks passed";
    return 0;
}
