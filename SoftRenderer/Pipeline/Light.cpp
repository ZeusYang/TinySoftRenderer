#include "Light.h"

#include "../Math/MathUtils.h"

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
    // ambient
    ambient = this->m_ambient;

    // diffuse
    Vector3D lightDir = (this->m_position - position);
    lightDir.normalize();
    float diff = max(normal.dotProduct(lightDir), 0.0f);
    diffuse = this->m_diffuse * diff;

    // specular
    Vector3D halfwayDir = eyeDir + lightDir;
    halfwayDir.normalize();
    float spec = pow(max(eyeDir.dotProduct(halfwayDir), 0.0f), material.m_shininess);
    specular = this->m_specular * spec;

    // attenuation
    float distance    = (this->m_position - position).getLength();
    float attenuation = 1.0 / (m_attenuation.x +
                               m_attenuation.y * distance +
                               m_attenuation.z * (distance * distance));
    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;
}

void SpotLight::lighting(const Material &material,
                         const Vector3D &position,
                         const Vector3D &normal,
                         const Vector3D &eyeDir,
                         Vector3D &ambient,
                         Vector3D &diffuse,
                         Vector3D &specular) const
{
    // ambient
    ambient = this->m_ambient;

    // diffuse
    Vector3D lightDir = this->m_position - position;
    lightDir.normalize();
    float diff = max(normal.dotProduct(lightDir), 0.0f);
    diffuse = this->m_diffuse * diff ;

    // specular
    Vector3D halfwayDir = eyeDir + lightDir;
    halfwayDir.normalize();
    float spec = pow(max(eyeDir.dotProduct(halfwayDir), 0.0f), material.m_shininess);
    specular = this->m_specular * spec;

    // spotlight (soft edges)
    float theta = lightDir.dotProduct(-this->m_direction);
    float epsilon = (this->m_cutoff - this->m_outcutoff);
    float intensity = (theta - this->m_outcutoff) / epsilon;
    if(intensity < 0.0f)intensity = 0.0f;
    if(intensity > 1.0f)intensity = 1.0f;
    diffuse  *= intensity;
    specular *= intensity;

    // attenuation
    float distance    = (this->m_position - position).getLength();
    float attenuation = 1.0 / (m_attenuation.x +
                               m_attenuation.y * distance +
                               m_attenuation.z * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
}

}
