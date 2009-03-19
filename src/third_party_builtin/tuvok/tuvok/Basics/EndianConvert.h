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
  \file    EndianConvert.h
  \author    Jens Schneider
        Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    July 2008
*/

#pragma once

#ifndef ENDIANCONVERT_H
#define ENDIANCONVERT_H

#include <cassert>

#ifndef WIN32
  #include <unistd.h>
#endif

#define ENDIANCONVERT_THREAD_SAFE  ///< Switch on to ensure Thread-safeness. Quick and untested hack.

namespace EndianConvert {
  /**
   * Swap routine to convert between little and big-endian.
   * x is converted in situ.
   * \param x pointer to data we should swap.
   * \return void 
   * \date October.2004
   */
  template<class TYPE>
  inline void Swap(TYPE* x) {
  #ifndef ENDIANCONVERT_THREAD_SAFE
    static 
  #endif
    union {
      TYPE t;
      char c[sizeof(TYPE)];
    } uSwapSpace;
  #ifndef ENDIANCONVERT_THREAD_SAFE
    static 
  #endif
    char c;
    uSwapSpace.t=*x;
    for (size_t i=0; i<(sizeof(TYPE)>>1); i++) {
      c=uSwapSpace.c[i];
      uSwapSpace.c[i]=uSwapSpace.c[sizeof(TYPE)-i-1];
      uSwapSpace.c[sizeof(TYPE)-i-1]=c;
    }
    *x=uSwapSpace.t;
  }



  /**
   * Swap routine to convert between little and big-endian.
   * x is converted and returned.
   * \param x
   * \return the converted element
   * \date October.2004
   */
  template<class TYPE>
  inline TYPE Swap(const TYPE x) {
  #ifndef ENDIANCONVERT_THREAD_SAFE
    static 
  #endif
    union {
      TYPE t;
      char c[sizeof(TYPE)];
    } uSwapSpace;
  #ifndef ENDIANCONVERT_THREAD_SAFE
    static 
  #endif
    char c;
    uSwapSpace.t=x;
    for (int i=0; i<(int)(sizeof(TYPE)>>1); i++) {  // use int here to supress warning when called with 8bit data
      c=uSwapSpace.c[i];
      uSwapSpace.c[i]=uSwapSpace.c[sizeof(TYPE)-i-1];
      uSwapSpace.c[sizeof(TYPE)-i-1]=c;
    }
    return uSwapSpace.t;
  }



  /**
   * Checks whether this machine is big-endian. 
   * \return true if the machine is big-endian, false otherwise
   * \date December.2004
   */
  inline bool IsBigEndian(void) {
    union {
      int i;
      char c[sizeof(int)];
    } tmp;
    assert (sizeof(int)>1);
    tmp.i=0x1020;
    return tmp.c[0]!=0x20;
  }



  /**
   * Checks whether this machine is little-endian. 
   * \return true if the machine is little-endian, false otherwise
   * \date December.2004
   */
  inline bool IsLittleEndian(void) {
    return !IsBigEndian();
  }


  template<class T>
  inline bool IsNaN(T a) {
    return (!(a<=T(42.0)) && !(a>=T(42.0)));
  }

  /**
   * Swaps the endianness without prior knowledge about bit order.
   * \param data         array to swap every element of.
   * \param NumElements  size of the array.
   * \return the variance of the swapped data.
   * \date Jul.2006
   */
  template<class T>
  inline double SmartSwap(T* data, size_t NumElements) {
    double AVG=0.0,VAR=0.0;
    double AVG2=0.0,VAR2=0.0;
    for (size_t i=0; i<NumElements; i++) {
      double d=double(data[i]);
      AVG+=d;
      VAR+=d*d;
      Swap<T>(&data[i]);
      d=double(data[i]);
      AVG2+=d;
      VAR2+=d*d;
      // Early NaN-Check to speed up the process
      if ((i%1024)!=1023 && i!=NumElements-1) {}
      else {
        if (IsNaN<double>(VAR)) { // early out, swapped is correct
          for (size_t j=i+1; j<NumElements; j++) {
            Swap<T>(&data[j]);
            d=double(data[j]);
            AVG2+=d;
            VAR2+=d*d;
          }
          AVG2/=double(NumElements);
          VAR2=(VAR2-double(NumElements)*AVG2*AVG2)/double(NumElements-1);
  #ifdef _DEBUG
          printf("%s (%.3f vs NaN) ",IsBigEndian() ? "little endian" : "big endian",VAR2);
  #endif
          return VAR2;
        }
        else if (IsNaN<double>(VAR2)) { // early out, non-swapped was correct
          for (size_t j=0; j<=i; j++) Swap<T>(&data[j]);
          for (size_t j=i=i+1; j<NumElements; j++) {
            d=double(data[j]);
            AVG+=d;
            VAR+=d*d;
          }
          AVG/=double(NumElements);
          VAR=(VAR-double(NumElements)*AVG*AVG)/double(NumElements-1);
  #ifdef _DEBUG        
          printf("%s (%.3f vs NaN) ",IsBigEndian() ? "big endian" : "little endian",VAR);
  #endif
          return VAR;
        }
      }
    }
    AVG/=double(NumElements);
    VAR=(VAR-double(NumElements)*AVG*AVG)/double(NumElements-1);
    AVG2/=double(NumElements);
    VAR2=(VAR2-double(NumElements)*AVG2*AVG2)/double(NumElements-1);
    if (VAR<VAR2) { // non-flipped was correct.
  #ifdef _DEBUG
      printf("%s (%.3f vs %.3f) ",IsBigEndian() ? "big endian" : "little endian",VAR,VAR2);
  #endif
      for (size_t i=0; i<NumElements; i++) Swap<T>(&data[i]);
      return VAR;
    }
    else {
  #ifdef _DEBUG
      printf("%s (%.3f vs %.3f) ",IsBigEndian() ? "little endian" : "big endian",VAR2,VAR);
  #endif
      return VAR2;
    }
  }
};

#endif // ENDIANCONVERT_H
