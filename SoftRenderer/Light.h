#ifndef LIGHT_H
#define LIGHT_H

#include "Math/Vector3D.h"

namespace SoftRenderer
{

class Material
{
public:
    Material() = default;
    ~Material() = default;
    double shininess;
    Vector3D ambient;
    Vector3D diffuse;
    Vector3D specular;
    Vector3D reflect;
};

class Light
{
public:
    Light() = default;
    virtual ~Light() = default;

    virtual void lighting();
};

}

#endif // LIGHT_H
