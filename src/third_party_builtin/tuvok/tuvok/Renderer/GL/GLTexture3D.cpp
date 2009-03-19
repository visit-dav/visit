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
  \file    GLTexture3D.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#include <cassert>
#include "GLTexture3D.h"
#include "GLDebug.h"

GLTexture3D::GLTexture3D(UINT32 iSizeX, UINT32 iSizeY, UINT32 iSizeZ,
                         GLint internalformat, GLenum format, GLenum type,
                         UINT32 iSizePerElement, const GLvoid *pixels,
                         GLint iMagFilter, GLint iMinFilter, GLint wrapX,
                         GLint wrapY, GLint wrapZ) :
  GLTexture(iSizePerElement),
  m_iSizeX(GLuint(iSizeX)),
  m_iSizeY(GLuint(iSizeY)),
  m_iSizeZ(GLuint(iSizeZ)),
  m_internalformat(internalformat),
  m_format(format),
  m_type(type)
{
  GL();
  GL(glEnable(GL_TEXTURE_3D));
  assert(glIsEnabled(GL_TEXTURE_3D) != GL_FALSE);

  glGenTextures(1, &m_iGLID);
  glBindTexture(GL_TEXTURE_3D, m_iGLID);

  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapX);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapY);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapZ);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, iMagFilter);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, iMinFilter);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage3D(GL_TEXTURE_3D, 0, m_internalformat,
               m_iSizeX, m_iSizeY, m_iSizeZ,
               0, m_format, m_type, (GLvoid*)pixels);
}

void GLTexture3D::SetData(const void *pixels) {
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glBindTexture(GL_TEXTURE_3D, m_iGLID);
  glTexImage3D(GL_TEXTURE_3D, 0, m_internalformat,
               m_iSizeX, m_iSizeY, m_iSizeZ,
               0, m_format, m_type, (GLvoid*)pixels);
}
