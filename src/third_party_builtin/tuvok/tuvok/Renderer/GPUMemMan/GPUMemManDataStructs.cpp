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

#include "GPUMemManDataStructs.h"
#include <Controller/Controller.h>

Texture3DListElem::Texture3DListElem(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo, bool bIsDownsampledTo8Bits, bool bDisableBorder, UINT64 iIntraFrameCounter, UINT64 iFrameCounter, MasterController* pMasterController) :
  pData(NULL),
  pTexture(NULL),
  pDataset(_pDataset),
  iUserCount(1),
  m_iIntraFrameCounter(iIntraFrameCounter),
  m_iFrameCounter(iFrameCounter),
  m_pMasterController(pMasterController),
  vLOD(_vLOD),
  vBrick(_vBrick),
  m_bIsPaddedToPowerOfTwo(bIsPaddedToPowerOfTwo),
  m_bIsDownsampledTo8Bits(bIsDownsampledTo8Bits),
  m_bDisableBorder(bDisableBorder)
{
  if (!CreateTexture()) {
    pTexture->Delete();
    delete pTexture;
    pTexture = NULL;
  }
}

Texture3DListElem::~Texture3DListElem() {
  FreeData();
  FreeTexture();
}

bool Texture3DListElem::Equals(const VolumeDataset* _pDataset,
                               const std::vector<UINT64>& _vLOD,
                               const std::vector<UINT64>& _vBrick,
                               bool bIsPaddedToPowerOfTwo,
                               bool bIsDownsampledTo8Bits, bool bDisableBorder)
{
  if (_pDataset != pDataset ||
      _vLOD.size() != vLOD.size() ||
      _vBrick.size() != vBrick.size() ||
      m_bIsPaddedToPowerOfTwo != bIsPaddedToPowerOfTwo ||
      m_bIsDownsampledTo8Bits != bIsDownsampledTo8Bits ||
      m_bDisableBorder != bDisableBorder) return false;

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

bool Texture3DListElem::BestMatch(const std::vector<UINT64>& vDimension, bool bIsPaddedToPowerOfTwo, bool bIsDownsampledTo8Bits, bool bDisableBorder, UINT64& iIntraFrameCounter, UINT64& iFrameCounter) {
  if (!Match(vDimension) || iUserCount > 0
      || m_bIsPaddedToPowerOfTwo != bIsPaddedToPowerOfTwo
      || m_bIsDownsampledTo8Bits != bIsDownsampledTo8Bits
      || m_bDisableBorder != bDisableBorder) return false;

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


bool Texture3DListElem::Match(const std::vector<UINT64>& vDimension) {
  if (pTexture == NULL) return false;

  const std::vector<UINT64> vSize = pDataset->GetInfo()->GetBrickSizeND(vLOD, vBrick);

  if (vDimension.size() != vSize.size()) return false;
  for (size_t i = 0;i<vSize.size();i++)   if (vSize[i] != vDimension[i]) return false;

  return true;
}

bool Texture3DListElem::Replace(VolumeDataset* _pDataset,
                                const std::vector<UINT64>& _vLOD,
                                const std::vector<UINT64>& _vBrick,
                                bool bIsPaddedToPowerOfTwo,
                                bool bIsDownsampledTo8Bits,
                                bool bDisableBorder, UINT64 iIntraFrameCounter,
                                UINT64 iFrameCounter) {
  if (pTexture == NULL) return false;

  pDataset = _pDataset;
  vLOD     = _vLOD;
  vBrick   = _vBrick;
  m_bIsPaddedToPowerOfTwo = bIsPaddedToPowerOfTwo;
  m_bIsDownsampledTo8Bits = bIsDownsampledTo8Bits;
  m_bDisableBorder        = bDisableBorder;

  m_iIntraFrameCounter = iIntraFrameCounter;
  m_iFrameCounter = iFrameCounter;

  LoadData();
  glGetError();
  pTexture->SetData(pData);
  return GL_NO_ERROR==glGetError();
}


bool Texture3DListElem::LoadData() {
  FreeData();
  return pDataset->GetBrick(&pData, vLOD, vBrick);
}

void  Texture3DListElem::FreeData() {
  delete [] pData;
  pData = NULL;
}


bool Texture3DListElem::CreateTexture(bool bDeleteOldTexture) {
  if (bDeleteOldTexture) FreeTexture();

  if (pData == NULL)
    if (!LoadData()) return false;

  const std::vector<UINT64> vSize = pDataset->GetInfo()->GetBrickSizeND(vLOD, vBrick);

  bool bToggleEndian = !pDataset->GetInfo()->IsSameEndianess();

  UINT64 iBitWidth  = pDataset->GetInfo()->GetBitWidth();
  UINT64 iCompCount = pDataset->GetInfo()->GetComponentCount();

  GLint glInternalformat;
  GLenum glFormat;
  GLenum glType;


  if (m_bIsDownsampledTo8Bits && iBitWidth != 8) {
    // here we assume that data which is not 8 bit is 16 bit
    if (iBitWidth != 16) {
      FreeData();
      return false;
    }

    unsigned char* pTmpData = new unsigned char[vSize[0]*vSize[1]*vSize[2]*iCompCount];

    size_t iMax = pDataset->Get1DHistogram()->GetFilledSize();

    for (size_t i = 0;i<vSize[0]*vSize[1]*vSize[2]*iCompCount;i++) {
      unsigned char iQuantizedVal = (unsigned char)(255.0*((unsigned short*)pData)[i]/float(iMax));
      pTmpData[i] = iQuantizedVal;
    }

    delete [] pData;
    pData = pTmpData;
    iBitWidth = 8;
  }


  switch (iCompCount) {
    case 1 : glFormat = GL_LUMINANCE; break;
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
        UINT64 iElemCount = vSize[0];
        for (size_t i = 1;i<vSize.size();i++) iElemCount *= vSize[i];
        short* pShorData = (short*)pData;
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
        FreeData();
        return false;
    }
  }

  glGetError();
  if (!m_bIsPaddedToPowerOfTwo || (MathTools::IsPow2(UINT32(vSize[0])) && MathTools::IsPow2(UINT32(vSize[1])) && MathTools::IsPow2(UINT32(vSize[2])))) {
    pTexture = new GLTexture3D(UINT32(vSize[0]), UINT32(vSize[1]), UINT32(vSize[2]), glInternalformat, glFormat, glType, UINT32(iBitWidth/8*iCompCount), pData, GL_LINEAR, GL_LINEAR, m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP, m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP, m_bDisableBorder ? GL_CLAMP_TO_EDGE : GL_CLAMP);
  } else {
    // pad the data to a power of two
    UINTVECTOR3 vPaddedSize(MathTools::NextPow2(UINT32(vSize[0])), MathTools::NextPow2(UINT32(vSize[1])), MathTools::NextPow2(UINT32(vSize[2])));

    size_t iTarget = 0;
    size_t iSource = 0;
    size_t iElementSize = iBitWidth/8*iCompCount;
    size_t iRowSizeSource = vSize[0]*iElementSize;
    size_t iRowSizeTarget = vPaddedSize[0]*iElementSize;

    unsigned char* pPaddedData = new unsigned char[iRowSizeTarget*vPaddedSize[1]*vPaddedSize[2]];
    memset(pPaddedData, 0, iRowSizeTarget*vPaddedSize[1]*vPaddedSize[2]);

    for (size_t z = 0;z<vSize[2];z++) {
      for (size_t y = 0;y<vSize[1];y++) {
        memcpy(pPaddedData+iTarget, pData+iSource, iRowSizeSource);

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
