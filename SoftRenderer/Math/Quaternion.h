#ifndef QUATERNION_H
#define QUATERNION_H

#include "Vector3D.h"

/**
 * @projectName   SoftRenderer
 * @brief         Quaternion class.
 * @author        YangWC
 * @date          2019-05-03
 */

namespace SoftRenderer
{
class Matrix4x4;
class Quaternion
{

public:
    const static Quaternion identity;
    double x,y,z,w;

    Quaternion();
    Quaternion(float x, float y, float z, float w);
    Quaternion(float yaw, float pitch, float roll);
    ~Quaternion() = default;

    void set(float _x, float _y, float _z, float _w);
    void setEulerAngle(float yaw, float pitch, float roll);
    void setRotationAxis(Vector3D axis, double angle);

    Quaternion inverse() const;
    Quaternion conjugate() const;
    Vector3D eulerAngle() const;
    Matrix4x4 toMatrix() const;

    static float dot(const Quaternion &lhs, const Quaternion &rhs);
    static Quaternion lerp(const Quaternion &a, const Quaternion &b, float t);
    static Quaternion slerp(const Quaternion &a, const Quaternion &b, float t);
    static float angle(const Quaternion &lhs, const Quaternion &rhs);

    void operator*(float s);
    void operator+(const Quaternion &q);
    void operator-(const Quaternion &q);

    friend Quaternion operator * (const Quaternion& lhs, const Quaternion& rhs);
    friend Vector3D operator *(const Quaternion& rotation, const Vector3D& point);

private:
    Vector3D eulerAngles;
};

}
#endif // QUATERNION_H
