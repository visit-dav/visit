#ifndef VECTOR_H
#define VECTOR_H

class ostream;

// ****************************************************************************
//  Class:  Vector
//
//  Purpose:
//    Encapsulation of a three-component vector
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
class Vector
{
  public:
    float x,y,z;
  public:
    Vector();
    Vector(const Vector&);
    Vector(float,float,float);

    // assignment operator
    void    operator=(const Vector&);

    // vector addition/subtraction
    Vector  operator+(const Vector&) const;
    void    operator+=(const Vector&);
    Vector  operator-(const Vector&) const;
    void    operator-=(const Vector&);

    // scalar multiplication/division
    Vector  operator*(const float&) const;
    void    operator*=(const float&);
    Vector  operator/(const float&) const;
    void    operator/=(const float&);

    // cross product
    Vector  operator%(const Vector&) const;

    // dot product
    float   operator*(const Vector&) const;

    // 2-norm
    float   norm() const;
    // normalize
    void    normalize();
    Vector  normalized() const;

    // friends
    friend ostream& operator<<(ostream&,const Vector&);

    // input/output
    const char *getAsText();
    void        setAsText(const char*);
  private:
    char text[256];
};

#include <math.h>
#include <visitstream.h>

inline 
Vector::Vector()
{
    x=y=z=0;
}

inline 
Vector::Vector(float x_,float y_,float z_)
{
    x=x_;
    y=y_;
    z=z_;
}

inline 
Vector::Vector(const Vector &r)
{
    x=r.x;
    y=r.y;
    z=r.z;
}

inline void
Vector::operator=(const Vector &r)
{
    x=r.x;
    y=r.y;
    z=r.z;
}

// vector addition/subtraction

inline Vector
Vector::operator+(const Vector &r) const
{
    return Vector(x+r.x, y+r.y, z+r.z);
}

inline void
Vector::operator+=(const Vector &r)
{
    x += r.x;
    y += r.y;
    z += r.z;
}

inline Vector
Vector::operator-(const Vector &r) const
{
    return Vector(x-r.x, y-r.y, z-r.z);
}

inline void
Vector::operator-=(const Vector &r)
{
    x -= r.x;
    y -= r.y;
    z -= r.z;
}


// scalar multiplication/division

inline Vector
operator*(const float &s, const Vector &v)
{
    return Vector(v.x*s, v.y*s, v.z*s);
}

inline Vector
Vector::operator*(const float &s) const
{
    return Vector(x*s, y*s, z*s);
}

inline void
Vector::operator*=(const float &s)
{
    x *= s;
    y *= s;
    z *= s;
}

inline Vector
Vector::operator/(const float &s) const
{
    return Vector(x/s, y/s, z/s);
}

inline void
Vector::operator/=(const float &s)
{
    x /= s;
    y /= s;
    z /= s;
}



// cross product
inline Vector
Vector::operator%(const Vector &r) const
{
    Vector v;
    v.x = y*r.z - z*r.y;
    v.y = z*r.x - x*r.z;
    v.z = x*r.y - y*r.x;
    return v;
}

// dot product
inline float
Vector::operator*(const Vector &r) const
{
    return x*r.x + y*r.y + z*r.z;
}

// 2-norm
inline float
Vector::norm() const
{
    float n = (x*x + y*y + z*z);
    if (n>0)
        n = sqrt(n);
    return n;
}

// normalize
inline void
Vector::normalize()
{
    float n = (x*x + y*y + z*z);
    if (n>0)
    {
        n = 1./sqrt(n);
        x *= n;
        y *= n;
        z *= n;
    }
}

inline Vector
Vector::normalized() const
{
    float n = (x*x + y*y + z*z);
    if (n==0)
        return *this;

    n = 1./sqrt(n);
    return Vector(x*n, y*n, z*n);
}


#endif
