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
  \file    GLTexture1D.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#pragma once

#ifndef GLTEXTURE1D_H
#define GLTEXTURE1D_H

#include "GLTexture.h"

class GLTexture1D : public GLTexture {
  public:
    GLTexture1D(GLuint iSize, GLint internalformat, GLenum format, GLenum type, 
          unsigned int iSizePerElement,
          const GLvoid *pixels = 0,
          GLint iMagFilter = GL_NEAREST, 
          GLint iMinFilter = GL_NEAREST,
          GLint wrap = GL_CLAMP);
    virtual ~GLTexture1D() {}

    virtual void Bind(unsigned int iUnit=0) {
        glActiveTextureARB(GL_TEXTURE0 + iUnit);
        glBindTexture(GL_TEXTURE_1D, m_iGLID);
    }
    virtual void SetData(const GLvoid *pixels);

    virtual UINT64 GetCPUSize() {return m_iSize*m_iSizePerElement/8;}
    virtual UINT64 GetGPUSize() {return m_iSize*m_iSizePerElement/8;} 

    unsigned int GetSize() {return m_iSize;}


  protected:
    GLuint m_iSize;
    GLint  m_internalformat;
    GLenum m_format;
    GLenum m_type;
};

#endif // GLTEXTURE1D_H
