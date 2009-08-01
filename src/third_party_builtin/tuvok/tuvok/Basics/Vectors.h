/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    Vectors.h
  \brief   Vector, matrix, and quaternion templates with additional OpenGL and Direct3D features
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 3.0
  \date    December 2008
*/

#pragma once

#ifndef VECTORS_H
#define VECTORS_H

#if defined DIRECT3D_VERSION
  #define USEDX
#endif

#if defined __GL_H__
  #define USEGL
#endif

// some DX files define min/max but that interferes
// with the numeric_limits so undef them
#ifdef min
  #undef min
#endif

#ifdef max
  #undef max
#endif

// use MAX / MIN instead
#ifndef MAX
  #define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
  #define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <cassert>
#include <cmath>
#include <istream>
#include <iomanip>
#include <limits>
#include <vector>

#ifdef WIN32
  #pragma warning( disable : 4995 ) // disable deprecated warning
#endif
#include <ostream>
#ifdef WIN32
  #pragma warning( default : 4995 )
#endif

#ifdef WIN32
  #pragma warning( disable : 4201 )  // Disable warning messages about nameless union
  #ifdef USEDX
    #pragma message("    [vectors.h] Using DX extensions.\n")
  #endif
  #ifdef USEGL
    #pragma message("    [vectors.h] Using GL extensions.\n")
  #endif
#endif

#include "StdDefines.h"

template <class T> class MATRIX2;
template <class T> class MATRIX3;
template <class T> class MATRIX4;

template <class T=int> class VECTOR2 {
public:
  T x,y;

  VECTOR2<T>(): x(0), y(0) {}
  template <class S> explicit VECTOR2<T>( const std::vector<S>& v ) {
    x = T(v.size()>0 ? v[0] : 0);
    y = T(v.size()>1 ? v[1] : 0);
  }
  VECTOR2<T>(const VECTOR2<T> &other): x(other.x), y(other.y) {}
  template <class S> explicit VECTOR2<T>(const VECTOR2<S> &other): x(T(other.x)), y(T(other.y)) {}
  VECTOR2<T>(const T _x, const T _y) : x(_x), y(_y) {}
  VECTOR2<T>(const T* vec) : x(vec[0]), y(vec[1]) {}

  bool operator == ( const VECTOR2<T>& other ) const {return (other.x==x && other.y==y); }
  bool operator != ( const VECTOR2<T>& other ) const {return (other.x!=x || other.y!=y); }

    // binary operators with scalars
  VECTOR2<T> operator + ( T scalar ) const {return VECTOR2<T>(x+scalar,y+scalar);}
  VECTOR2<T> operator - ( T scalar ) const {return VECTOR2<T>(x-scalar,y-scalar);}
  VECTOR2<T> operator * ( T scalar ) const {return VECTOR2<T>(x*scalar,y*scalar);}
  VECTOR2<T> operator / ( T scalar ) const {return VECTOR2<T>(x/scalar,y/scalar);}
  VECTOR2<T> operator % ( T scalar ) const {return VECTOR2<T>(x%scalar,y%scalar);}

  // binary operators with vectors
  VECTOR2<T> operator + ( const VECTOR2& other ) const {return VECTOR2<T>(x+other.x,y+other.y);}
  VECTOR2<T> operator - ( const VECTOR2& other ) const {return VECTOR2<T>(x-other.x,y-other.y);}
  VECTOR2<T> operator / ( const VECTOR2& other ) const {return VECTOR2<T>(x/other.x,y/other.y);}
  VECTOR2<T> operator * ( const VECTOR2& other ) const {return VECTOR2<T>(x*other.x,y*other.y);} // component product
  T operator ^ ( const VECTOR2<T>& other ) const {return T(x*other.x+y*other.y);} // dot product

  // unary operators
  VECTOR2<T> operator + () const {return *this;}
  VECTOR2<T> operator - () const {return *this * -1;}
  VECTOR2<T> operator ~ () const {return VECTOR2<T>(T(1)/x,T(1)/y);}

  // binary operators with a matrix
  VECTOR2<T> operator * ( const MATRIX2<T>& matrix ) const {
    return VECTOR2<T>(x*matrix.m11+y*matrix.m21,
                      x*matrix.m12+y*matrix.m22);
  }

  VECTOR2<T>& operator=(const VECTOR2<T>& other)  { x = other.x; y = other.y;return *this; }
  VECTOR2<T>& operator+=(const VECTOR2<T>& other) { x += other.x; y += other.y;return *this; }
  VECTOR2<T>& operator-=(const VECTOR2<T>& other) { x -= other.x; y -= other.y;return *this; }
  VECTOR2<T>& operator*=(const VECTOR2<T>& other) { x *= other.x; y *= other.y;return *this; }
  VECTOR2<T>& operator/=(const VECTOR2<T>& other) { x /= other.x; y /= other.y;return *this; }
  VECTOR2<T>& operator=(const T& other) { x = other; y = other;return *this; }
  VECTOR2<T>& operator+=(const T& other) { x += other; y += other;return *this; }
  VECTOR2<T>& operator-=(const T& other) { x -= other; y -= other;return *this; }
  VECTOR2<T>& operator*=(const T& other) { x *= other; y *= other;return *this; }
  VECTOR2<T>& operator/=(const T& other) { x /= other; y /= other;return *this; }

  friend std::ostream& operator<<(std::ostream &os,const VECTOR2<T>& v){os << v.x << '\t' << v.y; return os;}

  T& operator [](size_t i) {
    assert(i <= 1);
    return (i == 0) ? this->x : this->y;
  }
  const T& operator [](size_t i) const {
    assert(i <= 1);
    return (i == 0) ? this->x : this->y;
  }

  VECTOR2<T> abs() const {return VECTOR2<T>(fabs(x),fabs(y));}
  T area() const {return x*y;}
  T length() const {return sqrt(T(x*x+y*y));}
  void normalize() {T len = length(); x/=len;y/=len;}
  T maxVal() const {return MAX(x,y);}
  T minVal() const {return MIN(x,y);}

  VECTOR2<T> makepow2() const {
    VECTOR2<T> vOut;
    vOut.x = T(1<<int(ceil(log(double(x))/log(2.0))));
    vOut.y = T(1<<int(ceil(log(double(y))/log(2.0))));
    return vOut;
  }

  void StoreMin(const VECTOR2<T> &other) {
    x = std::min(x,other.x);
    y = std::min(y,other.y);
  }

  void StoreMax(const VECTOR2<T> &other) {
    x = std::max(x,other.x);
    y = std::max(y,other.y);
  }

  #ifdef USEDX
    VECTOR2<T>(const D3DXVECTOR2 &other): x(T(other.x)), y(T(other.y)) {}
    D3DXVECTOR2 toD3DXVEC() const {return D3DXVECTOR2(float(x),float(y));}
    bool operator == ( const D3DXVECTOR2& other ) const {return (other.x==T(x) && other.y== T(y)); }
    bool operator != ( const D3DXVECTOR2& other ) const {return (other.x!=T(x) || other.y!= T(y)); }
    operator D3DXVECTOR2(void) const {return toD3DXVEC();}
  #endif

  // OpenGL
  #ifdef USEGL
    void glVertex() {
      glVertex2f(GLfloat(x),GLfloat(y));
    }
  #endif
};


template <class T> VECTOR2<T> operator + ( T scalar, const VECTOR2<T>& vec ) {return VECTOR2<T>(scalar+vec.x,scalar+vec.y);}
template <class T> VECTOR2<T> operator - ( T scalar, const VECTOR2<T>& vec ) {return VECTOR2<T>(scalar-vec.x,scalar-vec.y);}
template <class T> VECTOR2<T> operator * ( T scalar, const VECTOR2<T>& vec ) {return VECTOR2<T>(scalar*vec.x,scalar*vec.y);}
template <class T> VECTOR2<T> operator / ( T scalar, const VECTOR2<T>& vec ) {return VECTOR2<T>(scalar/vec.x,scalar/vec.y);}
template <class T> VECTOR2<T> operator % ( T scalar, const VECTOR2<T>& vec ) {return VECTOR2<T>(scalar%vec.x,scalar%vec.y);}


