#include "Vector3D.h"

#include "Vector4D.h"

namespace SoftRenderer
{

Vector3D::Vector3D(const Vector4D &rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
}

void Vector3D::normalize()
{
    float length = getLength();
    if(equal(length,1.0f) || equal(length,0.0f))
        return;
    float scalefactor = 1.0f / length;
    x *= scalefactor;
    y *= scalefactor;
    z *= scalefactor;
}

Vector3D Vector3D::getNormalized() const
{
    Vector3D result(*this);
    result.normalize();
    return result;
}

}
