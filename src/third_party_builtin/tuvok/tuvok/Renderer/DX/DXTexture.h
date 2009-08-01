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
  \file    DXTexture.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#pragma once

#if defined(_WIN32) && defined(USE_DIRECTX)

#ifndef DXTEXTURE_H
#define DXTEXTURE_H

#include "../../StdTuvokDefines.h"
#include "DXObject.h"

/** \class DXTexture
 * Abstracted texture usage.
 *
 * DXTexture loads and unloads textures from the GPU. */
class DXTexture : public DXObject {
  public:
    /** Constructs an invalid texture.
     * A texture is created with an invalid GL identifier.  No data are
     * pushed to the GPU.  Derived classes are expected to generate the
     * texture in their constructor[s].
     * @param iSizePerElement bits per texel.  Used to track memory size of the
                              texture. */
    DXTexture(ID3D10Device* pd3dDevice, UINT32 iSizePerElement, bool bIsReadOnly) : 
                                        m_pd3dDevice(pd3dDevice),
                                        m_iSizePerElement(iSizePerElement),
                                        m_bIsReadOnly(bIsReadOnly),
                                        m_pTexture_SRV(NULL),
                                        m_pSRVarBound(NULL) {}
    virtual ~DXTexture();

    virtual void Delete();

    /** Initializes the texture data with the given data. */
    virtual void SetData(const void *pixels) = 0;

    virtual void Bind(ID3D10EffectShaderResourceVariable* pSRVar);
    virtual void UnBind();

  protected:
    ID3D10Device*                       m_pd3dDevice;
    UINT32                              m_iSizePerElement;
    bool                                m_bIsReadOnly;

    ID3D10ShaderResourceView*	          m_pTexture_SRV;
    ID3D10EffectShaderResourceVariable* m_pSRVarBound;
};

#endif // DXTEXTURE_H

#endif // _WIN32 && USE_DIRECTX
