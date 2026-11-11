#ifndef SCENE3D_H
#define SCENE3D_H

#include "point3d.h"
#include <QColor>
#include <QMap>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QWidget>

namespace Qt3DCore { class QEntity; class QTransform; }
namespace Qt3DExtras { class Qt3DWindow; class QCylinderMesh; }
class DroneVisual;

// Площадка, камера и DroneVisual перенесены из Practical_01_Task_1.
// В отличие от исходной сцены, здесь отображается несколько БПЛА с маршрутами.
class Scene3D : public QWidget
{
public:
    explicit Scene3D(QWidget *parent = nullptr);
    ~Scene3D() override;
    void setDrone(const QString &name, const QVector<Point3D> &route, const QColor &color);
    // Добавляет к маршруту один отрезок, не перестраивая остальные.
    void appendRoutePoint(const QString &name, const Point3D &point);
    // Перемещает маркер БПЛА без изменения маршрута (плавный автополёт).
    void setDronePosition(const QString &name, const Point3D &point);
    void setDroneImage(const QString& name, const QString& resource);
    void removeDrone(const QString &name);
    void setSelected(const QString &name);
    void setTarget(double x, double y, double altitude);
    void zoom(float factor);
    void resetCamera();
    void focusDrone();

private:
    struct DroneEntity
    {
        Qt3DCore::QEntity *group = nullptr;
        Qt3DCore::QEntity *route = nullptr;
        DroneVisual *visual = nullptr;
        Qt3DCore::QTransform *marker = nullptr;
        Qt3DCore::QEntity *stem = nullptr;
        Qt3DCore::QTransform *stemTransform = nullptr;
        Qt3DExtras::QCylinderMesh *stemMesh = nullptr;
        QColor color;
        Point3D last;
        QVector3D position;
    };

    static QVector3D toScene(const Point3D &position);
    Qt3DCore::QEntity *addBox(const Point3D &center, const Point3D &size,
                            const QColor &color, Qt3DCore::QEntity *parent);
    Qt3DCore::QEntity *addLine(const Point3D &from, const Point3D &to, const QColor &color,
                               Qt3DCore::QEntity *parent, float width = 1.5f);
    void updateAspectRatio();

    Qt3DExtras::Qt3DWindow *m_view;
    QWidget *m_container;
    Qt3DCore::QEntity *m_root;
    Qt3DCore::QTransform *m_targetTransform;
    QMap<QString, DroneEntity> m_drones;
    QString m_selected;
};

#endif // SCENE3D_H
