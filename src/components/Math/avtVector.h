#ifndef VECTOR_H
#define VECTOR_H
#include <math_exports.h>
#include <iostream.h>

// ****************************************************************************
//  Class:  avtVector
//
//  Purpose:
//    Encapsulation of a three-component vector.
//    Can effectively be used as a two-component vector as well.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
//  Modifications:
//    Eric Brugger, Mon Feb  9 16:32:31 PST 2004
//    Added const to a couple of constructors.
//
// ****************************************************************************
class MATH_API avtVector
{
  public:
    double x,y,z;
  public:
    avtVector();
    avtVector(const avtVector&);
    avtVector(double,double);
    avtVector(double,double,double);
    avtVector(const double*);
    avtVector(const float*);

    // assignment operator
    void       operator=(const avtVector&);

    // vector addition/subtraction
    avtVector  operator+(const avtVector&) const;
    void       operator+=(const avtVector&);
    avtVector  operator-(const avtVector&) const;
    void       operator-=(const avtVector&);

    // scalar multiplication/division
    avtVector  operator*(const double&) const;
    void       operator*=(const double&);
    avtVector  operator/(const double&) const;
    void       operator/=(const double&);

    // cross product
    avtVector  operator%(const avtVector&) const;

    // dot product
    double     operator*(const avtVector&) const;

    // 2-norm
    double     norm() const;
    // normalize
    void       normalize();
    avtVector  normalized() const;

    // friends
    friend ostream& operator<<(ostream&,const avtVector&);

    // input/output
    const char *getAsText();
    void        setAsText(const char*);
  private:
    char text[256];
};

#include <math.h>
#include <iostream.h>

inline 
avtVector::avtVector()
    : x(0), y(0), z(0)
{
}

inline 
avtVector::avtVector(double x_,double y_)
    : x(x_), y(y_), z(0)
{
}

inline 
avtVector::avtVector(double x_,double y_,double z_)
    : x(x_), y(y_), z(z_)
{
}

inline 
avtVector::avtVector(const double *p)
    : x(p[0]), y(p[1]), z(p[2])
{
}

inline 
avtVector::avtVector(const float *p)
    : x(p[0]), y(p[1]), z(p[2])
{
}

inline 
avtVector::avtVector(const avtVector &r)
{
    x=r.x;
    y=r.y;
    z=r.z;
}

inline void
avtVector::operator=(const avtVector &r)
{
    x=r.x;
    y=r.y;
    z=r.z;
}

// vector addition/subtraction

inline avtVector
avtVector::operator+(const avtVector &r) const
{
    return avtVector(x+r.x, y+r.y, z+r.z);
}

inline void
avtVector::operator+=(const avtVector &r)
{
    x += r.x;
    y += r.y;
    z += r.z;
}

inline avtVector
avtVector::operator-(const avtVector &r) const
{
    return avtVector(x-r.x, y-r.y, z-r.z);
}

inline void
avtVector::operator-=(const avtVector &r)
{
    x -= r.x;
    y -= r.y;
    z -= r.z;
}


// scalar multiplication/division

inline avtVector
avtVector::operator*(const double &s) const
{
    return avtVector(x*s, y*s, z*s);
}

inline void
avtVector::operator*=(const double &s)
{
    x *= s;
    y *= s;
    z *= s;
}

inline avtVector
avtVector::operator/(const double &s) const
{
    return avtVector(x/s, y/s, z/s);
}

inline void
avtVector::operator/=(const double &s)
{
    x /= s;
    y /= s;
    z /= s;
}



// cross product
inline avtVector
avtVector::operator%(const avtVector &r) const
{
    avtVector v;
    v.x = y*r.z - z*r.y;
    v.y = z*r.x - x*r.z;
    v.z = x*r.y - y*r.x;
    return v;
}

// dot product
inline double
avtVector::operator*(const avtVector &r) const
{
    return x*r.x + y*r.y + z*r.z;
}

// 2-norm
inline double
avtVector::norm() const
{
    double n = (x*x + y*y + z*z);
    if (n>0)
        n = sqrt(n);
    return n;
}

// normalize
inline void
avtVector::normalize()
{
    double n = (x*x + y*y + z*z);
    if (n>0)
    {
        n = 1./sqrt(n);
        x *= n;
        y *= n;
        z *= n;
    }
}

inline avtVector
avtVector::normalized() const
{
    double n = (x*x + y*y + z*z);
    if (n==0)
        return *this;

    n = 1./sqrt(n);
    return avtVector(x*n, y*n, z*n);
}


#endif
