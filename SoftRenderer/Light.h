#ifndef LIGHT_H
#define LIGHT_H

#include "Math/Vector3D.h"

/**
 * @projectName   SoftRenderer
 * @brief         Lighting class.
 * @author        YangWC
 * @date          2019-05-02
 */

namespace SoftRenderer
{

class Material
{
public:
    Material() = default;
    ~Material() = default;
    double m_shininess;
    Vector3D m_ambient;
    Vector3D m_diffuse;
    Vector3D m_specular;
    Vector3D m_reflect;
    void setMaterial(Vector3D _amb, Vector3D _diff, Vector3D _spec, double _shin)
    {
        m_shininess = _shin;
        m_ambient = _amb;
        m_diffuse = _diff;
        m_specular = _spec;
    }
};

class Light
{
public:
    Light() = default;
    virtual ~Light() = default;

    virtual void lighting(const Material &material,
                          const Vector3D &position,
                          const Vector3D &normal,
                          const Vector3D &eyeDir,
                          Vector3D& ambient,
                          Vector3D& diffuse,
                          Vector3D& specular) const = 0;
};

class DirectionalLight : public Light
{
public:
    Vector3D m_ambient;
    Vector3D m_diffuse;
    Vector3D m_specular;
    Vector3D m_direction;

    virtual void lighting(const Material &material,
                          const Vector3D &position,
                          const Vector3D &normal,
                          const Vector3D &eyeDir,
                          Vector3D& ambient,
                          Vector3D& diffuse,
                          Vector3D& specular) const;

    void setDirectionalLight(Vector3D _amb, Vector3D _diff, Vector3D _spec, Vector3D _dir)
    {
        m_ambient = _amb;
        m_diffuse = _diff;
        m_specular = _spec;
        _dir.normalize();
        m_direction = _dir;
    }
};

class PointLight : public Light
{
public:
    Vector3D m_ambient;
    Vector3D m_diffuse;
    Vector3D m_specular;
    Vector3D m_position;
    Vector3D m_attenuation;

    virtual void lighting(const Material &material,
                          const Vector3D &position,
                          const Vector3D &normal,
                          const Vector3D &eyeDir,
                          Vector3D& ambient,
                          Vector3D& diffuse,
                          Vector3D& specular) const;
};

class SpotLight : public Light
{
public:
    double spot;
    Vector3D m_ambient;
    Vector3D m_diffuse;
    Vector3D m_specular;
    Vector3D m_position;
    Vector3D m_direction;
    Vector3D m_attenuation;

    virtual void lighting(const Material &material,
                          const Vector3D &position,
                          const Vector3D &normal,
                          const Vector3D &eyeDir,
                          Vector3D& ambient,
                          Vector3D& diffuse,
                          Vector3D& specular) const;
};

}

#endif // LIGHT_H
