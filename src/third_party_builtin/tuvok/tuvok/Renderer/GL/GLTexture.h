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
  \file    GLTexture.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#pragma once

#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include "../../StdTuvokDefines.h"
#include "GLObject.h"

/** \class GLTexture
 * Abstracted texture usage.
 *
 * GLTexture loads and unloads textures from the GPU. */
class GLTexture : public GLObject {
  public:
    /** Constructs an invalid texture.
     * A texture is created with an invalid GL identifier.  No data are
     * pushed to the GPU.  Derived classes are expected to generate the
     * texture in their constructor[s].
     * @param iSizePerElement bits per texel.  Used to track memory size of the
                              texture. */
    GLTexture(UINT32 iSizePerElement) : m_iGLID(UINT32_INVALID),
                                        m_iSizePerElement(iSizePerElement) {}
    virtual ~GLTexture();

    /** Removes this texture from the OpenGL context. */
    virtual void Delete();

    /** Initializes the texture data with the given data. */
    virtual void SetData(const void *pixels) = 0;
    virtual void Bind(UINT32 iUnit=0) = 0;

    /** \return The OpenGL identifier for this texture. */
    GLuint GetGLID() const {return m_iGLID;}

  protected:
    GLuint  m_iGLID;
    UINT32  m_iSizePerElement;
};

#endif // GLTEXTURE_H
