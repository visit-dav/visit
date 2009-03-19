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
  \file    GLTexture2D.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#include "GLTexture2D.h"


GLTexture2D::GLTexture2D(UINT32 iSizeX, UINT32 iSizeY, GLint internalformat, GLenum format, GLenum type,
             UINT32 iSizePerElement, const GLvoid *pixels, GLint iMagFilter, GLint iMinFilter, GLint wrapX, GLint wrapY) :
  GLTexture(iSizePerElement),
  m_iSizeX(GLuint(iSizeX)),
  m_iSizeY(GLuint(iSizeY)),
  m_internalformat(internalformat),
  m_format(format),
  m_type(type)
{
  glGenTextures(1, &m_iGLID);
  glBindTexture(GL_TEXTURE_2D, m_iGLID);

  glPixelStorei(GL_PACK_ALIGNMENT ,1);
  glPixelStorei(GL_UNPACK_ALIGNMENT ,1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapX);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapY);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iMagFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iMinFilter);
  glTexImage2D(GL_TEXTURE_2D, 0, m_internalformat, GLuint(m_iSizeX), GLuint(m_iSizeY), 0, m_format, m_type, (GLvoid*)pixels);
}

void GLTexture2D::SetData(const void *pixels) {
  glPixelStorei(GL_PACK_ALIGNMENT ,1);
  glPixelStorei(GL_UNPACK_ALIGNMENT ,1);

  glBindTexture(GL_TEXTURE_2D, m_iGLID);
  glTexImage2D(GL_TEXTURE_2D, 0, m_internalformat, m_iSizeX, m_iSizeY, 0, m_format, m_type, (GLvoid*)pixels);
}
