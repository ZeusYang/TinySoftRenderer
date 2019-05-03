#ifndef FPSCAMERA_H
#define FPSCAMERA_H

#include "Camera3D.h"
#include "Math/Quaternion.h"

/**
 * @projectName   SoftRenderer
 * @brief         First person camera.
 * @author        YangWC
 * @date          2019-05-03
 */

namespace SoftRenderer
{

class FPSCamera : public Camera3D
{
private:
    mutable bool m_dirty;
    Vector3D m_translation;
    Quaternion m_rotation;
    Matrix4x4 m_viewMatrix;

public:

    FPSCamera(Vector3D _pos);
    virtual ~FPSCamera() = default;

    virtual Vector3D getPosition() const {return m_translation;}
    virtual Matrix4x4 getViewMatrix();
    virtual void onKeyPress(char key);
    virtual void onWheelMove(double delta);
    virtual void onMouseMove(double deltaX, double deltaY);

    void translate(const Vector3D &dt);
    void rotate(const Vector3D &axis, float angle);

    void setTranslation(const Vector3D &t);
    void setRotation(const Quaternion &r);

    Vector3D forward() const;
    Vector3D up() const;
    Vector3D right() const;
};

}

#endif // FPSCAMERA_H
