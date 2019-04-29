#include "Vector3D.h"

namespace SoftRenderer
{

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