template <class T=int> class VECTOR3 {
public:
  T x,y,z;

  VECTOR3<T>(): x(0), y(0),z(0) {}
  template <class S> explicit VECTOR3<T>( const std::vector<S>& v ) {
    x = T(v.size()>0 ? v[0] : 0);
    y = T(v.size()>1 ? v[1] : 0);
    z = T(v.size()>2 ? v[2] : 0);
  }

  VECTOR3<T>(const VECTOR3<T> &other): x(other.x), y(other.y), z(other.z) {}
  template <class S> explicit VECTOR3<T>(const VECTOR3<S> &other): x(T(other.x)), y(T(other.y)), z(T(other.z)) {}
  VECTOR3<T>(const VECTOR2<T> &other, const T _z): x(other.x), y(other.y), z(_z) {}
  VECTOR3<T>(const T _x, const T _y, const T _z) : x(_x), y(_y), z(_z) {}
  VECTOR3<T>(const T* vec) : x(vec[0]), y(vec[1]), z(vec[2]) {}


  bool operator == ( const VECTOR3<T>& other ) const {return (other.x==x && other.y==y && other.z==z); }
  bool operator != ( const VECTOR3<T>& other ) const {return (other.x!=x || other.y!=y || other.z!=z); }

    // binary operators with scalars
  VECTOR3<T> operator + ( T scalar ) const {return VECTOR3<T>(x+scalar,y+scalar,z+scalar);}
  VECTOR3<T> operator - ( T scalar ) const {return VECTOR3<T>(x-scalar,y-scalar,z-scalar);}
  VECTOR3<T> operator * ( T scalar ) const {return VECTOR3<T>(x*scalar,y*scalar,z*scalar);}
  VECTOR3<T> operator / ( T scalar ) const {return VECTOR3<T>(x/scalar,y/scalar,z/scalar);}
  VECTOR3<T> operator % ( T scalar ) const {return VECTOR3<T>(x%scalar,y%scalar,z%scalar);}

  // binary operators with vectors
  VECTOR3<T> operator + ( const VECTOR3<T>& other ) const {return VECTOR3<T>(x+other.x,y+other.y,z+other.z);}
  VECTOR3<T> operator - ( const VECTOR3<T>& other ) const {return VECTOR3<T>(x-other.x,y-other.y,z-other.z);}
  VECTOR3<T> operator / ( const VECTOR3<T>& other ) const {return VECTOR3<T>(x/other.x,y/other.y,z/other.z);}
  VECTOR3<T> operator * ( const VECTOR3<T>& other ) const {return VECTOR3<T>(x*other.x,y*other.y,z*other.z);} // component product
  VECTOR3<T> operator % ( const VECTOR3<T>& other ) const {return VECTOR3<T>(y*other.z-z*other.y,z*other.x-x*other.z,x*other.y-y*other.x);} // cross product
  T operator ^ ( const VECTOR3<T>& other ) const {return T(x*other.x+y*other.y+z*other.z);} // dot product

  // unary opartors
  VECTOR3<T> operator + () const {return *this;}
  VECTOR3<T> operator - () const {return *this * -1;}
  VECTOR3<T> operator ~ () const {return VECTOR3<T>(T(1)/x,T(1)/y,T(1)/z);}

  // binary operators with a matrix
  VECTOR3<T> operator * ( const MATRIX3<T>& matrix ) const {
    return VECTOR3<T>(x*matrix.m11+y*matrix.m21+z*matrix.m31,
                      x*matrix.m12+y*matrix.m22+z*matrix.m32,
                      x*matrix.m13+y*matrix.m23+z*matrix.m33);
  }

  VECTOR3<T>& operator=(const VECTOR3<T>& other)  { x = other.x; y = other.y; z = other.z; return *this; }
  VECTOR3<T>& operator+=(const VECTOR3<T>& other) { x += other.x; y += other.y; z += other.z; return *this; }
  VECTOR3<T>& operator-=(const VECTOR3<T>& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
  VECTOR3<T>& operator*=(const VECTOR3<T>& other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
  VECTOR3<T>& operator/=(const VECTOR3<T>& other) { x /= other.x; y /= other.y; z /= other.z; return *this; }
  VECTOR3<T>& operator=(const T& other) { x = other; y = other; z = other; return *this; }
  VECTOR3<T>& operator+=(const T& other) { x += other; y += other; z += other; return *this; }
  VECTOR3<T>& operator-=(const T& other) { x -= other; y -= other; z -= other; return *this; }
  VECTOR3<T>& operator*=(const T& other) { x *= other; y *= other; z *= other; return *this; }
  VECTOR3<T>& operator/=(const T& other) { x /= other; y /= other; z /= other; return *this; }

  friend std::ostream& operator<<(std::ostream &os,const VECTOR3<T>& v){os << v.x << '\t' << v.y << '\t' << v.z; return os;}

  T& operator [](size_t i) {
    assert(i <= 2);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      default: return this->z;
    }
  }
  const T& operator [](size_t i) const {
    assert(i <= 2);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      default: return this->z;
    }
  }

  VECTOR3<T> abs() const {return VECTOR3<T>(fabs(x),fabs(y),fabs(z));}
  T maxVal() const {return MAX(x,MAX(y,z));}
  T minVal() const {return MIN(x,MIN(y,z));}
  T volume() const {return x*y*z;}
  T length() const {return sqrt(T(x*x+y*y+z*z));}
  void normalize() {T len = length(); x/=len;y/=len;z/=len;}
  void normalize(T epsilon, const VECTOR3<T> replacement=VECTOR3<T>(T(0),T(0),T(1))) {
    T len = length();
    if (len > epsilon) {
      x/=len;
      y/=len;
      z/=len;
    } else { // specify some arbitrary normal
      x = replacement.x;
      y = replacement.y;
      z = replacement.z;
    }
  }

  VECTOR2<T> xx() const {return VECTOR2<T>(x,x);}
  VECTOR2<T> xy() const {return VECTOR2<T>(x,y);}
  VECTOR2<T> xz() const {return VECTOR2<T>(x,z);}
  VECTOR2<T> yx() const {return VECTOR2<T>(y,x);}
  VECTOR2<T> yy() const {return VECTOR2<T>(y,y);}
  VECTOR2<T> yz() const {return VECTOR2<T>(y,z);}
  VECTOR2<T> zx() const {return VECTOR2<T>(z,x);}
  VECTOR2<T> zy() const {return VECTOR2<T>(z,y);}
  VECTOR2<T> zz() const {return VECTOR2<T>(z,z);}

  VECTOR3<T> makepow2() const  {
    VECTOR3<T> vOut;
    vOut.x = T(1<<int(ceil(log(double(x))/log(2.0))));
    vOut.y = T(1<<int(ceil(log(double(y))/log(2.0))));
    vOut.z = T(1<<int(ceil(log(double(z))/log(2.0))));
    return vOut;
  }

  void StoreMin(const VECTOR3<T> &other) {
    x = std::min(x,other.x);
    y = std::min(y,other.y);
    z = std::min(z,other.z);
  }

  void StoreMax(const VECTOR3<T> &other) {
    x = std::max(x,other.x);
    y = std::max(y,other.y);
    z = std::max(z,other.z);
  }

  #ifdef USEDX
    VECTOR3(const D3DXVECTOR3 &other): x(T(other.x)), y(T(other.y)), z(T(other.z)) {}
    VECTOR3(const D3DXVECTOR4 &other): x(T(other.x)), y(T(other.y)), z(T(other.z)) {}
    D3DXVECTOR3 toD3DXVEC() const {return D3DXVECTOR3(float(x),float(y),float(z));}
    bool operator == ( const D3DXVECTOR3& other ) const {return (other.x==T(x) && other.y== T(y) && other.z== T(z)); }
    bool operator != ( const D3DXVECTOR3& other ) const {return (other.x!=T(x) || other.y!= T(y) || other.z!= T(z)); }
    operator D3DXVECTOR3(void) const {return toD3DXVEC();}
  #endif

  // OpenGL
  #ifdef USEGL
    void glVertex() {
      glVertex3f(GLfloat(x),GLfloat(y),GLfloat(z));
    }
    void glNormal() {
      glNormal3f(GLfloat(x),GLfloat(y),GLfloat(z));
    }
    void glNNormal() {
      normalize();
      glNormal3f(GLfloat(x),GLfloat(y),GLfloat(z));
    }
  #endif

};

template <class T> VECTOR3<T> operator + ( T scalar, const VECTOR3<T>& vec ) {return VECTOR3<T>(scalar+vec.x,scalar+vec.y,scalar+vec.z);}
template <class T> VECTOR3<T> operator - ( T scalar, const VECTOR3<T>& vec ) {return VECTOR3<T>(scalar-vec.x,scalar-vec.y,scalar-vec.z);}
template <class T> VECTOR3<T> operator * ( T scalar, const VECTOR3<T>& vec ) {return VECTOR3<T>(scalar*vec.x,scalar*vec.y,scalar*vec.z);}
template <class T> VECTOR3<T> operator / ( T scalar, const VECTOR3<T>& vec ) {return VECTOR3<T>(scalar/vec.x,scalar/vec.y,scalar/vec.z);}
template <class T> VECTOR3<T> operator % ( T scalar, const VECTOR3<T>& vec ) {return VECTOR3<T>(scalar%vec.x,scalar%vec.y,scalar%vec.z);}

