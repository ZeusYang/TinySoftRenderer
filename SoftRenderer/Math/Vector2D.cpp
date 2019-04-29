#include "Vector2D.h"

namespace SoftRenderer
{

void Vector2D::normalize()
{
    float length;
    float scalefactor;
    length = getLength();
    if(equal(length,1.0) || equal(length,0))
        return;
    scalefactor = 1.0f / length;
    x *= scalefactor;
    y *= scalefactor;
}

Vector2D Vector2D::getNormalize() const
{
    Vector2D result(*this);
    result.normalize();
    return result;
}

}
