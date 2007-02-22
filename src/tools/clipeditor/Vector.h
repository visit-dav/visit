/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef VECTOR_H
#define VECTOR_H

#include <visitstream.h>

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