template <class T=int> class VECTOR4 {
  template <class U>
  friend std::istream& operator >>(std::istream &, VECTOR4<U>&);
public:
  T x,y,z,w;

  VECTOR4<T>(): x(0), y(0),z(0), w(0) {}
  template <class S> explicit VECTOR4<T>( const std::vector<S>& v ) {
    x = T(v.size()>0 ? v[0] : 0);
    y = T(v.size()>1 ? v[1] : 0);
    z = T(v.size()>2 ? v[2] : 0);
    w = T(v.size()>3 ? v[3] : 0);
  }
  VECTOR4<T>(const VECTOR2<T> &other, const T _z, const T _w): x(other.x), y(other.y), z(_z), w(_w) {}
  VECTOR4<T>(const VECTOR3<T> &other, const T _w): x(other.x), y(other.y), z(other.z), w(_w) {}
  VECTOR4<T>(const VECTOR4<T> &other): x(other.x), y(other.y), z(other.z), w(other.w) {}
  template <class S> explicit VECTOR4<T>(const VECTOR4<S> &other): x(T(other.x)), y(T(other.y)), z(T(other.z)), w(T(other.w)) {}
  VECTOR4<T>(const VECTOR3<T> &other): x(other.x), y(other.y), z(other.z), w(1) {}
  VECTOR4<T>(const T _x, const T _y, const T _z, const T _w) : x(_x), y(_y), z(_z), w(_w) {}
  VECTOR4<T>(const T* vec) : x(vec[0]), y(vec[1]), z(vec[2]), w(vec[3]) {}

  bool operator == ( const VECTOR4<T>& other ) const {return (other.x==x && other.y==y && other.z==z && other.w==w); }
  bool operator != ( const VECTOR4<T>& other ) const {return (other.x!=x || other.y!=y || other.z!=z || other.w!=w); }

    // binary operators with scalars
  VECTOR4<T> operator + ( T scalar ) const {return VECTOR4<T>(x+scalar,y+scalar,z+scalar,w+scalar);}
  VECTOR4<T> operator - ( T scalar ) const {return VECTOR4<T>(x-scalar,y-scalar,z-scalar,w-scalar);}
  VECTOR4<T> operator * ( T scalar ) const {return VECTOR4<T>(x*scalar,y*scalar,z*scalar,w*scalar);}
  VECTOR4<T> operator / ( T scalar ) const {return VECTOR4<T>(x/scalar,y/scalar,z/scalar,w/scalar);}
  VECTOR4<T> operator % ( T scalar ) const {return VECTOR4<T>(x%scalar,y%scalar,z%scalar,w%scalar);}

  // binary operators with vectors
  VECTOR4<T> operator + ( const VECTOR4<T>& other ) const {return VECTOR4<T>(x+other.x,y+other.y,z+other.z,w+other.w);}
  VECTOR4<T> operator - ( const VECTOR4<T>& other ) const {return VECTOR4<T>(x-other.x,y-other.y,z-other.z,w-other.w);}
  VECTOR4<T> operator * ( const VECTOR4<T>& other ) const {return VECTOR4<T>(x*other.x,y*other.y,z*other.z,w*other.w);}
  VECTOR4<T> operator / ( const VECTOR4<T>& other ) const {return VECTOR4<T>(x/other.x,y/other.y,z/other.z,w/other.w);}
  T operator ^ ( const VECTOR4<T>& other ) const {return T(x*other.x+y*other.y+z*other.z+w*other.w);} // dot product

  // binary operators with a matrix
  VECTOR4<T> operator * ( const MATRIX4<T>& matrix ) const {
    return VECTOR4<T>(x*matrix.m11+y*matrix.m21+z*matrix.m31+w*matrix.m41,
                      x*matrix.m12+y*matrix.m22+z*matrix.m32+w*matrix.m42,
                      x*matrix.m13+y*matrix.m23+z*matrix.m33+w*matrix.m43,
                      x*matrix.m14+y*matrix.m24+z*matrix.m34+w*matrix.m44);
  }

  // unary opartors
  VECTOR4<T> operator + () const {return *this;}
  VECTOR4<T> operator - () const {return *this * -1;}
  VECTOR4<T> operator ~ () const {return VECTOR4<T>(T(1)/x,T(1)/y,T(1)/z,T(1)/w);}

  T& operator [](size_t i) {
    assert(i <= 3);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      case 2: return this->z;
      default: return this->w;
    }
  }
  const T& operator [](size_t i) const {
    assert(i <= 3);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      case 2: return this->z;
      default: return this->w;
    }
  }

  VECTOR4<T>& operator=(const VECTOR4<T>& other)  { x = other.x; y = other.y; z = other.z; w = other.w; return *this; }
  VECTOR4<T>& operator+=(const VECTOR4<T>& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
  VECTOR4<T>& operator-=(const VECTOR4<T>& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
  VECTOR4<T>& operator*=(const VECTOR4<T>& other) { x *= other.x; y *= other.y; z *= other.z; w *= other.w; return *this; }
  VECTOR4<T>& operator/=(const VECTOR4<T>& other) { x /= other.x; y /= other.y; z /= other.z; w /= other.w; return *this; }
  VECTOR4<T>& operator=(const T& other)  { x = other; y = other; z = other; w = other; return *this; }
  VECTOR4<T>& operator+=(const T& other) { x += other; y += other; z += other;  w += other; return *this; }
  VECTOR4<T>& operator-=(const T& other) { x -= other; y -= other; z -= other;  w -= other; return *this; }
  VECTOR4<T>& operator*=(const T& other) { x *= other; y *= other; z *= other;  w *= other; return *this; }
  VECTOR4<T>& operator/=(const T& other) { x /= other; y /= other; z /= other;  w /= other; return *this; }

  friend std::ostream& operator<<(std::ostream &os,const VECTOR4<T>& v) {
    os << "[" << std::setiosflags(std::ios::left)
       << std::setw(7) << std::setprecision(3) << v.x
       << std::resetiosflags(std::ios::left)
       << std::setw(7) << std::setprecision(3) << v.y
       << std::setw(7) << std::setprecision(3) << v.z
       << std::setw(7) << std::setprecision(3) << v.w
       << "]";
    return os;
  }


  T max() const {return MAX(MAX(x,y),MAX(z,w));}
  T min() const {return MIN(MIN(x,y),MIN(z,w));}

  VECTOR4<T> abs() const {return VECTOR4<T>(fabs(x),fabs(y),fabs(z),fabs(w));}

  VECTOR4<T> makepow2() const  {
    VECTOR4<T> vOut;
    vOut.x = T(1<<int(ceil(log(double(x))/log(2.0))));
    vOut.y = T(1<<int(ceil(log(double(y))/log(2.0))));
    vOut.z = T(1<<int(ceil(log(double(z))/log(2.0))));
    vOut.w = T(1<<int(ceil(log(double(w))/log(2.0))));
    return vOut;
  }

  VECTOR3<T> dehomo() const  {
    VECTOR3<T> vOut;
    vOut.x = x/w;
    vOut.y = y/w;
    vOut.z = z/w;
    return vOut;
  }

  void StoreMin(const VECTOR4<T> &other) {
    x = std::min(x,other.x);
    y = std::min(y,other.y);
    z = std::min(z,other.z);
    w = std::min(w,other.w);
  }

  void StoreMax(const VECTOR4<T> &other) {
    x = std::max(x,other.x);
    y = std::max(y,other.y);
    z = std::max(z,other.z);
    w = std::max(w,other.w);
  }

  VECTOR2<T> xx() const {return VECTOR2<T>(x,x);}
  VECTOR2<T> xy() const {return VECTOR2<T>(x,y);}
  VECTOR2<T> xz() const {return VECTOR2<T>(x,z);}
  VECTOR2<T> yx() const {return VECTOR2<T>(y,x);}
  VECTOR2<T> yy() const {return VECTOR2<T>(y,y);}
  VECTOR2<T> yz() const {return VECTOR2<T>(y,z);}
  VECTOR2<T> zx() const {return VECTOR2<T>(z,x);}
  VECTOR2<T> zy() const {return VECTOR2<T>(z,y);}
  VECTOR2<T> zz() const {return VECTOR2<T>(z,z);}
  VECTOR2<T> zw() const {return VECTOR2<T>(z,w);}

  VECTOR3<T> xyz() const {return VECTOR3<T>(x,y,z);}

  // DirectX
  #ifdef USEDX
    VECTOR4<T>(const D3DXVECTOR4 &other): x(T(other.x)), y(T(other.y)), z(T(other.z)), w(T(other.w)){}
    D3DXVECTOR4 toD3DXVEC() const {return D3DXVECTOR4(float(x),float(y),float(z),float(w));}

    bool operator == ( const D3DXVECTOR4& other ) const {return (other.x==T(x) && other.y==T(y) && other.z==T(z) && other.w==T(w)); }
    bool operator != ( const D3DXVECTOR4& other ) const {return (other.x!=T(x) || other.y!=T(y) || other.z!=T(z) || other.w!=T(w)); }
    operator D3DXVECTOR4(void) const {return toD3DXVEC();}
  #endif

  // OpenGL
  #ifdef USEGL
    void glVertex() {
      glVertex4f(GLfloat(x),GLfloat(y),GLfloat(z),GLfloat(w));
    }
    void glNormal() {
      dehomo().glNormal();
    }
  #endif
};

template <class T>
std::istream& operator >>(std::istream &is, VECTOR4<T>& v4)
{
  is >> v4[0];
  is >> v4[1];
  is >> v4[2];
  is >> v4[3];
  return is;
}

template <class T> VECTOR4<T> operator + ( T scalar, const VECTOR4<T>& vec ) {return VECTOR4<T>(scalar+vec.x,scalar+vec.y,scalar+vec.z,scalar+vec.w);}
template <class T> VECTOR4<T> operator - ( T scalar, const VECTOR4<T>& vec ) {return VECTOR4<T>(scalar-vec.x,scalar-vec.y,scalar-vec.z,scalar-vec.w);}
template <class T> VECTOR4<T> operator * ( T scalar, const VECTOR4<T>& vec ) {return VECTOR4<T>(scalar*vec.x,scalar*vec.y,scalar*vec.z,scalar*vec.w);}
template <class T> VECTOR4<T> operator / ( T scalar, const VECTOR4<T>& vec ) {return VECTOR4<T>(scalar/vec.x,scalar/vec.y,scalar/vec.z,scalar/vec.w);}
template <class T> VECTOR4<T> operator % ( T scalar, const VECTOR4<T>& vec ) {return VECTOR4<T>(scalar%vec.x,scalar%vec.y,scalar%vec.z,scalar%vec.w);}

typedef VECTOR4<> INTVECTOR4;
typedef VECTOR3<> INTVECTOR3;
typedef VECTOR2<> INTVECTOR2;

typedef VECTOR4<UINT32> UINTVECTOR4;
typedef VECTOR3<UINT32> UINTVECTOR3;
typedef VECTOR2<UINT32> UINTVECTOR2;

typedef VECTOR3<UINT64> UINT64VECTOR3;

typedef VECTOR4<unsigned short> USHORTVECTOR4;
typedef VECTOR3<unsigned short> USHORTVECTOR3;
typedef VECTOR2<unsigned short> USHORTVECTOR2;

typedef VECTOR4<float> FLOATVECTOR4;
typedef VECTOR3<float> FLOATVECTOR3;
typedef VECTOR2<float> FLOATVECTOR2;

typedef VECTOR4<double> DOUBLEVECTOR4;
typedef VECTOR3<double> DOUBLEVECTOR3;
typedef VECTOR2<double> DOUBLEVECTOR2;

template <class T=int> class MATRIX2 {
public:
  union {
    struct {
      T  m11,m12,
        m21,m22;
    };
    T array[4];
  };

  MATRIX2() : m11(1), m12(0),
        m21(0), m22(1) {};
  MATRIX2( const T *e ) : m11(e[0]),  m12(e[1]),
              m21(e[4]),  m22(e[5])  {};
    MATRIX2( const MATRIX2<T>& other ) : m11(other.m11), m12(other.m12),
                                  m21(other.m21), m22(other.m22) {};
    MATRIX2( const MATRIX3<T>& other ) : m11(other.m11), m12(other.m12),
                                  m21(other.m21), m22(other.m22) {};
    MATRIX2( const MATRIX4<T>& other ) : m11(other.m11), m12(other.m12),
                                  m21(other.m21), m22(other.m22) {};
  MATRIX2( const VECTOR2<T> *rows ) : m11(rows[0].x), m12(rows[0].y),
                    m21(rows[1].x), m22(rows[1].y) {};
    MATRIX2(T _m11, T _m12, T _m21, T _m22) : m11(_m11), m12(_m12),
                        m21(_m21), m22(_m22) {};

  bool operator == ( const MATRIX2<T>& other ) const {return (other.m11==m11 && other.m12==m12 && other.m21==m21 && other.m22==m22); }
  bool operator != ( const MATRIX2<T>& other ) const {return (other.m11!=m11 || other.m12!=m12 || other.m21!=m21 || other.m22!=m22); }

  friend std::ostream& operator<<(std::ostream &os,const MATRIX2<T>& m){os << m.m11 << '\t' << m.m12 << '\n' << m.m21 << '\t' << m.m22; return os;}

  // binary operators with scalars
  MATRIX2<T> operator * ( T scalar ) const {return MATRIX2<T>(m11*scalar,m12*scalar,
                                m21*scalar,m22*scalar);}
  MATRIX2<T> operator + ( T scalar ) const {return MATRIX2<T>(m11+scalar,m12+scalar,
                                m21+scalar,m22+scalar);}
  MATRIX2<T> operator - ( T scalar ) const {return MATRIX2<T>(m11-scalar,m12-scalar,
                                m21-scalar,m22-scalar);}
  MATRIX2<T> operator / ( T scalar ) const {return MATRIX2<T>(m11/scalar,m12/scalar,
                                m21/scalar,m22/scalar);}


  // binary operators with matrices
  MATRIX2<T> operator * ( const MATRIX4<T>& other ) const {
    MATRIX2<T> result;

    for (int x = 0;x<4;x+=2)
      for (int y = 0;y<2;y++)
        result[x+y] = array[0+x]*other.array[0+y] + array[1+x]*other.array[2+y];

    return result;
  }

  // binary operators with vectors
  VECTOR2<T> operator * ( const VECTOR2<T>& other ) const {
    return VECTOR2<T>(other.x*m11+other.y*m12,
                      other.x*m22+other.y*m22);
  }

  MATRIX2<T> inverse() const {
    T determinant = m11*m22-m12*m21;
    return MATRIX2<T> (m22/determinant, -m12/determinant,-m21/determinant, m11/determinant);
  }
};

template <class T> class MATRIX3 {
public:
  union {
    struct {
      T  m11,m12,m13,
        m21,m22,m23,
        m31,m32,m33;
    };
    T array[9];
  };

    MATRIX3() : m11(1), m12(0), m13(0),
        m21(0), m22(1), m23(0),
        m31(0), m32(0), m33(1) {};
    MATRIX3( const T *e ) : m11(e[0]), m12(e[1]), m13(e[2]),
              m21(e[3]), m22(e[4]), m23(e[5]),
              m31(e[6]), m32(e[7]), m33(e[8]) {};
    MATRIX3( const MATRIX3<T>& other ) : m11(other.m11), m12(other.m12), m13(other.m13),
                                  m21(other.m21), m22(other.m22), m23(other.m23),
                    m31(other.m31), m32(other.m32), m33(other.m33) {};
    MATRIX3( const MATRIX4<T>& other ) : m11(other.m11), m12(other.m12), m13(other.m13),
                                  m21(other.m21), m22(other.m22), m23(other.m23),
                    m31(other.m31), m32(other.m32), m33(other.m33) {};
    MATRIX3( const VECTOR3<T> *rows ) : m11(rows[0].x), m12(rows[0].y), m13(rows[0].z),
                    m21(rows[1].x), m22(rows[1].y), m23(rows[1].z),
                    m31(rows[2].x), m32(rows[2].y), m33(rows[2].z) {};
    MATRIX3(T _m11, T _m12, T _m13,
      T _m21, T _m22, T _m23,
      T _m31, T _m32, T _m33) : m11(_m11), m12(_m12), m13(_m13),
                    m21(_m21), m22(_m22), m23(_m23),
                    m31(_m31), m32(_m32), m33(_m33) {};

  bool operator == ( const MATRIX3<T>& other ) const {return (other.m11==m11 && other.m12==m12 && other.m13==m13 &&
                                other.m21==m21 && other.m22==m22 && other.m23==m23 &&
                                other.m31==m31 && other.m32==m32 && other.m33==m33); }
  bool operator != ( const MATRIX3<T>& other ) const {return (other.m11!=m11 || other.m12!=m12 || other.m13!=m13 ||
                                other.m21!=m21 || other.m22!=m22 || other.m23!=m23 ||
                                other.m31!=m31 || other.m32!=m32 || other.m33!=m33); }


  friend std::ostream& operator<<(std::ostream &os,const MATRIX3<T>& m){os << m.m11 << '\t' << m.m12 << '\t' << m.m13 << '\n'
                                       << m.m21 << '\t' << m.m22 << '\t' << m.m23 << '\n'
                                       << m.m31 << '\t' << m.m32 << '\t' << m.m33; return os;}

  // binary operators with matrices
  MATRIX3<T> operator * ( const MATRIX3<T>& other ) const {
    MATRIX3<T> result;
    for (int x = 0;x<9;x+=3)
      for (int y = 0;y<3;y++)
        result[x+y] = array[1+x]  * other.array[0+y]+
                      array[2+x]  * other.array[3+y]+
                      array[3+x]  * other.array[6+y];

    return result;
  }

  // binary operators with scalars
  MATRIX3<T> operator * ( T scalar ) const {return MATRIX3<T>(m11*scalar,m12*scalar,m13*scalar,
                                m21*scalar,m22*scalar,m23*scalar,
                                m31*scalar,m32*scalar,m33*scalar);}
  MATRIX3<T> operator + ( T scalar ) const {return MATRIX3<T>(m11+scalar,m12+scalar,m13+scalar,
                                m21+scalar,m22+scalar,m23+scalar,
                                m31+scalar,m32+scalar,m33+scalar);}
  MATRIX3<T> operator - ( T scalar ) const {return MATRIX3<T>(m11-scalar,m12-scalar,m13-scalar,
                                m21-scalar,m22-scalar,m23-scalar,
                                m31-scalar,m32-scalar,m33-scalar);}
  MATRIX3<T> operator / ( T scalar ) const {return MATRIX3<T>(m11/scalar,m12/scalar,m13/scalar,
                                m21/scalar,m22/scalar,m23/scalar,
                                m31/scalar,m32/scalar,m33/scalar);}

  // binary operators with vectors
  VECTOR3<T> operator * ( const VECTOR3<T>& other ) const {
    return VECTOR3<T>(other.x*m11+other.y*m12+other.z*m13,
                      other.x*m21+other.y*m22+other.z*m23,
                      other.x*m31+other.y*m32+other.z*m33);
  }

  void Scaling(T x, T y, T z) {
    m11=x; m12=0; m13=0;
    m21=0; m22=y; m23=0;
    m31=0; m32=0; m33=z;
  }

  void RotationX(double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=1;    m12=0;      m13=0;
    m21=0;    m22= dCosAngle;  m23=dSinAngle;
    m31=0;    m32=-dSinAngle;  m33=dCosAngle;
  }

  void RotationY(double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=dCosAngle;  m12=0;  m13=-dSinAngle;
    m21=0;      m22=1;  m23=0;
    m31=dSinAngle;  m32=0;  m33=dCosAngle;
  }

  void RotationZ(double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=dCosAngle;  m12=dSinAngle;  m13=0;
    m21=-dSinAngle;  m22=dCosAngle;  m23=0;
    m31=0;      m32=0;      m33=1;
  }

  void RotationAxis(const VECTOR3<T>& axis, double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));
    T dOneMinusCosAngle = 1-dCosAngle;

    VECTOR3<T> sqrAxis   = axis * axis;

    m11 = dCosAngle+dOneMinusCosAngle*sqrAxis.x;        m12 = dOneMinusCosAngle*axis.x*axis.y-dSinAngle*axis.z;  m13 = dOneMinusCosAngle*axis.x*axis.z+dSinAngle*axis.y;
    m21 = dOneMinusCosAngle*axis.x*axis.y+dSinAngle*axis.z;    m22 = dCosAngle+dOneMinusCosAngle*sqrAxis.y;      m23 = dOneMinusCosAngle*axis.y*axis.z-dSinAngle*axis.x;
    m31 = dOneMinusCosAngle*axis.x*axis.z-dSinAngle*axis.y;    m32 = dOneMinusCosAngle*axis.y*axis.z+dSinAngle*axis.x;  m33=dCosAngle+dOneMinusCosAngle*sqrAxis.z;
  }

  MATRIX3<T> inverse() const {
    T determ = 1.0f/(array[0]*(array[4]*array[8]-array[5]*array[7]) - array[1]*(array[3]*array[8]-array[5]*array[6]) + array[2]*(array[3]*array[7]-array[4]*array[6]));

    MATRIX3<T> result;

    result.array[0]=determ*(array[4]*array[8]-array[5]*array[7]);
    result.array[3]=determ*(array[2]*array[7]-array[1]*array[8]);
    result.array[6]=determ*(array[1]*array[5]-array[2]*array[4]);
    result.array[1]=determ*(array[5]*array[6]-array[3]*array[8]);
    result.array[4]=determ*(array[0]*array[8]-array[2]*array[6]);
    result.array[7]=determ*(array[2]*array[3]-array[0]*array[5]);
    result.array[2]=determ*(array[3]*array[7]-array[4]*array[6]);
    result.array[5]=determ*(array[1]*array[6]-array[0]*array[7]);
    result.array[8]=determ*(array[0]*array[4]-array[1]*array[3]);

    return result;
  }

  // DirectX
  #ifdef USEDX
    MATRIX3( const D3DXMATRIX& other ) : m11(other(0,0)), m12(other(0,1)), m13(other(0,2)),
                       m21(other(1,0)), m22(other(1,1)), m23(other(1,2)),
                       m31(other(2,0)), m32(other(2,1)), m33(other(2,2)) {};
    D3DXMATRIX toD3DXMAT() const {return D3DXMATRIX(m11,m12,m13,0,m21,m22,m23,0,m31,m32,m33,0,0,0,0,1);}

    operator D3DXMATRIX(void) const {return toD3DXMAT();}
  #endif

  // OpenGL
  #ifdef USEGL
    void getProjection() {
      float P[16];
      glGetFloatv(GL_PROJECTION_MATRIX,P);
      m11 = T(P[0]); m12 = T(P[1]); m13 = T(P[2]);
      m21 = T(P[4]); m22 = T(P[5]); m23 = T(P[6]);
      m31 = T(P[8]); m32 = T(P[9]); m33 = T(P[10]);
    }

    void getModelview() {
      float M[16];
      glGetFloatv(GL_MODELVIEW_MATRIX,M);
      m11 = T(M[0]); m12 = T(M[1]); m13 = T(M[2]);
      m21 = T(M[4]); m22 = T(M[5]); m23 = T(M[6]);
      m31 = T(M[8]); m32 = T(M[9]); m33 = T(M[10]);
    }

    void multModelview() {
      float M[16];
       M[0] = float(m11);  M[1] = float(m12);  M[2] = float(m13);  M[3] = 0;
       M[4] = float(m21);  M[5] = float(m22);  M[6] = float(m23);  M[7] = 0;
       M[8] = float(m31);  M[9] = float(m32); M[10] = float(m33); M[11] = 0;
      M[12] = 0; M[13] = 0; M[14] = 0; M[15] =1;
      glMatrixMode(GL_MODELVIEW);
      glMultMatrixf(M);
    }

    void setModelview() {
      float M[16];
      M[0] = float(m11);  M[1] = float(m12);  M[2] = float(m13);  M[3] = 0;
      M[4] = float(m21);  M[5] = float(m22);  M[6] = float(m23);  M[7] = 0;
      M[8] = float(m31);  M[9] = float(m32); M[10] = float(m33); M[11] = 0;
      M[12] = 0; M[13] = 0; M[14] = 0; M[15] =1;
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(M);
    }
  #endif

};

