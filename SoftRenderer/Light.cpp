#include "Light.h"

#include "Math/MathUtils.h"
#include <QDebug>

namespace SoftRenderer
{

void DirectionalLight::lighting(const Material &material,
                                const Vector3D &position,
                                const Vector3D &normal,
                                const Vector3D &eyeDir,
                                Vector3D &ambient,
                                Vector3D &diffuse,
                                Vector3D &specular) const
{
    float diff = max(normal.dotProduct(-this->m_direction), 0.0f);
    //Vector3D reflectDir = this->m_direction - normal * 2 * (normal.dotProduct(this->m_direction));
    Vector3D halfwayDir = eyeDir + this->m_direction;
    halfwayDir.normalize();
    float spec = pow(max(eyeDir.dotProduct(halfwayDir), 0.0f), material.m_shininess);
    ambient = m_ambient;
    diffuse = m_diffuse * diff;
    specular = m_specular * spec;
}

void PointLight::lighting(const Material &material,
                          const Vector3D &position,
                          const Vector3D &normal,
                          const Vector3D &eyeDir,
                          Vector3D &ambient,
                          Vector3D &diffuse,
                          Vector3D &specular) const
{
}

void SpotLight::lighting(const Material &material,
                         const Vector3D &position,
                         const Vector3D &normal,
                         const Vector3D &eyeDir,
                         Vector3D &ambient,
                         Vector3D &diffuse,
                         Vector3D &specular) const
{

}

}
