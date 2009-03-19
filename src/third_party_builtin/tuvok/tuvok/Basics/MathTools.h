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
  \file    MathTools.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.11
  \date    October 2008
*/

#pragma once

#ifndef MATHTOOLS_H
#define MATHTOOLS_H

#include "../StdTuvokDefines.h"
#include <cmath>

#define ROOT3 1.732050f

namespace MathTools {
  UINT32 Log(UINT32 value, UINT32 base);
  float Log(float value, float base);
  UINT32 Pow(UINT32 base, UINT32 exponent);
  UINT64 Pow(UINT64 base, UINT64 exponent);

  UINT32 Log2(UINT32 n);
  UINT32 Pow2(UINT32 e);
  UINT64 Log2(UINT64 n);
  UINT64 Pow2(UINT64 e);
  UINT32 GaussianSum(UINT32 n);
  bool IsPow2(UINT32 n);
  UINT32 NextPow2(UINT32 n, bool bReturn_ID_on_Pow2=true);

  template<class T> inline T sign(T v){return T((v > T(0)) - (v < T(0)));}

  template<class T> inline T MakeMultiple(T v, T m){return v + (((v%m) == T(0)) ? T(0) : m-(v%m));}
};

#endif // MATHTOOLS_H