template <class T> class MATRIX4 {
public:
  union {
    struct {
      T  m11,m12,m13,m14,
        m21,m22,m23,m24,
        m31,m32,m33,m34,
        m41,m42,m43,m44;
    };
    T array[16];
  };

  MATRIX4() : m11(1), m12(0), m13(0), m14(0),
              m21(0), m22(1), m23(0), m24(0),
              m31(0), m32(0), m33(1), m34(0),
              m41(0), m42(0), m43(0), m44(1) {};
  MATRIX4( const T *e ) : m11(e[0]),  m12(e[1]),  m13(e[2]),  m14(e[3]),
              m21(e[4]),  m22(e[5]),  m23(e[6]),  m24(e[7]),
              m31(e[8]),  m32(e[9]),  m33(e[10]), m34(e[11]),
              m41(e[12]), m42(e[13]), m43(e[14]), m44(e[15])  {};
  MATRIX4( const MATRIX4<T>& other ) : m11(other.m11), m12(other.m12), m13(other.m13), m14(other.m14),
                                       m21(other.m21), m22(other.m22), m23(other.m23), m24(other.m24),
                                       m31(other.m31), m32(other.m32), m33(other.m33), m34(other.m34),
                                       m41(other.m41), m42(other.m42), m43(other.m43), m44(other.m44) {};
  MATRIX4( const MATRIX3<T>& other ) : m11(other.m11), m12(other.m12), m13(other.m13), m14(0),
                                       m21(other.m21), m22(other.m22), m23(other.m23), m24(0),
                                       m31(other.m31), m32(other.m32), m33(other.m33), m34(0),
                                       m41(0), m42(0), m43(0), m44(1) {};
  MATRIX4( const VECTOR4<T> *rows ) : m11(rows[0].x), m12(rows[0].y), m13(rows[0].z), m14(rows[0].w),
                                      m21(rows[1].x), m22(rows[1].y), m23(rows[1].z), m24(rows[1].w),
                                      m31(rows[2].x), m32(rows[2].y), m33(rows[2].z), m34(rows[2].w),
                                      m41(rows[3].x), m42(rows[3].y), m43(rows[3].z), m44(rows[3].w) {};
  MATRIX4(T _m11, T _m12, T _m13, T _m14,
          T _m21, T _m22, T _m23, T _m24,
          T _m31, T _m32, T _m33, T _m34,
          T _m41, T _m42, T _m43, T _m44) :
      m11(_m11), m12(_m12), m13(_m13), m14(_m14),
      m21(_m21), m22(_m22), m23(_m23), m24(_m24),
      m31(_m31), m32(_m32), m33(_m33), m34(_m34),
      m41(_m41), m42(_m42), m43(_m43), m44(_m44) {};

  bool operator == ( const MATRIX4<T>& other ) const {return (other.m11==m11 && other.m12==m12 && other.m13==m13 && other.m14==m14 &&
                                other.m21==m21 && other.m22==m22 && other.m23==m23 && other.m24==m24 &&
                                other.m31==m31 && other.m32==m32 && other.m33==m33 && other.m34==m34 &&
                                other.m31==m41 && other.m32==m42 && other.m33==m43 && other.m44==m44); }
  bool operator != ( const MATRIX4<T>& other ) const {return (other.m11!=m11 || other.m12!=m12 || other.m13!=m13 || other.m14!=m14 ||
                                other.m21!=m21 || other.m22!=m22 || other.m23!=m23 || other.m24!=m24 ||
                                other.m31!=m31 || other.m32!=m32 || other.m33!=m33 || other.m34!=m34 ||
                                other.m41!=m41 || other.m42!=m42 || other.m43!=m43 || other.m44!=m44); }

  friend std::ostream& operator<<(std::ostream &os,const MATRIX4<T>& m){os << m.m11 << '\t' << m.m12 << '\t' << m.m13 << '\t' << m.m14 << '\n'
                                       << m.m21 << '\t' << m.m22 << '\t' << m.m23 << '\t' << m.m24 << '\n'
                                       << m.m31 << '\t' << m.m32 << '\t' << m.m33 << '\t' << m.m34 << '\n'
                                       << m.m41 << '\t' << m.m42 << '\t' << m.m43 << '\t' << m.m44 ; return os;}

  operator T*(void) {return &m11;}
  const T *operator *(void) const {return &m11;}
  T *operator *(void) {return &m11;}

  // binary operators with scalars
  MATRIX4<T> operator * ( T scalar ) const {return MATRIX4<T>(m11*scalar,m12*scalar,m13*scalar,m14*scalar,
                                m21*scalar,m22*scalar,m23*scalar,m24*scalar,
                                m31*scalar,m32*scalar,m33*scalar,m34*scalar,
                                m41*scalar,m42*scalar,m43*scalar,m44*scalar);}
  MATRIX4<T> operator + ( T scalar ) const {return MATRIX4<T>(m11+scalar,m12+scalar,m13+scalar,m14+scalar,
                                m21+scalar,m22+scalar,m23+scalar,m24+scalar,
                                m31+scalar,m32+scalar,m33+scalar,m34+scalar,
                                m41+scalar,m42+scalar,m43+scalar,m44+scalar);}
  MATRIX4<T> operator - ( T scalar ) const {return MATRIX4<T>(m11-scalar,m12-scalar,m13-scalar,m14-scalar,
                                m21-scalar,m22-scalar,m23-scalar,m24-scalar,
                                m31-scalar,m32-scalar,m33-scalar,m34-scalar,
                                m41-scalar,m42-scalar,m43-scalar,m44-scalar);}
  MATRIX4<T> operator / ( T scalar ) const {return MATRIX4<T>(m11/scalar,m12/scalar,m13/scalar,m14/scalar,
                                m21/scalar,m22/scalar,m23/scalar,m24/scalar,
                                m31/scalar,m32/scalar,m33/scalar,m34/scalar,
                                m41/scalar,m42/scalar,m43/scalar,m44/scalar);}


  // binary operators with matrices
  MATRIX4<T> operator * ( const MATRIX4<T>& other ) const {
    MATRIX4<T> result;
    for (int x = 0;x<16;x+=4)
      for (int y = 0;y<4;y++)
        result[x+y] = array[0+x] * other.array[0+y]+
                      array[1+x] * other.array[4+y]+
                      array[2+x] * other.array[8+y]+
                      array[3+x] * other.array[12+y];

    return result;
  }

  // binary operators with vectors
  VECTOR4<T> operator * ( const VECTOR4<T>& other ) const {
    return VECTOR4<T>(other.x*m11+other.y*m12+other.z*m13+other.w*m14,
                      other.x*m21+other.y*m22+other.z*m23+other.w*m24,
                      other.x*m31+other.y*m32+other.z*m33+other.w*m34,
                      other.x*m41+other.y*m42+other.z*m43+other.w*m44);
  }

  VECTOR3<T> operator * ( const VECTOR3<T>& other ) const {
    return VECTOR3<T>(other.x*m11+other.y*m12+other.z*m13,
                      other.x*m21+other.y*m22+other.z*m23,
                      other.x*m31+other.y*m32+other.z*m33);
  }

  void Translation(T x, T y, T z) {
    m11=1; m12=0; m13=0; m14=0;
    m21=0; m22=1; m23=0; m24=0;
    m31=0; m32=0; m33=1; m34=0;
    m41=x; m42=y; m43=z; m44=1;
  }

  void Scaling(T x, T y, T z) {
    m11=x; m12=0; m13=0; m14=0;
    m21=0; m22=y; m23=0; m24=0;
    m31=0; m32=0; m33=z; m34=0;
    m41=0; m42=0; m43=0; m44=1;
  }

  void RotationX(double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=1;    m12=0;      m13=0;      m14=0;
    m21=0;    m22= dCosAngle;  m23=dSinAngle;  m24=0;
    m31=0;    m32=-dSinAngle;  m33=dCosAngle;  m34=0;
    m41=0;    m42=0;      m43=0;      m44=1;
  }

  void RotationY(double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=dCosAngle;  m12=0;  m13=-dSinAngle;  m14=0;
    m21=0;      m22=1;  m23=0;      m24=0;
    m31=dSinAngle;  m32=0;  m33=dCosAngle;  m34=0;
    m41=0;      m42=0;  m43=0;      m44=1;
  }

  void RotationZ(double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=dCosAngle;  m12=dSinAngle;  m13=0;
    m21=-dSinAngle;  m22=dCosAngle;  m23=0;
    m31=0;      m32=0;      m33=1;
    m41=0;      m42=0;      m43=0;    m44=1;
  }

  void RotationAxis(const VECTOR3<T>& axis, double angle) {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));
    T dOneMinusCosAngle = 1-dCosAngle;

    VECTOR3<T> sqrAxis   = axis * axis;

    m11 = dCosAngle+dOneMinusCosAngle*sqrAxis.x;        m12 = dOneMinusCosAngle*axis.x*axis.y-dSinAngle*axis.z;  m13 = dOneMinusCosAngle*axis.x*axis.z+dSinAngle*axis.y;    m14=0;
    m21 = dOneMinusCosAngle*axis.x*axis.y+dSinAngle*axis.z;    m22 = dCosAngle+dOneMinusCosAngle*sqrAxis.y;      m23 = dOneMinusCosAngle*axis.y*axis.z-dSinAngle*axis.x;    m24=0;
    m31 = dOneMinusCosAngle*axis.x*axis.z-dSinAngle*axis.y;    m32 = dOneMinusCosAngle*axis.y*axis.z+dSinAngle*axis.x;  m33 = dCosAngle+dOneMinusCosAngle*sqrAxis.z;        m34=0;
    m41 = 0;                          m42 = 0;                        m43 = 0;                          m44=1;
  }

  MATRIX4<T> Transpose() const {
    MATRIX4<T> result;

    result.m41 = m14;
    result.m31 = m13;  result.m42 = m24;
    result.m21 = m12;  result.m32 = m23;  result.m43 = m34;
    result.m11 = m11;  result.m22 = m22;  result.m33 = m33;  result.m44 = m44;
    result.m12 = m21;  result.m23 = m32;  result.m34 = m43;
    result.m13 = m31;  result.m24 = m42;
    result.m14 = m41;

    return result;
  }


  MATRIX4<T> inverse() const {

    MATRIX4<T> result;

    T Q =   m21 *(m34*( m12 * m43 - m13  * m42)+
            m14 *(-m32 * m43 + m42 * m33)+
            m44*( m13 *  m32 - m12  * m33))
          +
          m24 *(m11 *( m32 * m43 - m42 * m33)+
            m13 *(-m41* m32  + m31  * m42)+
            m12 *(-m31 * m43 + m41 * m33))
          +
          m44*(m22 *(-m31 *  m13 + m11  * m33)+
            m23 *(-m11 *  m32 + m12  * m31))
          +
          m34*(m11 *(-m22 * m43 + m23  * m42)+
            m41*( m13 * m22  - m23  * m12))
          +
          m14* (m23 *( m32 * m41 - m42 * m31)+
            m22 *( m31 * m43 - m41 * m33));

    result.m11 =  ( m24  * m32  * m43
        + m44 * m22  * m33
        - m44 * m23  * m32
        - m34 * m22  * m43
        - m24  * m42 * m33
        + m34 * m23  * m42)/Q;
      result.m21 = -( m21  * m44 * m33
        - m21  * m34 * m43
        - m44 * m23  * m31
        + m34 * m23  * m41
        + m24  * m31  * m43
        - m24  * m41 * m33)/Q;
    result.m31 = (- m21  * m34 * m42
        + m21  * m44 * m32
        - m44 * m31  * m22
        - m24  * m41 * m32
        + m34 * m41 * m22
        + m24  * m31  * m42)/Q;
      result.m41 =  -(m21  * m32  * m43
        - m21  * m42 * m33
        + m41 * m22  * m33
        - m32  * m23  * m41
        - m31  * m22  * m43
        + m42 * m23  * m31)/Q;
    /// 2
    result.m12 = (- m12  * m44 * m33
        + m12  * m34 * m43
        - m34 * m13  * m42
        - m14  * m32  * m43
        + m44 * m13  * m32
        + m14  * m42 * m33)/Q;

    result.m22 = (- m44 * m13  * m31
        + m44 * m11  * m33
        - m34 * m11  * m43
        - m14  * m41 * m33
        + m34 * m13  * m41
        + m14  * m31  * m43)/Q;

    result.m32 = -(-m12  * m44 * m31
        + m12  * m34 * m41
        + m44 * m11  * m32
        - m14  * m32  * m41
        + m14  * m42 * m31
        - m34 * m11  * m42)/Q;

    result.m42 = (- m12  * m31  * m43
        + m12  * m41 * m33
        + m11  * m32  * m43
        - m11  * m42 * m33
        - m41 * m13  * m32
        + m31  * m13  * m42)/Q;
    /// 3
    result.m13 = -( m44 * m13  * m22
        - m24  * m13  * m42
        - m14  * m22  * m43
        + m12  * m24  * m43
        - m12  * m44 * m23
        + m14  * m42 * m23)/Q;

    result.m23 = (- m21  * m14  * m43
        + m21  * m44 * m13
        + m24  * m11  * m43
        - m44 * m23  * m11
        - m24  * m41 * m13
        + m14  * m23  * m41)/Q;

    result.m33 = -(-m44 * m11  * m22
        + m44 * m12  * m21
        + m14  * m41 * m22
        + m24  * m11  * m42
        - m24  * m12  * m41
        - m14  * m21  * m42)/Q;

    result.m43 = -( m43 * m11  * m22
        - m43 * m12  * m21
        - m13  * m41 * m22
        - m23  * m11  * m42
        + m23  * m12  * m41
        + m13  * m21  * m42)/Q;
    /// 4
    result.m14 = (- m12  * m34 * m23
        + m12  * m24  * m33
        - m24  * m13  * m32
        - m14  * m22  * m33
        + m34 * m13  * m22
        + m14  * m32  * m23)/Q;

    result.m24 = -(-m21  * m14  * m33
        + m21  * m34 * m13
        + m24  * m11  * m33
        - m34 * m23  * m11
        + m14  * m23  * m31
        - m24  * m31  * m13)/Q;

    result.m34 = (- m34 * m11  * m22
        + m34 * m12  * m21
        + m14  * m31  * m22
        + m24  * m11  * m32
        - m24  * m12  * m31
        - m14  * m21  * m32)/Q;

    result.m44 =  ( m33 * m11  * m22
        - m33 * m12  * m21
        - m13  * m31  * m22
        - m23  * m11  * m32
        + m23  * m12  * m31
        + m13  * m21  * m32)/Q;
    return result;
  }

  // DirectX
  #ifdef USEDX
    MATRIX4( const D3DXMATRIX& other ) : m11(other(0,0)), m12(other(0,1)), m13(other(0,2)),m14(other(0,3)),
                       m21(other(1,0)), m22(other(1,1)), m23(other(1,2)),m24(other(1,3)),
                       m31(other(2,0)), m32(other(2,1)), m33(other(2,2)),m34(other(2,3)),
                       m41(other(3,0)), m42(other(3,1)), m43(other(3,2)),m44(other(3,3)) {};
    D3DXMATRIX toD3DXMAT() const {return D3DXMATRIX(FLOAT(m11),FLOAT(m12),FLOAT(m13),FLOAT(m14),
                            FLOAT(m21),FLOAT(m22),FLOAT(m23),FLOAT(m24),
                            FLOAT(m31),FLOAT(m32),FLOAT(m33),FLOAT(m34),
                            FLOAT(m41),FLOAT(m42),FLOAT(m43),FLOAT(m44));}
    operator D3DXMATRIX(void) const {return toD3DXMAT();}
  #endif

  // OpenGL
  #ifdef USEGL


    static void BuildStereoLookAtAndProjection(const VECTOR3<T> vEye, const VECTOR3<T> vAt, const VECTOR3<T> vUp,
                                              T fFOVY, T fAspect, T fZNear, T fZFar, T fFocalLength,
                                              T fEyeDist, int iEyeID, MATRIX4<T>& mView, MATRIX4<T>& mProj) {

      T radians = T(3.14159265358979323846/180.0) * fFOVY/T(2);
      T wd2     = fZNear * T(tan(radians));
      T nfdl    = fZNear / fFocalLength;
      T fShift  =   fEyeDist * nfdl;
      T left    = - fAspect * wd2 + T(iEyeID)*fShift;
      T right   =   fAspect * wd2 + T(iEyeID)*fShift;
      T top     =   wd2;
      T bottom  = - wd2;

      // projection matrix
      mProj.MatrixPerspectiveOffCenter(left, right, bottom, top, fZNear, fZFar);

      // view matrix
      mView.BuildLookAt(vEye, vAt, vUp);
      MATRIX4<T> mTranslate;
      mTranslate.Translation(fEyeDist*T(iEyeID), 0.0f, 0.0f);
      mView= mTranslate * mView;
    }


    static void BuildStereoLookAtAndProjection(const VECTOR3<T> vEye, const VECTOR3<T> vAt, const VECTOR3<T> vUp,
                                              T fFOVY, T fAspect, T fZNear, T fZFar, T fFocalLength,
                                              T fEyeDist, MATRIX4<T>& mViewLeft, MATRIX4<T>& mViewRight, MATRIX4<T>& mProjLeft, MATRIX4<T>& mProjRight) {

      T radians = T(3.14159265358979323846/180.0) *  fFOVY/2;
      T wd2     = fZNear * T(tan(radians));
      T nfdl    = fZNear / fFocalLength;
      T fShift  =   fEyeDist * nfdl;
      T left    = - fAspect * wd2 + fShift;
      T right   =   fAspect * wd2 + fShift;
      T top     =   wd2;
      T bottom  = - wd2;

      // projection matrices
      mProjLeft.MatrixPerspectiveOffCenter(left, right, bottom, top, fZNear, fZFar);
      left    = - fAspect * wd2 - fShift;
      right   =   fAspect * wd2 - fShift;
      mProjRight.MatrixPerspectiveOffCenter(left, right, bottom, top, fZNear, fZFar);

      // view matrices
      mViewLeft.BuildLookAt(vEye, vAt, vUp);
      mViewRight.BuildLookAt(vEye, vAt, vUp);

      // eye translation
      MATRIX4<T> mTranslate;

      mTranslate.Translation(fEyeDist, 0.0f, 0.0f);
      mViewLeft = mTranslate * mViewLeft;

      mTranslate.Translation(-fEyeDist, 0.0f, 0.0f);
      mViewRight = mTranslate * mViewRight;
    }

    void BuildLookAt(const VECTOR3<T> vEye, const VECTOR3<T> vAt, const VECTOR3<T> vUp) {
      VECTOR3<T> F = vAt-vEye;
      VECTOR3<T> U = vUp;
      VECTOR3<T> S = F % U;
      U = S % F;

      F.normalize();
      U.normalize();
      S.normalize();

      array[ 0]= S[0];  array[ 4]= S[1];  array[ 8]= S[2];  array[12]=-(S^vEye);
      array[ 1]= U[0];  array[ 5]= U[1];  array[ 9]= U[2];  array[13]=-(U^vEye);
      array[ 2]=-F[0];  array[ 6]=-F[1];  array[10]=-F[2];  array[14]= (F^vEye);
      array[ 3]= T(0);  array[ 7]=T(0);   array[11]=T(0);   array[15]= T(1);
    }

    void Perspective(T fovy, T aspect, T n, T f) {
      // deg 2 rad
      fovy = fovy * T(3.14159265358979323846/180.0);

      T cotan = T(1.0/tan(double(fovy)/2.0));

      array[ 0]= cotan/aspect;  array[ 4]=T(0);     array[ 8]=T(0);             array[12]=T(0);
      array[ 1]= T(0);          array[ 5]=cotan;    array[ 9]=T(0);             array[13]=T(0);
      array[ 2]= T(0);          array[ 6]=T(0);     array[10]=-(f+n)/(f-n);     array[14]=T(-2)*(f*n)/(f-n);
      array[ 3]= T(0);          array[ 7]=T(0);     array[11]=T(-1);            array[15]=T(0);
    }

    void Ortho(T left, T right, T bottom, T top, T znear, T zfar ) {
      array[ 0]= T(2)/(right-left);  array[ 4]=T(0);                array[ 8]=T(0);               array[12]=-(right+left)/(right-left);
      array[ 1]= T(0);               array[ 5]=T(2)/(top-bottom);   array[ 9]=T(0);               array[13]=-(top+bottom)/(top-bottom);
      array[ 2]= T(0);               array[ 6]=T(0);                array[10]=-T(2)/(zfar-znear); array[14]=-(zfar+znear)/(zfar-znear);
      array[ 3]= T(0);               array[ 7]=T(0);                array[11]=T(0);               array[15]=T(1);
    }

    void MatrixPerspectiveOffCenter(T left, T right, T bottom, T top, T n, T f) {
      array[ 0]= T(2)*n/(right-left);   array[ 4]=T(0);                   array[ 8]=(right+left)/(right-left);  array[12]=T(0);
      array[ 1]= T(0);                  array[ 5]=T(2)*n/(top-bottom);    array[ 9]=(top+bottom)/(top-bottom);  array[13]=T(0);
      array[ 2]= T(0);                  array[ 6]=T(0);                   array[10]=-(f+n)/(f-n);               array[14]=T(-2)*(f*n)/(f-n);
      array[ 3]= T(0);                  array[ 7]=T(0);                   array[11]=T(-1);                      array[15]=T(0);
    }

    void getProjection() {
      float P[16];
      glGetFloatv(GL_PROJECTION_MATRIX,P);
      for (int i=0;i<16;i++) array[i] = T(P[i]);
    }

    void getModelview() {
      float M[16];
      glGetFloatv(GL_MODELVIEW_MATRIX,M);
      for (int i=0;i<16;i++) array[i] = T(M[i]);
    }

    void multModelview() {
      float M[16];
      for (int i=0;i<16;i++) M[i] = float(array[i]);
      glMatrixMode(GL_MODELVIEW);
      glMultMatrixf(M);
    }

    void setProjection() {
      float M[16];
      for (int i=0;i<16;i++) M[i] = float(array[i]);
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(M);
    }

    void setModelview() {
      float M[16];
      for (int i=0;i<16;i++) M[i] = float(array[i]);
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(M);
    }

    void setTextureMatrix(int iUnit = 0) {
      float M[16];
      for (int i=0;i<16;i++) M[i] = float(array[i]);
      glActiveTextureARB(GL_TEXTURE0_ARB+iUnit);
      glMatrixMode(GL_TEXTURE);
      glLoadMatrixf(M);
    }

  #endif
};

