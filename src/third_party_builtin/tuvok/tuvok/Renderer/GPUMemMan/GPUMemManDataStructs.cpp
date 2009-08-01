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
  \file    GPUMemManDataStructs.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#include <cstring>
#include <new>
#include <typeinfo>
#include <IO/IOManager.h>
#include <boost/algorithm/minmax_element.hpp>
#include "GPUMemManDataStructs.h"
#include "Basics/MathTools.h"
#include "Controller/Controller.h"
#include "IO/uvfDataset.h"
#include "IO/uvfMetadata.h"
#include "Renderer/GL/GLTexture3D.h"
using namespace tuvok;


Texture3DListElem::Texture3DListElem(Dataset* _pDataset,
                                     const std::vector<UINT64>& _vLOD,
                                     const std::vector<UINT64>& _vBrick,
                                     bool bIsPaddedToPowerOfTwo,
                                     bool bIsDownsampledTo8Bits,
                                     bool bDisableBorder,
                                     UINT64 iIntraFrameCounter,
                                     UINT64 iFrameCounter,
                                     MasterController* pMasterController,
                                     const CTContext &ctx,
                                     std::vector<unsigned char>& vUploadHub) :
  pTexture(NULL),
  pDataset(_pDataset),
  iUserCount(1),
  m_iIntraFrameCounter(iIntraFrameCounter),
  m_iFrameCounter(iFrameCounter),
  m_pMasterController(pMasterController),
  m_Context(ctx),
  vLOD(_vLOD),
  vBrick(_vBrick),
  m_bIsPaddedToPowerOfTwo(bIsPaddedToPowerOfTwo),
  m_bIsDownsampledTo8Bits(bIsDownsampledTo8Bits),
  m_bDisableBorder(bDisableBorder),
  m_bUsingHub(false)
{
  if (!CreateTexture(vUploadHub) && pTexture) {
    pTexture->Delete();
    delete pTexture;
    pTexture = NULL;
  }
}

Texture3DListElem::~Texture3DListElem() {
  FreeData();
  FreeTexture();
}

bool Texture3DListElem::Equals(const Dataset* _pDataset,
                               const std::vector<UINT64>& _vLOD,
                               const std::vector<UINT64>& _vBrick,
                               bool bIsPaddedToPowerOfTwo,
                               bool bIsDownsampledTo8Bits, bool bDisableBorder,
                               const CTContext &cid)
{
  if (_pDataset != pDataset ||
      _vLOD.size() != vLOD.size() ||
      _vBrick.size() != vBrick.size() ||
      m_bIsPaddedToPowerOfTwo != bIsPaddedToPowerOfTwo ||
      m_bIsDownsampledTo8Bits != bIsDownsampledTo8Bits ||
      m_bDisableBorder != bDisableBorder ||
      m_Context != cid) {
    return false;
  }

  for (size_t i = 0;i<vLOD.size();i++)   if (vLOD[i] != _vLOD[i]) return false;
  for (size_t i = 0;i<vBrick.size();i++) if (vBrick[i] != _vBrick[i]) return false;

  return true;
}

GLTexture3D* Texture3DListElem::Access(UINT64& iIntraFrameCounter, UINT64& iFrameCounter) {
  m_iIntraFrameCounter = iIntraFrameCounter;
  m_iFrameCounter = iFrameCounter;
  iUserCount++;

  return pTexture;
}

bool Texture3DListElem::BestMatch(const std::vector<UINT64>& vDimension,
                                  bool bIsPaddedToPowerOfTwo,
                                  bool bIsDownsampledTo8Bits,
                                  bool bDisableBorder,
                                  UINT64& iIntraFrameCounter,
                                  UINT64& iFrameCounter,
                                  const CTContext &cid)
{
  if (!Match(vDimension) || iUserCount > 0
      || m_bIsPaddedToPowerOfTwo != bIsPaddedToPowerOfTwo
      || m_bIsDownsampledTo8Bits != bIsDownsampledTo8Bits
      || m_bDisableBorder != bDisableBorder
      || m_Context != cid) {
    return false;
  }

  // framewise older data as before found -> use this object
  if (iFrameCounter > m_iFrameCounter) {
      iFrameCounter = m_iFrameCounter;
      iIntraFrameCounter = m_iIntraFrameCounter;

      return true;
  }

  // framewise older data as before found -> use this object
  if (iFrameCounter == m_iFrameCounter &&
      iIntraFrameCounter < m_iIntraFrameCounter) {

      iFrameCounter = m_iFrameCounter;
      iIntraFrameCounter = m_iIntraFrameCounter;

      return true;
  }

  return false;
}

