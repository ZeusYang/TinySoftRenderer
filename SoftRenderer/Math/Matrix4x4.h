#ifndef Matrix4x4_H
#define Matrix4x4_H

#include "MathUtils.h"
#include "Vector4D.h"

/**
 * @projectName   SoftRenderer
 * @brief         4x4 matrix class.
 * @author        YangWC
 * @date          2019-04-28
 */
namespace SoftRenderer
{

class Matrix4x4
{
public:
    float entries[16];

    // constructors
    Matrix4x4(){loadIdentity();}
    Matrix4x4(float e0, float e1, float e2, float e3,
              float e4, float e5, float e6, float e7,
              float e8, float e9, float e10,float e11,
              float e12,float e13,float e14,float e15);
    Matrix4x4(const float *rhs);
    Matrix4x4(const Matrix4x4 &rhs);
    ~Matrix4x4() = default;

    // setter,getter
    void setEntry(int position, float value);
    float getEntry(int position) const;
    Vector4D getRow(int position) const;
    Vector4D getColumn(int position) const;
    void loadIdentity();
    void loadZero();

    // overloaded operators
    Matrix4x4 operator+(const Matrix4x4 & rhs) const;
    Matrix4x4 operator-(const Matrix4x4 & rhs) const;
    Matrix4x4 operator*(const Matrix4x4 & rhs) const;
    Matrix4x4 operator*(const float rhs) const;
    Matrix4x4 operator/(const float rhs) const;

    bool operator==(const Matrix4x4 & rhs) const;
    bool operator!=(const Matrix4x4 & rhs) const;

    void operator+=(const Matrix4x4 & rhs);
    void operator-=(const Matrix4x4 & rhs);
    void operator*=(const Matrix4x4 & rhs);
    void operator*=(const float rhs);
    void operator/=(const float rhs);

    Matrix4x4 operator-() const;
    Matrix4x4 operator+() const {return (*this);}
    Vector4D operator*(const Vector4D rhs) const;

    // inverse, transpose
    void inverted();
    Matrix4x4 getInverse() const;
    void transpose();
    Matrix4x4 getTranspose() const;
    void invertTranspose();
    Matrix4x4 getInverseTranspose() const;

    // operation on space
    void setTranslation(const Vector3D & translation);
    void setScale(const Vector3D & scaleFactor);
    void setRotationAxis(const double angle, const Vector3D & axis);
    void setRotationX(const double angle);
    void setRotationY(const double angle);
    void setRotationZ(const double angle);
    void setRotationEuler(const double angleX, const double angleY, const double angleZ);
    void setPerspective(float fovy, float aspect, float near, float far);
    void setOrtho(float left, float right, float bottom, float top, float near, float far);
    void setLookAt(Vector3D cameraPos, Vector3D target, Vector3D worldUp);
    void setViewPort(int left, int bottom, int width, int height);
};

}

#endif // Matrix4x4_H
