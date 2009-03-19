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
  \file    DXTexture1D.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#pragma once

#if defined(_WIN32) && defined(USE_DIRECTX)

#ifndef DXTEXTURE1D_H
#define DXTEXTURE1D_H

#include "../../StdTuvokDefines.h"
#include "DXTexture.h"

class DXTexture1D : public DXTexture {
  public:

    DXTexture1D(ID3D10Device* pd3dDevice, UINT32 iSize, DXGI_FORMAT format);
    DXTexture1D(ID3D10Device* pd3dDevice, UINT32 iSize, DXGI_FORMAT format, 
                const void* pInitialData, bool bIsReadOnly=true);
    virtual ~DXTexture1D();

    virtual void SetData(const void *pData);
    virtual void Delete();

    virtual UINT64 GetCPUSize() {return UINT64(m_iSize*m_iSizePerElement);}
    virtual UINT64 GetGPUSize() {return UINT64(m_iSize*m_iSizePerElement);}

    UINT32 GetSize() const {return UINT32(m_iSize);}

  protected:
    UINT32 m_iSize;
    ID3D10Texture1D* m_pTexture;
};

#endif // DXTEXTURE1D_H

#endif // _WIN32 && USE_DIRECTX
