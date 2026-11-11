#ifndef DRONEVISUAL_H
#define DRONEVISUAL_H

#include <Qt3DCore/QEntity>
#include <QVector3D>

namespace Qt3DCore { class QTransform; }
namespace Qt3DRender { class QCamera; class QTextureLoader; }

class DroneVisual : public Qt3DCore::QEntity
{
public:
    DroneVisual(Qt3DRender::QCamera *camera, Qt3DCore::QEntity *parent);
    void setPosition(const QVector3D &position);
    void setImage(const QString& resource);

private:
    void faceCamera();

    Qt3DCore::QTransform *m_transform;
    Qt3DRender::QCamera *m_camera;
    Qt3DRender::QTextureLoader *m_texture;
};

#endif
