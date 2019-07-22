// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <math_exports.h>
#include <visitstream.h>
#include <ImproperUseException.h>

#ifdef DBIO_ONLY 
#define STUB_VOID {}
#define STUB_OSTR {return ostr;}
#define STUB_STR {return "";}
#else
#define STUB_VOID
#define STUB_OSTR
#define STUB_STR
#endif

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
//    Dave Pugmire, Mon Nov 17 12:05:04 EST 2008
//    Added operators == != and methods dot, cross, length2 and length.
//
//    Kathleen Bonnell, Mon Apr 20 10:38:22 MST 2009 
//    Added MATH_API in front of operator<< for compilation on windows when
//    other classes attempt to use the method.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Add [] operator and lerp method.
//
//    Dave Pugmire (for Chrisoph Garth), Wed Jan 20 09:28:59 EST 2010
//    Remove the getText method.
//
//    Dave Pugmire, Mon Feb 15 14:05:19 EST 2010
//    Add perpendiculars method and unary - operator.
//
//    Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//    Add set() methods.
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
    avtVector& operator=(const avtVector&);

    // vector addition/subtraction
    avtVector  operator+(const avtVector&) const;
    void       operator+=(const avtVector&);
    avtVector  operator-(const avtVector&) const;
    void       operator-=(const avtVector&);
    avtVector  operator-() const;

    // scalar multiplication/division
    avtVector  operator*(const double&) const;
    void       operator*=(const double&);
    avtVector  operator/(const double&) const;
    void       operator/=(const double&);

    // comparison.
    bool       operator==(const avtVector&) const;
    bool       operator!=(const avtVector&) const;

    // cross product
    avtVector  operator%(const avtVector&) const;
    avtVector  cross(const avtVector&) const;

    // dot product
    double     operator*(const avtVector&) const;
    double     dot(const avtVector&) const;

    // element access
    double& operator[](const size_t &i);
    const double& operator[](const size_t &i) const;
    void  set(const double *v);
    void  set(const float *v);
    void  set(double, double, double);
    void  set(float, float, float);

    // 2-norm
    double     norm() const;
    // normalize
    void       normalize();
    avtVector  normalized() const;

    // length.
    double length2() const;
    double length() const;

    static void perpendiculars(const avtVector &x, avtVector &y, avtVector &z, double theta);

    // friends
    MATH_API friend ostream& operator<<(ostream& ostr,const avtVector&) STUB_OSTR;
};

#undef STUB_VOID
#undef STUB_OSTR
#undef STUB_STR

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

inline avtVector&
avtVector::operator=(const avtVector &r)
{
    x=r.x;
    y=r.y;
    z=r.z;

    return *this;
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

inline avtVector
avtVector::operator-() const
{
    return avtVector(-x, -y, -z);
}

inline void
avtVector::operator-=(const avtVector &r)
{
    x -= r.x;
    y -= r.y;
    z -= r.z;
}


// scalar multiplication/division

// Allows for double * avtVector so that everything doesn't have to be
// avtVector * double
inline avtVector operator*(const double s, const avtVector& v) {
  return v*s;
}

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

inline bool
avtVector::operator==(const avtVector &v) const
{
    return x == v.x && y == v.y && z == v.z;
}

inline bool
avtVector::operator!=(const avtVector &v) const
{
    return x != v.x || y != v.y || z != v.z;
}


// cross product
inline avtVector
avtVector::operator%(const avtVector &r) const
{
    return cross(r);
}

inline avtVector
avtVector::cross(const avtVector &r) const
{
    avtVector v;
    v.x = y*r.z - z*r.y;
    v.y = z*r.x - x*r.z;
    v.z = x*r.y - y*r.x;
    return v;
}

inline avtVector Cross(const avtVector& v0, const avtVector& v1) {
  return v0.cross(v1);
}

// dot product
inline double
avtVector::operator*(const avtVector &r) const
{
    return dot(r);
}

inline double
avtVector::dot(const avtVector &r) const
{
    return x*r.x + y*r.y + z*r.z;
}

inline double Dot(const avtVector& v0, const avtVector& v1) {
  return v0.dot(v1);
}

// 2-norm
inline double
avtVector::norm() const
{
    double n = length2();
    if (n>0)
        n = sqrt(n);
    return n;
}

// normalize
inline void
avtVector::normalize()
{
    double n = length2();
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
    double n = length2();
    if (n==0)
        return *this;

    n = 1./sqrt(n);
    return avtVector(x*n, y*n, z*n);
}

// length
inline double
avtVector::length2() const
{
    return (x*x + y*y + z*z);
}

inline double
avtVector::length() const
{
    return sqrt(length2());
}

inline double &
avtVector::operator[](const size_t &i)
{
    if (i == 0)
        return x;
    else if (i == 1)
        return y;
    else if (i == 2)
        return z;
    EXCEPTION0(ImproperUseException);
}

inline const double &
avtVector::operator[](const size_t &i) const
{
    if (i == 0)
        return x;
    else if (i == 1)
        return y;
    else if (i == 2)
        return z;
    EXCEPTION0(ImproperUseException);
}

inline avtVector lerp(const double &t, const avtVector &v0, const avtVector &v1)
{
    double one_minus_t = 1.0-t;

    avtVector result(one_minus_t*v0.x + t*v1.x,
                     one_minus_t*v0.y + t*v1.y,
                     one_minus_t*v0.z + t*v1.z);
    return result;
}

inline void avtVector::set(const double *v)
{
    x=v[0];
    y=v[1];
    z=v[2];
}

inline void avtVector::set(const float *v)
{
    x=v[0];
    y=v[1];
    z=v[2];
}

inline void avtVector::set(double v0, double v1, double v2)
{
    x = v0;
    y = v1;
    z = v2;
}

inline void avtVector::set(float v0, float v1, float v2)
{
    x = v0;
    y = v1;
    z = v2;
}


#endif
