#include "Transform3D.h"

namespace SoftRenderer
{

const Vector3D Transform3D::LocalUp(0.0f,1.0f,0.0f);
const Vector3D Transform3D::LocalForward(0.0f,0.0f,1.0f);
const Vector3D Transform3D::LocalRight(1.0f,0.0f,0.0f);

Transform3D::Transform3D()
    :m_dirty(true), m_scale(1.0,1.0,1.0)
{
    m_world.loadIdentity();
}

Matrix4x4 Transform3D::toMatrix()
{
    if(m_dirty)
    {
        m_dirty = false;
        m_world = m_rotation.toMatrix();
        Matrix4x4 trans, scals;
        scals.setScale(m_scale);
        trans.setTranslation(m_translation);
        m_world =  trans * m_world * scals;
    }
    return m_world;
}

void Transform3D::scale(const Vector3D &ds)
{
    m_dirty = true;
    m_scale.x *= ds.x;
    m_scale.y *= ds.y;
    m_scale.z *= ds.z;
}

void Transform3D::translate(const Vector3D &dt)
{
    m_dirty = true;
    m_translation += dt;
}

void Transform3D::rotate(const Vector3D &axis, float angle)
{
    m_dirty = true;
    Quaternion newRot;
    newRot.setRotationAxis(axis, angle);
    m_rotation = newRot * m_rotation;
}

void Transform3D::setScale(const Vector3D &s)
{
    m_dirty = true;
    m_scale = s;
}

void Transform3D::setRotation(const Quaternion &r)
{
    m_dirty = true;
    m_rotation = r;
}

void Transform3D::setTranslation(const Vector3D &t)
{
    m_dirty = true;
    m_translation = t;
}

Vector3D Transform3D::forward() const
{
    return m_rotation * LocalForward;
}

Vector3D Transform3D::up() const
{
    return m_rotation * LocalUp;
}

Vector3D Transform3D::right() const
{
    return m_rotation * LocalRight;
}


}
