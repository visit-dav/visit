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
  \file    StdDefines.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    October 2008
*/

#pragma once

#ifndef STDDEFINES_H
#define STDDEFINES_H

#include <limits>

#ifdef _WIN32
  typedef unsigned __int64 UINT64;
#else
  typedef unsigned long long UINT64;
#endif
typedef unsigned int UINT32;

#define UNUSED 0
#define UNUSED_FLOAT 0.0f
#define UNUSED_DOUBLE 0.0f
#define IV3D_VERSION "0.02 beta"
#define UINT32_INVALID (std::numeric_limits<UINT>::max())
#define UINT64_INVALID (std::numeric_limits<UINT64>::max())


#endif // STDDEFINES_H
