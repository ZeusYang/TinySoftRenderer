#include "TPSCamera.h"

namespace SoftRenderer
{

TPSCamera::TPSCamera(Vector3D target)
{
    m_dirty = true;
    m_targetMatrix.setTranslation(target);
}

Matrix4x4 TPSCamera::getViewMatrix()
{
    if(m_dirty)
    {

    }
}

void TPSCamera::onKeyPress(char key)
{

}

void TPSCamera::onWheelMove(double delta)
{

}

void TPSCamera::onMouseMove(double deltaX, double deltaY)
{

}

Vector3D TPSCamera::forward() const
{

}

Vector3D TPSCamera::up() const
{

}

Vector3D TPSCamera::right() const
{

}



}
