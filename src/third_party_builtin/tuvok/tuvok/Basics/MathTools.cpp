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
  \file    MathTools.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.11
  \date    October 2008
*/

#include "MathTools.h"

unsigned int MathTools::Log(unsigned int value, unsigned int base) {
  return (unsigned int)(log(float(value)) / log(float(base)));
}

float MathTools::Log(float value, float base) {
  return log(value) / log(base);
}

unsigned int MathTools::Pow(unsigned int base, unsigned int exponent) {
  return (unsigned int)(0.5f+pow(float(base), float(exponent)));
}

UINT64 MathTools::Pow(UINT64 base, UINT64 exponent) {
  return (UINT64)(0.5+pow(double(base), double(exponent)));
}

unsigned int MathTools::Log2(unsigned int n) {
  int iLog=0;
  while( n>>=1 ) iLog++;
  return iLog;
}

unsigned int MathTools::Pow2(unsigned int e) {
  return 1<<e;
}


UINT64 MathTools::Log2(UINT64 n) {
  int iLog=0;
  while( n>>=1 ) iLog++;
  return iLog;
}

UINT64 MathTools::Pow2(UINT64 e) {
#ifdef WIN32
  return 1i64<<e;
#else
  return 1<<e;
#endif
}

unsigned int MathTools::GaussianSum(unsigned int n) {
  return n*(n+1)/2;
}

bool MathTools::IsPow2(unsigned int n) { 
    return ((n&(n-1))==0); 
};

unsigned int MathTools::NextPow2(unsigned int n, bool bReturn_ID_on_Pow2) {
    if (bReturn_ID_on_Pow2 && IsPow2(n)) return n;
    return Pow2(Log2(n)+1);
}