typedef MATRIX2<int> INTMATRIX2;
typedef MATRIX3<int> INTMATRIX3;
typedef MATRIX4<int> INTMATRIX4;
typedef MATRIX2<float> FLOATMATRIX2;
typedef MATRIX3<float> FLOATMATRIX3;
typedef MATRIX4<float> FLOATMATRIX4;
typedef MATRIX2<double> DOUBLEMATRIX2;
typedef MATRIX3<double> DOUBLEMATRIX3;
typedef MATRIX4<double> DOUBLEMATRIX4;

template <class T> class QUATERNION4 {
public:
  float x, y, z, w;

  QUATERNION4<T>(): x(0), y(0),z(0), w(0) {}
  QUATERNION4<T>(T _x, T _y, T _z, T _w): x(_x), y(_y), z(_z), w(_w) {}
  QUATERNION4<T>(const T* other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

  MATRIX4<T> ComputeRotation() {
    float n, s;
    float xs, ys, zs;
    float wx, wy, wz;
    float xx, xy, xz;
    float yy, yz, zz;

    n = (x * x) + (y * y) + (z * z) + (w * w);
    s = (n > 0.0f) ? (2.0f / n) : 0.0f;

    xs = x * s;
    ys = y * s;
    zs = z * s;
    wx = w * xs;
    wy = w * ys;
    wz = w * zs;
    xx = x * xs;
    xy = x * ys;
    xz = x * zs;
    yy = y * ys;
    yz = y * zs;
    zz = z * zs;

    return MATRIX4<T>(T(1.0f - (yy + zz)), T(xy - wz),          T(xz + wy),          T(0),
                      T(xy + wz),          T(1.0f - (xx + zz)), T(yz - wx),          T(0),
                      T(xz - wy),          T(yz + wx),          T(1.0f - (xx + yy)), T(0),
                      T(0),                T(0),                T(0),                T(1));

  }

  bool operator == ( const QUATERNION4<T>& other ) const {return (other.x==x && other.y==y && other.z==z && other.w==w); }
  bool operator != ( const QUATERNION4<T>& other ) const {return (other.x!=x || other.y!=y || other.z!=z || other.w!=w); }

  // binary operators with other quaternions
  QUATERNION4<T> operator + ( const QUATERNION4<T>& other ) const {return QUATERNION4<T>(x+other.x,y+other.y,z+other.z,w+other.w);}
  QUATERNION4<T> operator - ( const QUATERNION4<T>& other ) const {return QUATERNION4<T>(x-other.x,y-other.y,z-other.z,w-other.w);}
  QUATERNION4<T> operator * ( const QUATERNION4<T>& other ) const {
    VECTOR3<T> v1(x,y,z);
    VECTOR3<T> v2(other.x,other.y,other.z);

    T _w = w * other.w - (v1 ^ v2);
    VECTOR3<T> _v = (v2 * w) + (v1 * other.w) + (v1 % v2);

    return QUATERNION4<T>(_v.x, _v.y, _v.z, _w);
  }


  // binary operator with scalars
  QUATERNION4<T> operator * ( const T other ) const {
    return QUATERNION4<T>(x*other,y*other,z*other,w*other);
  }

  QUATERNION4<T>& operator=(const QUATERNION4<T>& other)  { x = other.x; y = other.y; z = other.z; w = other.w; return *this; }
  QUATERNION4<T>& operator+=(const T& other) { x += other; y += other; z += other;  w += other; return *this; }
  QUATERNION4<T>& operator-=(const T& other) { x -= other; y -= other; z -= other;  w -= other; return *this; }
};

typedef QUATERNION4<float>  FLOATQUATERNION4;
typedef QUATERNION4<double> DOUBLEQUATERNION4;


/// Tests to see if the two values are basically the same.
template <class T> static bool EpsilonEqual(T a, T b) {
  /// @todo FIXME provide specializations for types; currently this would only
  ///       be correct if instantiated as a float!
  return fabs(a-b) <= std::numeric_limits<T>::epsilon();
}

/// a PLANE is a VECTOR4 which is always normalized.
template <class T> class PLANE : public VECTOR4<T> {
public:
  PLANE<T>(): VECTOR4<T>(0,0,0,0) {}
  template <class S> explicit PLANE<T>( const std::vector<S>& v ) {
    this->x = T(v.size()>0 ? v[0] : 0);
    this->y = T(v.size()>1 ? v[1] : 0);
    this->z = T(v.size()>2 ? v[2] : 0);
    this->w = T(v.size()>3 ? v[3] : 0);
  }
  PLANE<T>(const VECTOR2<T> &other, const T _z, const T _w):
    VECTOR4<T>(other, _z, _w) {}
  PLANE<T>(const VECTOR3<T> &other, const T _w = 1): VECTOR4<T>(other, _w) {}
  PLANE<T>(const VECTOR4<T> &other): VECTOR4<T>(other) {}

  template <class S> explicit PLANE<T>(const PLANE<S> &other):
    VECTOR4<T>(other) {}

  PLANE<T>(const T _x, const T _y, const T _z, const T _w) :
    VECTOR4<T>(_x,_y,_z,_w) {}
  PLANE<T>(const T* vec) : VECTOR4<T>(vec) {}

  /// @return true if the given point is clipped by this plane.
  bool clip(VECTOR3<T> point) const {
    return ((FLOATVECTOR4::xyz() ^ point)+this->w >= 0);
  }

  /// Transform the plane by the given matrix.
  void transform(const MATRIX4<T> &m) {
    FLOATMATRIX4 mIT(m.inverse());
    mIT = mIT.Transpose();
    transformIT(mIT);
  }

  /// Transform the plane by the inverse transpose of the given matrix.
  void transformIT(const MATRIX4<T> &M) {
    (*this) = FLOATVECTOR4::operator*(M);
    normalize();
  }

  void normalize() {
    const T x = this->x;
    const T y = this->y;
    const T z = this->z;
    T length = sqrt(x*x + y*y + z*z);
    (*this) /= length;
  }
  T d() const { return this->w; }
  VECTOR3<T> normal() { return this->xyz(); }

  // binary operators with a matrix
  PLANE<T> operator * ( const MATRIX4<T>& matrix ) const {
    PLANE<T> tmp(*this);
    tmp.transform(matrix);
    return tmp;
  }

  /// Determine the intersection point of the plane and a line `ab'.
  /// @return whether or not the two intersect.  If false, `hit' will not be
  ///         valid.
  bool intersect(const FLOATVECTOR3& a, const FLOATVECTOR3& b,
                 FLOATVECTOR3& hit) const {
    const float denom = (*this) ^ (a - b);
    if(EpsilonEqual(denom, 0.0f)) {
      return false;
    }
    const float t = (((*this) ^ a) + this->d()) / denom;
    hit = a + (t*(b - a));
    return true;
  }
};

#endif // VECTORS_H
