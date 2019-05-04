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
    mutable bool m_dirty;               // Should update or not.
    Vector3D m_cameraPos;               // Camera's position.
    Transform3D m_player;               // Player's transformation.
    Matrix4x4 m_viewMatrix;             // View matrix.
    double m_yaw, m_pitch, m_distance;  // yaw, pitch and distance to player's space.

public:
    // ctor/dtor.
    TPSCamera(Vector3D target);
    virtual ~TPSCamera() = default;

    // Getter.
    Matrix4x4 getPlayerMatrix();
    virtual Matrix4x4 getViewMatrix();
    virtual Vector3D getPosition() {update();return m_cameraPos;}

    // Key/Mouse reaction.
    virtual void onKeyPress(char key);
    virtual void onWheelMove(double delta);
    virtual void onMouseMove(double deltaX, double deltaY, std::string button);

private:
    // Update view matrix.
    void update();
};

}

#endif // TPSCAMERA_H
