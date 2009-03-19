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
#ifndef TUVOK_GL_DEBUG_H
#define TUVOK_GL_DEBUG_H

/**
  \file    GLDebug.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Functions for debugging OpenGL code.
*/

#include "Controller/MasterController.h"
#include "Controller/Controller.h"

#define CHECK_GL()                                                    \
  {                                                                   \
    GLenum err = glGetError();                                        \
    switch(err) {                                                     \
      case GL_NO_ERROR: /* do nothing. */ break;                      \
      case GL_INVALID_ENUM:                                           \
        T_ERROR("GL(line %zu): invalid enum", __LINE__);              \
        break;                                                        \
      case GL_INVALID_VALUE:                                          \
        T_ERROR("GL(line %zu): invalid value", __LINE__);             \
        break;                                                        \
      case GL_INVALID_OPERATION:                                      \
        T_ERROR("GL(line %zu): invalid operation", __LINE__);         \
        break;                                                        \
      case GL_STACK_OVERFLOW:                                         \
        T_ERROR("GL(line %zu): stack overflow", __LINE__);            \
        break;                                                        \
      case GL_STACK_UNDERFLOW:                                        \
        T_ERROR("GL(line %zu): stack underflow", __LINE__);           \
        break;                                                        \
      case GL_OUT_OF_MEMORY:                                          \
        T_ERROR("GL(line %zu): out of memory", __LINE__);             \
        break;                                                        \
      default:                                                        \
        WARNING("GL(line %zu): unknown error generated..", __LINE__); \
        break;                                                        \
    }                                                                 \
  }

#ifdef _DEBUG
# define GL(stmt) do { stmt; CHECK_GL(); } while(0)
#else
# define GL(stmt) do { stmt; } while(0)
#endif

#endif // TUVOK_GL_DEBUG_H
