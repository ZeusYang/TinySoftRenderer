#include "Quaternion.h"

#include "Matrix4x4.h"

namespace SoftRenderer
{

const Quaternion Quaternion::identity(0, 0, 0, 1);

Quaternion::Quaternion()
{
    x = y = z = 0;
    w = 1;
}

Quaternion::Quaternion(float _x, float _y, float _z, float _w)
{
    double mag = _x *_x + _y*_y + _z *_z + _w*_w;
    x = _x / mag;
    y = _y / mag;
    z = _z / mag;
    w = _w / mag;
}

Quaternion::Quaternion(float yaw, float pitch, float roll)
{
    this->setEulerAngle(yaw, pitch, roll);
}

void Quaternion::setEulerAngle(float yaw, float pitch, float roll)
{
    float  angle;
    float  sinRoll, sinPitch, sinYaw, cosRoll, cosPitch, cosYaw;

    angle = yaw * 0.5f;
    sinYaw = sin(angle);
    cosYaw = cos(angle);

    angle = pitch * 0.5f;
    sinPitch = sin(angle);
    cosPitch = cos(angle);

    angle = roll * 0.5f;
    sinRoll = sin(angle);
    cosRoll = cos(angle);

    float _x = cosRoll*cosPitch*sinYaw - sinRoll*sinPitch*cosYaw;
    float _y = cosRoll*sinPitch*cosYaw + sinRoll*cosPitch*sinYaw;
    float _z = sinRoll*cosPitch*cosYaw - cosRoll*sinPitch*sinYaw;
    float _w = cosRoll*cosPitch*cosYaw + sinRoll*sinPitch*sinYaw;

    float mag = _x *_x + _y*_y + _z *_z + _w*_w;
    x = _x / mag;
    y = _y / mag;
    z = _z / mag;
    w = _w / mag;
}

void Quaternion::setRotationAxis(Vector3D axis, double angle)
{
    angle = radians(angle);
    axis.normalize();
    double angleDiv2 = angle * 0.5;
    double sinAngle = sin(angleDiv2);
    x = axis.x * sinAngle;
    y = axis.y * sinAngle;
    z = axis.z * sinAngle;
    w = cos(angleDiv2);
}

Quaternion Quaternion::inverse() const
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::conjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

Vector3D Quaternion::eulerAngle() const
{
    float yaw = atan2(2 * (w * x + z * y), 1 - 2 * (x * x + y * y));
    float pitch = asin(2 * (w * y - x * z));
    float roll = atan2(2 * (w * z + x * y), 1 - 2 * (z * z + y * y));
//    if(pitch < -1.0f)pitch = -1.0f;
//    if(pitch > +1.0f)pitch = +1.0f;
    return Vector3D(
                angles(yaw),
                angles(pitch),
                angles(roll));
}

Matrix4x4 Quaternion::toMatrix() const
{
    Matrix4x4 result(
                1.0f - 2.0f*y*y - 2.0f*z*z, 2.0f*x*y - 2.0f*z*w, 2.0f*x*z + 2.0f*y*w, 0.0f,
                2.0f*x*y + 2.0f*z*w, 1.0f - 2.0f*x*x - 2.0f*z*z, 2.0f*y*z - 2.0f*x*w, 0.0f,
                2.0f*x*z - 2.0f*y*w, 2.0f*y*z + 2.0f*x*w, 1.0f - 2.0f*x*x - 2.0f*y*y, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    result.transpose();
    return result;
}

void Quaternion::set(float _x, float _y, float _z, float _w)
{
    x = _x;y = _y;z = _z;w = _w;
}

float Quaternion::dot(const Quaternion &lhs, const Quaternion &rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

Quaternion Quaternion::lerp(const Quaternion &a, const Quaternion &b, float t)
{
    return Quaternion(
                (1 - t) * a.x + t * b.x,
                (1 - t) * a.y + t * b.y,
                (1 - t) * a.z + t * b.z,
                (1 - t) * a.w + t * b.w
                );
}

Quaternion Quaternion::slerp(const Quaternion &a, const Quaternion &b, float t)
{
    float cos_theta = dot(a, b);

    // if B is on opposite hemisphere from A, use -B instead
    float sign;
    if (cos_theta < 0.f)
    {
        cos_theta = -cos_theta;
        sign = -1.f;
    }
    else sign = 1.f;

    float c1, c2;
    if (cos_theta > 1.f - 0.000001f)
    {
        // if q2 is (within precision limits) the same as q1,
        // just linear interpolate between A and B
        c2 = t;
        c1 = 1.f - t;
    }
    else
    {
        float theta = acos(cos_theta);
        float sin_theta = sin(theta);
        float t_theta = t*theta;
        float inv_sin_theta = 1.f / sin_theta;
        c2 = sin(t_theta) * inv_sin_theta;
        c1 = sin(theta - t_theta) * inv_sin_theta;
    }
    c2 *= sign;
    // interpolate
    return Quaternion(
                a.x * c1 + b.x * c2,
                a.y * c1 + b.y * c2,
                a.z * c1 + b.z * c2,
                a.w * c1 + b.w * c2);
}

float Quaternion::angle(const Quaternion &lhs, const Quaternion &rhs)
{
    float cos_theta = dot(lhs, rhs);
    if(cos_theta < 0.0f)
        cos_theta = -cos_theta;
    return 2.0f * angles(acos(cos_theta));
}

void Quaternion::operator*(float s)
{
    x *= s;y *= s;z *= s;w *= s;
}

void Quaternion::operator+(const Quaternion &q)
{
    x += q.x;y += q.y;z += q.z;w += q.w;
}

void Quaternion::operator-(const Quaternion &q)
{
    x -= q.x;y -= q.y;z -= q.z;w -= q.w;
}

Quaternion operator *(const Quaternion &lhs, const Quaternion &rhs)
{
    float w1 = lhs.w;
    float w2 = rhs.w;
    Vector3D v1(lhs.x, lhs.y, lhs.z);
    Vector3D v2(rhs.x, rhs.y, rhs.z);
    float w3 = w1 * w2 - v1.dotProduct(v2);
    Vector3D v3 = v1.crossProduct(v2) + v2 * w1 + v1 * w2;
    return Quaternion(v3.x, v3.y, v3.z, w3);
}

Vector3D operator *(const Quaternion &q, const Vector3D &v)
{
    // Extract the vector part of the quaternion
    Vector3D u(q.x, q.y, q.z);
    // Extract the scalar part of the quaternion
    float s = q.w;
    // Do the math
    return u * 2.0f * u.dotProduct(v)
            +  v * (s*s - u.dotProduct(u))
            +  u.crossProduct(v) * 2.0f * s;
}

}
