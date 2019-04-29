#ifndef VECTOR_H
#define VECTOR_H

#include "MathUtils.h"

/**
 * @projectName   SoftRenderer
 * @brief         2D vector class.
 * @author        YangWC
 * @date          2019-04-28
 */
namespace SoftRenderer
{

class Vector2D
{
public:
    float x,y;

    // constructors
    Vector2D():x(0.0f), y(0.0f) {}
    Vector2D(float newX, float newY):x(newX), y(newY){}
    Vector2D(const float * rhs):x(*rhs), y((*rhs)+1) {}
    Vector2D(const Vector2D & rhs):x(rhs.x), y(rhs.y){}
    ~Vector2D() = default;

    // setter,getter
    void set(float newX, float newY){x=newX;y=newY;	}
    void setX(float newX) {x = newX;}
    void setY(float newY) {y = newY;}
    float getX() const {return x;}
    float getY() const {return y;}

    // normalization
    void normalize();
    Vector2D getNormalize()const;

    // length
    float getLength() const { return static_cast<float>(sqrt(x*x + y*y));}
    float getSquaredLength()const{return static_cast<float>(x*x + y*y);}

    // overloaded operators
    Vector2D operator+(const Vector2D &rhs) const {return Vector2D(x + rhs.x, y + rhs.y);}
    Vector2D operator-(const Vector2D &rhs) const {return Vector2D(x - rhs.x, y - rhs.y);}
    Vector2D operator*(const float rhs) const {return Vector2D(x*rhs, y*rhs);}
    Vector2D operator/(const float rhs) const {return (rhs==0) ? Vector2D(0.0f, 0.0f) : Vector2D(x / rhs, y / rhs);}

    bool operator==(const Vector2D &rhs) const {return (equal(x,rhs.x) && equal(y,rhs.y));}
    bool operator!=(const Vector2D &rhs) const {return !((*this)==rhs);}

    void operator+=(const Vector2D &rhs){x+=rhs.x;	y+=rhs.y;}
    void operator-=(const Vector2D &rhs){x-=rhs.x;	y-=rhs.y;}
    void operator*=(const float rhs){x*=rhs;y*=rhs;}
    void operator/=(const float rhs){if(!equal(rhs, 0.0)){x/=rhs;y/=rhs;}}

    Vector2D operator-() const {return Vector2D(-x, -y);}
    Vector2D operator+() const {return *this;}

    // interpolation
    Vector2D lerp(const Vector2D &v2,const float factor)const {return (*this)*(1.0f - factor) + v2*factor;}
    Vector2D quadraticInterpolate(const Vector2D & v2, const Vector2D & v3, const float factor) const
    {return (*this)*(1.0f-factor)*(1.0f-factor) + v2*2.0f*factor*(1.0f-factor) + v3*factor*factor;}

};

}
#endif // VECTOR_H
