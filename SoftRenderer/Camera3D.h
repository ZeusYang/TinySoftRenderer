#ifndef CAMERA3D_H
#define CAMERA3D_H

#include "Math/Matrix4x4.h"

/**
 * @projectName   SoftRenderer
 * @brief         A virtual camera class.
 * @author        YangWC
 * @date          2019-05-03
 */
namespace SoftRenderer
{

class Camera3D
{
public:
    static const Vector3D LocalForward;
    static const Vector3D LocalUp;
    static const Vector3D LocalRight;

    Camera3D() = default;
    virtual ~Camera3D(){}

    virtual Matrix4x4 getViewMatrix() = 0;
    virtual Vector3D getPosition() = 0;
    virtual void onKeyPress(char key) = 0;
    virtual void onWheelMove(double delta) = 0;
    virtual void onMouseMove(double deltaX, double deltaY, std::string button) = 0;
};

}

#endif // CAMERA3D_H
