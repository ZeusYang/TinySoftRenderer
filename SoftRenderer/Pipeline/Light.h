#ifndef LIGHT_H
#define LIGHT_H

#include "../Math/Vector3D.h"

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

    void setDirectionalLight(Vector3D _amb, Vector3D _diff, Vector3D _spec,
                             Vector3D _dir)
    {
        m_ambient = _amb;
        m_diffuse = _diff;
        m_specular = _spec;
        m_direction = _dir;
        m_direction.normalize();
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

    void setPointLight(Vector3D _amb, Vector3D _diff, Vector3D _spec,
                       Vector3D _pos, Vector3D _atte)
    {
        m_ambient = _amb;
        m_diffuse = _diff;
        m_specular = _spec;
        m_position = _pos;
        m_attenuation = _atte;
    }
};

class SpotLight : public Light
{
public:
    double m_cutoff, m_outcutoff;
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

    void setSpotLight(Vector3D _amb, Vector3D _diff, Vector3D _spec,
                      double _cut, Vector3D _pos, Vector3D _dir, Vector3D _atte)
    {
        m_cutoff = cos(_cut * M_PI/180.0);
        m_outcutoff = cos((_cut + 10.0) * M_PI/180.0);
        m_ambient = _amb;
        m_diffuse = _diff;
        m_specular = _spec;
        m_position = _pos;
        m_direction = _dir;
        m_attenuation = _atte;
        m_direction.normalize();
    }
};

}

#endif // LIGHT_H
