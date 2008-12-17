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
  \file    GLTexture2D.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/


#pragma once

#ifndef GLTEXTURE2D_H
#define GLTEXTURE2D_H

#include "GLTexture.h"
#include "../Basics/Vectors.h"

class GLTexture2D : public GLTexture {
  public:
    GLTexture2D(GLuint iSizeX, GLuint iSizeY, GLint internalformat, GLenum format, GLenum type, 
          unsigned int iSizePerElement,
          const GLvoid *pixels = 0,
          GLint iMagFilter = GL_NEAREST, 
          GLint iMinFilter = GL_NEAREST,
          GLint wrapX = GL_CLAMP,
          GLint wrapY = GL_CLAMP);
    virtual ~GLTexture2D() {}
    
    virtual void Bind(unsigned int iUnit=0) {glActiveTextureARB(GL_TEXTURE0 + iUnit); glBindTexture(GL_TEXTURE_2D, m_iGLID);}
    virtual void SetData(const GLvoid *pixels);

    virtual UINT64 GetCPUSize() {return m_iSizeX*m_iSizeY*m_iSizePerElement/8;}
    virtual UINT64 GetGPUSize() {return m_iSizeX*m_iSizeY*m_iSizePerElement/8;}

    UINTVECTOR2 GetSize() {return UINTVECTOR2(m_iSizeX, m_iSizeY);}

  protected:
    GLuint m_iSizeX;
    GLuint m_iSizeY;
    GLint  m_internalformat;
    GLenum m_format;
    GLenum m_type;

};

#endif // GLTEXTURE2D_H
