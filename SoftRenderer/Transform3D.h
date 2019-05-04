#ifndef TRANSFORM3D_H
#define TRANSFORM3D_H

#include "Math/Quaternion.h"
#include "Math/Matrix4x4.h"

/**
 * @projectName   SoftRenderer
 * @brief         A 3D transformation class.
 * @author        YangWC
 * @date          2019-05-04
 */

namespace SoftRenderer
{

class Transform3D
{
private:
    mutable bool m_dirty;
    Vector3D m_scale;
    Quaternion m_rotation;
    Vector3D m_translation;
    Matrix4x4 m_world;

public:
    static const Vector3D LocalForward;
    static const Vector3D LocalUp;
    static const Vector3D LocalRight;

    Transform3D();
    ~Transform3D() = default;

    Matrix4x4 toMatrix();

    void scale(const Vector3D &ds);
    void translate(const Vector3D &dt);
    void rotate(const Vector3D &axis, float angle);

    void setScale(const Vector3D &s);
    void setRotation(const Quaternion &r);
    void setTranslation(const Vector3D &t);

    Vector3D forward() const;
    Vector3D up() const;
    Vector3D right() const;

    Vector3D translation() const {return m_translation;}
    Quaternion rotation() const {return m_rotation;}
    Vector3D scale() const {return m_scale;}
};

}

#endif // TRANSFORM3D_H
