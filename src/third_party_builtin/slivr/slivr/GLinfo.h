//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : GLinfo.h
//    Author : Milan Ikits
//    Date   : Wed Jul 14 15:55:55 2004

#ifndef SLIVR_GLinfo_h
#define SLIVR_GLinfo_h

#include <GL/glew.h>

namespace SLIVR {

template <typename T>
struct GLinfo
{
  static const GLenum type = GL_NONE;
};

template <>
struct GLinfo<unsigned char>
{
  static const GLenum type = GL_UNSIGNED_BYTE;
};

template <>
struct GLinfo<char>
{
  static const GLenum type = GL_UNSIGNED_BYTE;
};

template <>
struct GLinfo<unsigned short>
{
  static const GLenum type = GL_UNSIGNED_SHORT;
};

template <>
struct GLinfo<short>
{
  static const GLenum type = GL_SHORT;
};

template <>
struct GLinfo<float>
{
  static const GLenum type = GL_FLOAT;
};

} // end namespace SLIVR

#endif // Volume_GLinfo_h
