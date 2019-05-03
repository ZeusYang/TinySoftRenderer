#include "FPSCamera.h"

namespace SoftRenderer
{

FPSCamera::FPSCamera(Vector3D _pos)
    :m_dirty(true), m_translation(_pos)
{
}

Matrix4x4 FPSCamera::getViewMatrix()
{
    if(m_dirty)
    {
        m_dirty = false;
        m_viewMatrix.loadIdentity();
        m_viewMatrix = m_rotation.conjugate().toMatrix();
        Matrix4x4 trans;
        trans.setTranslation(-m_translation);
        m_viewMatrix *= trans;
    }
    return m_viewMatrix;
}

void FPSCamera::onKeyPress(char key)
{
    switch(key)
    {
    case 'W':
        this->translate(forward() * 0.2f);
        break;
    case 'S':
        this->translate(-forward() * 0.2f);
        break;
    case 'A':
        this->translate(-right() * 0.2f);
        break;
    case 'D':
        this->translate(+right() * 0.2f);
        break;
    case 'Q':
        this->translate(up() * 0.2f);
        break;
    case 'E':
        this->translate(-up() * 0.2f);
        break;
    default:
        break;
    }
}

void FPSCamera::onWheelMove(double delta)
{
    // nothing now.
}

void FPSCamera::onMouseMove(double deltaX, double deltaY)
{
    double speed = 0.1f;
    deltaX *= speed;
    deltaY *= speed;
    this->rotate(LocalUp, -deltaX);
    this->rotate(right(), -deltaY);
}

void FPSCamera::translate(const Vector3D &dt)
{
    m_dirty = true;
    m_translation += dt;
}

void FPSCamera::rotate(const Vector3D &axis, float angle)
{
    m_dirty = true;
    Quaternion newRot;
    newRot.setRotationAxis(axis, angle);
    m_rotation = newRot * m_rotation;
}

void FPSCamera::setTranslation(const Vector3D &t)
{
    m_dirty = true;
    m_translation = t;
}

void FPSCamera::setRotation(const Quaternion &r)
{
    m_dirty = true;
    m_rotation = r;
}

Vector3D FPSCamera::forward() const
{
    return m_rotation * LocalForward;
}

Vector3D FPSCamera::up() const
{
    return m_rotation * LocalUp;
}

Vector3D FPSCamera::right() const
{
    return m_rotation * LocalRight;
}

}
