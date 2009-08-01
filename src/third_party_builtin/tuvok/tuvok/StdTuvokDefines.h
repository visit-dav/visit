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
  \file    StdTuvokDefines.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    December 2008
*/
#pragma once

#ifndef STDTUVOKDEFINES_H
#define STDTUVOKDEFINES_H

#include "Basics/StdDefines.h"

// Make sure windows doesn't give us stupid macros.
#define NOMINMAX

#define BOOST_FILESYSTEM_STATIC_LINK 1
#ifdef __cplusplus
# include <limits>
#else
# include <limits.h>
#endif

#define TUVOK_MAJOR 1
#define TUVOK_MINOR 1
#define TUVOK_PATCH 1
#define TUVOK_VERSION "1.1.1"
#define TUVOK_VERSION_TYPE "Release"

#ifdef _MSC_VER
# define _func_ __FUNCTION__
#elif defined(__GNUC__)
# define _func_ __func__
#else
# warning "unknown compiler!"
# define _func_ "???"
#endif

#endif // STDTUVOKDEFINES_H
