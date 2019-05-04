#include "TPSCamera.h"

#include <QDebug>

namespace SoftRenderer
{

TPSCamera::TPSCamera(Vector3D target)
    :m_yaw(0), m_pitch(30.0), m_distance(10.0)
{
    m_dirty = true;
    m_player.setTranslation(target);
}

Matrix4x4 TPSCamera::getPlayerMatrix()
{
    return m_player.toMatrix();
}

Matrix4x4 TPSCamera::getViewMatrix()
{
    update();
    return m_viewMatrix;
}

void TPSCamera::onKeyPress(char key)
{
    double speed = 2.0f;
    switch(key)
    {
    case 'W':
        m_dirty = true;
        m_player.translate(-m_player.forward() * 0.1f);
        break;
    case 'S':
        m_dirty = true;
        m_player.translate(+m_player.forward() * 0.1f);
        break;
    case 'A':
        m_dirty = true;
        m_player.rotate(m_player.up(), +speed);
        break;
    case 'D':
        m_dirty = true;
        m_player.rotate(m_player.up(), -speed);
        break;
    }
}

void TPSCamera::onWheelMove(double delta)
{
    m_dirty = true;
    double speed = 0.01;
    m_distance += -speed * delta;
    if(m_distance > 35.0)m_distance = 35.0;
    if(m_distance < 5.00)m_distance = 5.0;
}

void TPSCamera::onMouseMove(double deltaX, double deltaY, std::string button)
{
    double speed = 0.2;
    if(button == "RIGHT")
    {
        m_dirty = true;
        m_pitch += speed * deltaY;
        if(m_pitch < 0.0)m_pitch = 0.0;
        if(m_pitch > 89.9)m_pitch = 89.9;
    }
    else if(button == "LEFT")
    {
        m_dirty = true;
        m_yaw   += -speed * deltaX;
        fmod(m_yaw, 360.0);
    }
}

Vector3D TPSCamera::forward() const
{
    // must update before calling.
    return m_rotation * LocalForward;
}

Vector3D TPSCamera::up() const
{
    // must update before calling.
    return m_rotation * LocalUp;
}

Vector3D TPSCamera::right() const
{
    // must update before calling.
    return m_rotation * LocalRight;
}

void TPSCamera::update()
{
    if(m_dirty)
    {
        m_dirty = false;

        // calculate position.
        Vector3D target = m_player.translation();
        float height = m_distance * sin(radians(m_pitch));
        float horizon = m_distance * cos(radians(m_pitch));
        Vector3D _playerRot = m_player.rotation().eulerAngle();
        _playerRot.y = fmod(_playerRot.y, 360);
        m_cameraPos.y = target.y + height;
        m_cameraPos.x = target.x + horizon * sin(radians(m_yaw));
        m_cameraPos.z = target.z + horizon * cos(radians(m_yaw));
        m_viewMatrix.setLookAt(m_cameraPos, m_player.translation(), LocalUp);


    }
}

}
