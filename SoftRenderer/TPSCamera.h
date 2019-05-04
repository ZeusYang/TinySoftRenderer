#ifndef TPSCAMERA_H
#define TPSCAMERA_H

#include "Camera3D.h"
#include "Transform3D.h"

/**
 * @projectName   SoftRenderer
 * @brief         Third person camera.
 * @author        YangWC
 * @date          2019-05-03
 */

namespace SoftRenderer
{

class TPSCamera : public Camera3D
{
private:
    mutable bool m_dirty;
    bool first = true;
    Vector3D m_cameraPos;
    Quaternion m_rotation;
    double m_yaw, m_pitch, m_distance;
    Transform3D m_player;
    Matrix4x4 m_viewMatrix;

public:
    TPSCamera(Vector3D target);
    virtual ~TPSCamera() = default;

    Matrix4x4 getPlayerMatrix();

    virtual Vector3D getPosition() {update();return m_cameraPos;}
    virtual Matrix4x4 getViewMatrix();
    virtual void onKeyPress(char key);
    virtual void onWheelMove(double delta);
    virtual void onMouseMove(double deltaX, double deltaY, std::string button);

    Vector3D forward() const;
    Vector3D up() const;
    Vector3D right() const;

private:
    void update();
};

}

#endif // TPSCAMERA_H
