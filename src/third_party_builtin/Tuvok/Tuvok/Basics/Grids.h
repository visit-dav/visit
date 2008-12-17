#pragma once

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
  \file    Grids.h
  \brief    Simple grid templates
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    July 2008
*/

#ifndef GRIDS
#define GRIDS

#include "Vectors.h"

template <class T> class GridnD  {
  public:
    GridnD<T>() : m_pData(NULL) {}
    virtual ~GridnD<T>() {delete [] this->m_pData;}

    T* GetDataPointer() {return this->m_pData;}
    T GetLinear(size_t i) const {return this->m_pData[i];}
    void SetLinear(size_t i, T tValue) {this->m_pData[i] = tValue;}

  protected:
    T* m_pData;
};

template <class T> class Grid1D : public GridnD<T> {
  public:
    Grid1D<T>() : GridnD<T>(), m_iSize(0) {}
    Grid1D<T>(const size_t& iSize) : m_iSize(iSize) { this->m_pData = new T[m_iSize];}
    Grid1D<T>(const Grid1D<T> &other): m_iSize(other.m_iSize) {
      this->m_pData = new T[m_iSize];
      memcpy(this->m_pData, other->m_pData, sizeof(T)*m_iSize);
    }
    virtual ~Grid1D<T>() {}

    size_t GetFilledSize() const {
      size_t iSize = 0;
      for (size_t i = 0;i<m_iSize;i++) {
        if (this->m_pData[i] != 0) iSize = i+1;
      }
      return iSize;
    }

    size_t GetSize() const {return m_iSize;}
    void Resize(const size_t& iSize) {
      delete [] this->m_pData; 
      m_iSize = iSize;
      this->m_pData = new T[m_iSize];
    }

    T Get(size_t i) const {return this->m_pData[i];}
    void Set(size_t i, T tValue) {this->m_pData[i] = tValue;}
    
    Grid1D<T>& operator=(const Grid1D<T>& other)  { 
      delete [] this->m_pData; 
      m_iSize = other.m_iSize;
      this->m_pData = new T[m_iSize];
      memcpy(this->m_pData, other->m_pData, sizeof(T)*m_iSize);
      return *this; 
    }

  private:
    size_t m_iSize;
};

template <class T> class Grid2D : public GridnD<T> {
  public:
    Grid2D<T>() : GridnD<T>(), m_iSize(0,0) {}
    Grid2D<T>(const VECTOR2<size_t>& iSize) : m_iSize(iSize) { this->m_pData = new T[m_iSize.area()];}
    Grid2D<T>(const Grid2D<T> &other): m_iSize(other.m_iSize) {
      this->m_pData = new T[m_iSize.area()];
      memcpy(this->m_pData, other->m_pData, sizeof(T)*m_iSize.area());
    }
    virtual ~Grid2D<T>() {}

    VECTOR2<size_t> GetFilledSize() const {
      VECTOR2<size_t> vSize(0,0);
      for (size_t y = 0;y<m_iSize.y;y++) {
        for (size_t x = 0;x<m_iSize.x;x++) {
          if (this->Get(x,y) != 0) {
            if ((x+1) > vSize.x) vSize.x = x+1;
            vSize.y = y+1;
          }
        }
      }
      return vSize;
    }


    VECTOR2<size_t> GetSize() const {return m_iSize;}
    void Resize(const VECTOR2<size_t>& iSize) {
      delete [] this->m_pData; 
      m_iSize = iSize;
      this->m_pData = new T[m_iSize.area()];
    }

    T Get(const UINTVECTOR2& pos) const {return this->m_pData[pos.x+pos.y*m_iSize.x];}
    void Set(const UINTVECTOR2& pos, T tValue) {this->m_pData[pos.x+pos.y*m_iSize.x] = tValue;}
    T Get(size_t x, size_t y) const {return this->m_pData[x+m_iSize.x*y];}
    void Set(size_t x, size_t y, T tValue) {this->m_pData[x+m_iSize.x*y] = tValue;}
    
    Grid2D<T>& operator=(const Grid2D<T>& other)  { 
      delete [] this->m_pData; 
      m_iSize = other.m_iSize;
      this->m_pData = new T[m_iSize.area()];
      memcpy(this->m_pData, other->m_pData, sizeof(T)*m_iSize.area());
      return *this; 
    }

  private:
    VECTOR2<size_t> m_iSize;
};


template <class T> class Grid3D : public GridnD<T> {
  public:
    Grid3D<T>() : GridnD<T>(), m_iSize(0,0,0) {}
    Grid3D<T>(const VECTOR3<size_t>& iSize) : m_iSize(iSize) { this->m_pData = new T[m_iSize.volume()];}
    Grid3D<T>(const Grid3D<T> &other): m_iSize(other.m_iSize) {
      this->m_pData = new T[m_iSize.volume()];
      memcpy(this->m_pData, other->m_pData, sizeof(T)*m_iSize.volume());
    }
    virtual ~Grid3D<T>() {}

    VECTOR3<size_t> GetSize() const {return m_iSize;}
    void Resize(const VECTOR3<size_t>& iSize) {
      delete [] this->m_pData; 
      m_iSize = iSize;
      this->m_pData = new T[m_iSize.volume()];
    }

    T Get(const UINTVECTOR3& pos) const {return this->m_pData[pos.x+pos.y*m_iSize.x+pos.z*m_iSize.x*m_iSize.y];}
    void Set(const UINTVECTOR3& pos, T tValue) {this->m_pData[pos.x+pos.y*m_iSize.x+pos.z*m_iSize.x*m_iSize.y] = tValue;}
    T Get(size_t x, size_t y, size_t z) const {return this->m_pData[x+m_iSize.x*y+z*m_iSize.x*m_iSize.y];}
    void Set(size_t x, size_t y, size_t z, T tValue) {this->m_pData[x+m_iSize.x*y+z*m_iSize.x*m_iSize.y] = tValue;}
    
    Grid3D<T>& operator=(const Grid3D<T>& other)  { 
      delete [] this->m_pData; 
      m_iSize = other.m_iSize;
      this->m_pData = new T[m_iSize.volume()];
      memcpy(this->m_pData, other->m_pData, sizeof(T)*m_iSize.volume());
      return *this; 
    }

  private:
    VECTOR3<size_t> m_iSize;
};

#endif // GRIDS
