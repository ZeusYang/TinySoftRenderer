#ifndef TPSCAMERA_H
#define TPSCAMERA_H

#include "Math/Quaternion.h"
#include "Camera3D.h"

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
    Vector3D m_translation;
    Quaternion m_rotation;
    Matrix4x4 m_viewMatrix;
    Matrix4x4 m_targetMatrix;

public:
    TPSCamera(Vector3D target);
    virtual ~TPSCamera() = default;

    virtual Vector3D getPosition() const {return m_translation;}
    virtual Matrix4x4 getViewMatrix();
    virtual void onKeyPress(char key);
    virtual void onWheelMove(double delta);
    virtual void onMouseMove(double deltaX, double deltaY);

    Vector3D forward() const;
    Vector3D up() const;
    Vector3D right() const;
};

}

#endif // TPSCAMERA_H
