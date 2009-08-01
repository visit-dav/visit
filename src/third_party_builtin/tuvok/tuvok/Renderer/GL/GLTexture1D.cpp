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
  \file    GLTexture1D.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#include "GLTexture1D.h"
#include "Controller/Controller.h"


GLTexture1D::GLTexture1D(UINT32 iSize, GLint internalformat, GLenum format,
                         GLenum type,  UINT32 iSizePerElement,
                         const GLvoid *pixels, GLint iMagFilter, GLint iMinFilter,
                         GLint wrap) :
  GLTexture(iSizePerElement),
  m_iSize(GLuint(iSize)),
  m_internalformat(internalformat),
  m_format(format),
  m_type(type)
{
  glGenTextures(1, &m_iGLID);
  glBindTexture(GL_TEXTURE_1D, m_iGLID);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, iMagFilter);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, iMinFilter);
  glTexImage1D(GL_TEXTURE_1D, 0, m_internalformat, m_iSize, 0, m_format,
               m_type, (GLvoid*)pixels);
}

void GLTexture1D::SetData(const void *pixels) {
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  MESSAGE("Uploading new %u element 1D texture.", static_cast<UINT32>(m_iSize));
  glBindTexture(GL_TEXTURE_1D, m_iGLID);
  glTexImage1D(GL_TEXTURE_1D, 0, m_internalformat, m_iSize, 0, m_format,
               m_type, (GLvoid*)pixels);
}