bool Texture3DListElem::BestMatch(const UINT64VECTOR3& vDimension,
                                  bool bIsPaddedToPowerOfTwo,
                                  bool bIsDownsampledTo8Bits,
                                  bool bDisableBorder,
                                  UINT64& iIntraFrameCounter,
                                  UINT64& iFrameCounter,
                                  const tuvok::CTContext &ctx) {
  std::vector<UINT64> dim(3);
  dim[0] = vDimension[0];
  dim[1] = vDimension[1];
  dim[2] = vDimension[2];
  return this->BestMatch(dim, bIsPaddedToPowerOfTwo, bIsDownsampledTo8Bits,
                         bDisableBorder, iIntraFrameCounter, iFrameCounter,
                         ctx);
}


bool Texture3DListElem::Match(const std::vector<UINT64>& vDimension) {
  if (pTexture == NULL) return false;

  const UVFMetadata& md = dynamic_cast<const UVFMetadata&>
                                      (pDataset->GetInfo());
  const std::vector<UINT64> vSize = md.GetBrickSizeND(vLOD, vBrick);

  if (vDimension.size() != vSize.size()) {
    return false;
  }
  for (size_t i=0; i < vSize.size(); i++) {
    if (vSize[i] != vDimension[i]) {
      return false;
    }
  }

  return true;
}

bool Texture3DListElem::Replace(Dataset* _pDataset,
                                const std::vector<UINT64>& _vLOD,
                                const std::vector<UINT64>& _vBrick,
                                bool bIsPaddedToPowerOfTwo,
                                bool bIsDownsampledTo8Bits,
                                bool bDisableBorder, UINT64 iIntraFrameCounter,
                                UINT64 iFrameCounter, const CTContext &cid,
                                std::vector<unsigned char>& vUploadHub) {
  if (pTexture == NULL) return false;
  if (m_Context != cid) {
    T_ERROR("Trying to replace texture in one context"
            "with a texture from a second context!");
    return false;
  }

  pDataset = _pDataset;
  vLOD     = _vLOD;
  vBrick   = _vBrick;
  m_bIsPaddedToPowerOfTwo = bIsPaddedToPowerOfTwo;
  m_bIsDownsampledTo8Bits = bIsDownsampledTo8Bits;
  m_bDisableBorder        = bDisableBorder;

  m_iIntraFrameCounter = iIntraFrameCounter;
  m_iFrameCounter = iFrameCounter;

  if (!LoadData(vUploadHub)) {
    T_ERROR("LoadData call failed, system may be out of memory");
    return false;
  }
  glGetError();  // clear gl error flags
  pTexture->SetData(m_bUsingHub ? &vUploadHub.at(0) : &vData.at(0));

  return GL_NO_ERROR==glGetError();
}


bool Texture3DListElem::LoadData(std::vector<unsigned char>& vUploadHub) {
  const Metadata& md = pDataset->GetInfo();
  const UINT64VECTOR3 brick(vBrick[0], vBrick[1], vBrick[2]);
  const UINT64VECTOR3 vSize = md.GetBrickSize(Metadata::BrickKey(vLOD[0],
                                                                 brick));
  UINT64 iByteWidth  = pDataset->GetInfo().GetBitWidth()/8;
  UINT64 iCompCount = pDataset->GetInfo().GetComponentCount();

  UINT64 iBrickSize = vSize[0]*vSize[1]*vSize[2]*iByteWidth * iCompCount;

  if (!vUploadHub.empty() && iBrickSize <= UINT64(INCORESIZE*4)) {
    m_bUsingHub = true;
    try {
      UVFDataset& ds = dynamic_cast<UVFDataset&>(*(this->pDataset));
      return ds.GetBrick(UVFDataset::NDBrickKey(vLOD, vBrick), vUploadHub);
    } catch(std::bad_cast) {
      return this->pDataset->GetBrick(Dataset::BrickKey(0,0), vUploadHub);
    }
  } else {
    try {
      UVFDataset& ds = dynamic_cast<UVFDataset&>(*(this->pDataset));
      return ds.GetBrick(UVFDataset::NDBrickKey(vLOD, vBrick), vData);
    } catch(std::bad_cast) {
      return this->pDataset->GetBrick(Dataset::BrickKey(0,0), vData);
    }
  }
}

void  Texture3DListElem::FreeData() {
  vData.resize(0);
}

bool Texture3DListElem::CreateTexture(std::vector<unsigned char>& vUploadHub,
                                      bool bDeleteOldTexture) {
  if (bDeleteOldTexture) FreeTexture();

  if (vData.empty()) {
    if (!LoadData(vUploadHub)) { return false; }
  }

  unsigned char* pRawData = (m_bUsingHub) ? &vUploadHub.at(0) : &vData.at(0);

  // Figure out how big this is going to be.
  const Metadata& md = pDataset->GetInfo();
  const UINT64VECTOR3 brick(vBrick[0], vBrick[1], vBrick[2]);
  const UINT64VECTOR3 vSize = md.GetBrickSize(Metadata::BrickKey(vLOD[0],
                                                                 brick));

  bool bToggleEndian = !pDataset->GetInfo().IsSameEndianness();
  UINT64 iBitWidth  = pDataset->GetInfo().GetBitWidth();
  UINT64 iCompCount = pDataset->GetInfo().GetComponentCount();

  MESSAGE("%llu components of width %llu", iCompCount, iBitWidth);

  GLint glInternalformat;
  GLenum glFormat;
  GLenum glType;

  if (m_bIsDownsampledTo8Bits && iBitWidth != 8) {
    // here we assume that data which is not 8 bit is 16 bit
    if (iBitWidth != 16) {
      FreeData();
      return false;
    }

    size_t iMax = pDataset->Get1DHistogram().GetFilledSize();
    MESSAGE("Downsampling to 8bits; max val: %u", static_cast<UINT32>(iMax));

    for (size_t i = 0;i<vSize[0]*vSize[1]*vSize[2]*iCompCount;i++) {
      unsigned char iQuantizedVal = (unsigned char)(255.0*((unsigned short*)pRawData)[i]/float(iMax));
      pRawData[i] = iQuantizedVal;
    }

    iBitWidth = 8;
  }


  switch (iCompCount) {
    case 1 : glFormat = GL_INTENSITY; break;
    case 3 : glFormat = GL_RGB; break;
    case 4 : glFormat = GL_RGBA; break;
    default : FreeData(); return false;
  }

  if (iBitWidth == 8) {
      glType = GL_UNSIGNED_BYTE;
      switch (iCompCount) {
        case 1 : glInternalformat = GL_LUMINANCE8; break;
        case 3 : glInternalformat = GL_RGB8; break;
        case 4 : glInternalformat = GL_RGBA8; break;
        default : FreeData(); return false;
      }
  } else {
    if (iBitWidth == 16) {
      glType = GL_UNSIGNED_SHORT;

      if (bToggleEndian) {
        /// @todo BROKEN for N-dimensional data; we're assuming we only get 3D
        /// data here.
        UINT64 iElemCount = vSize[0] * vSize[1] * vSize[2];
        short* pShorData = (short*)pRawData;
        for (UINT64 i = 0;i<iCompCount*iElemCount;i++) {
          EndianConvert::Swap<short>(pShorData+i);
        }
      }

      switch (iCompCount) {
        case 1 : glInternalformat = GL_LUMINANCE16; break;
        case 3 : glInternalformat = GL_RGB16; break;
        case 4 : glInternalformat = GL_RGBA16; break;
        default : FreeData(); return false;
      }
    } else {
      if(iBitWidth == 32) {
        MESSAGE("32bit FP dataset.");
        glType = GL_FLOAT;
        glInternalformat = 1;
        glInternalformat = GL_INTENSITY;
        glFormat = GL_RED;

        // testing hacks
        glInternalformat = GL_LUMINANCE32F_ARB;
        glFormat = GL_LUMINANCE;
        glType = GL_FLOAT;
        UINT64 iElemCount = vSize[0] * vSize[1] * vSize[2];
        float *begin = reinterpret_cast<float*>(pRawData);
        float *end = (reinterpret_cast<float*>(pRawData)) + iElemCount;
        std::pair<float*,float*> mmax = boost::minmax_element(begin,end);
        MESSAGE("GPU minmax: %5.3f, %5.3f", *mmax.first, *mmax.second);
      } else {
        T_ERROR("Cannot handle data of width %d", iBitWidth);
        FreeData();
        return false;
      }
    }
  }

  glGetError();
  if (!m_bIsPaddedToPowerOfTwo ||
      (MathTools::IsPow2(UINT32(vSize[0])) &&
       MathTools::IsPow2(UINT32(vSize[1])) &&
       MathTools::IsPow2(UINT32(vSize[2])))) {
    pTexture = new GLTexture3D(UINT32(vSize[0]), UINT32(vSize[1]),
                               UINT32(vSize[2]),
                               glInternalformat, glFormat, glType,
                               UINT32(iBitWidth/8*iCompCount), pRawData,
                               GL_LINEAR, GL_LINEAR,
                               m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP,
                               m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP,
                               m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP);
  } else {
    // pad the data to a power of two
    UINTVECTOR3 vPaddedSize(MathTools::NextPow2(UINT32(vSize[0])), MathTools::NextPow2(UINT32(vSize[1])), MathTools::NextPow2(UINT32(vSize[2])));

    size_t iTarget = 0;
    size_t iSource = 0;
    size_t iElementSize = iBitWidth/8*iCompCount;
    size_t iRowSizeSource = vSize[0]*iElementSize;
    size_t iRowSizeTarget = vPaddedSize[0]*iElementSize;

    unsigned char* pPaddedData;
    try {
      pPaddedData = new unsigned char[iRowSizeTarget *
                                      vPaddedSize[1] *
                                      vPaddedSize[2]];
    } catch(std::bad_alloc&) {
      return false;
    }
    memset(pPaddedData, 0, iRowSizeTarget*vPaddedSize[1]*vPaddedSize[2]);

    for (size_t z = 0;z<vSize[2];z++) {
      for (size_t y = 0;y<vSize[1];y++) {
        memcpy(pPaddedData+iTarget, pRawData+iSource, iRowSizeSource);

        // if the x sizes differ, duplicate the last element to make the
        // texture behave like clamp
        if (!m_bDisableBorder && iRowSizeTarget > iRowSizeSource)
          memcpy(pPaddedData+iTarget+iRowSizeSource,
                 pPaddedData+iTarget+iRowSizeSource-iElementSize,
                 iElementSize);
        iTarget += iRowSizeTarget;
        iSource += iRowSizeSource;
      }
      // if the y sizes differ, duplicate the last element to make the texture
      // behave like clamp
      if (vPaddedSize[1] > vSize[1]) {
        if (!m_bDisableBorder)
          memcpy(pPaddedData+iTarget, pPaddedData+iTarget-iRowSizeTarget, iRowSizeTarget);
        iTarget += (vPaddedSize[1]-vSize[1])*iRowSizeTarget;
      }
    }
    // if the z sizes differ, duplicate the last element to make the texture
    // behave like clamp
    if (!m_bDisableBorder && vPaddedSize[2] > vSize[2]) {
      memcpy(pPaddedData+iTarget, pPaddedData+(iTarget-vPaddedSize[1]*iRowSizeTarget), vPaddedSize[1]*iRowSizeTarget);
    }

    MESSAGE("Actually creating new texture %i x %i x %i, bitsize=%i, componentcount=%i due to compatibility settings", int(vPaddedSize[0]), int(vPaddedSize[1]), int(vPaddedSize[2]), int(iBitWidth), int(iCompCount));

    pTexture = new GLTexture3D(vPaddedSize[0], vPaddedSize[1], vPaddedSize[2], glInternalformat, glFormat, glType, UINT32(iBitWidth/8*iCompCount), pPaddedData, GL_LINEAR, GL_LINEAR, m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP, m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP, m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP);

    delete [] pPaddedData;
  }

  FreeData();
  return GL_NO_ERROR==glGetError();
}

void Texture3DListElem::FreeTexture() {
  if (pTexture != NULL) {
    pTexture->Delete();
    delete pTexture;
  }
  pTexture = NULL;
}
