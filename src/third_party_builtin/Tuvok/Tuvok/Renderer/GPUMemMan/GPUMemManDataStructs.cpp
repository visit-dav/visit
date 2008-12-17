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

Texture3DListElem::Texture3DListElem(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo, UINT64 iIntraFrameCounter, UINT64 iFrameCounter) :
  pData(NULL),
  pTexture(NULL),
  pDataset(_pDataset),
  iUserCount(1),
  m_iIntraFrameCounter(iIntraFrameCounter),
  m_iFrameCounter(iFrameCounter),
  vLOD(_vLOD),
  vBrick(_vBrick),
  m_bIsPaddedToPowerOfTwo(bIsPaddedToPowerOfTwo)
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

bool Texture3DListElem::Equals(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo) {
  if (_pDataset != pDataset || _vLOD.size() != vLOD.size() || _vBrick.size() != vBrick.size() || m_bIsPaddedToPowerOfTwo != bIsPaddedToPowerOfTwo) return false;

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

bool Texture3DListElem::BestMatch(const std::vector<UINT64>& vDimension, bool bIsPaddedToPowerOfTwo, UINT64& iIntraFrameCounter, UINT64& iFrameCounter) {
  if (!Match(vDimension) || iUserCount > 0 || m_bIsPaddedToPowerOfTwo != bIsPaddedToPowerOfTwo) return false;

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

bool Texture3DListElem::Replace(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo, UINT64 iIntraFrameCounter, UINT64 iFrameCounter) {
  if (pTexture == NULL) return false;

  pDataset = _pDataset;
  vLOD     = _vLOD;
  vBrick   = _vBrick;
  m_bIsPaddedToPowerOfTwo = bIsPaddedToPowerOfTwo;

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

  UINT64 iBitWidth  = pDataset->GetInfo()->GetBitwith();
  UINT64 iCompCount = pDataset->GetInfo()->GetComponentCount();

  GLint glInternalformat;
  GLenum glFormat;
  GLenum glType;

  switch (iCompCount) {
    case 1 : glFormat = GL_LUMINANCE; break;
    case 3 : glFormat = GL_RGB; break;
    case 4 : glFormat = GL_RGBA; break;
    default : return false;
  }

  if (iBitWidth == 8) {
      glType = GL_UNSIGNED_BYTE;
      switch (iCompCount) {
        case 1 : glInternalformat = GL_LUMINANCE8; break;
        case 3 : glInternalformat = GL_RGB8; break;
        case 4 : glInternalformat = GL_RGBA8; break;
        default : return false;
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
        default : return false;
      }

    } else {
        return false;
    }
  }

  glGetError();
  if (!m_bIsPaddedToPowerOfTwo || (MathTools::IsPow2(vSize[0]) && MathTools::IsPow2(vSize[1]) && MathTools::IsPow2(vSize[2]))) {   
    pTexture = new GLTexture3D(GLuint(vSize[0]), GLuint(vSize[1]), GLuint(vSize[2]), glInternalformat, glFormat, glType, (unsigned int)(iBitWidth*iCompCount), pData, GL_LINEAR, GL_LINEAR);
  } else {
    // pad the data to a power of two
    UINTVECTOR3 vPaddedSize(MathTools::NextPow2(vSize[0]), MathTools::NextPow2(vSize[1]), MathTools::NextPow2(vSize[2]));

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
        
        // if the x sizes differ copy one more element to make the texture behave like clamp
   //     if (iRowSizeTarget > iRowSizeSource) memcpy(pPaddedData+iTarget+iRowSizeSource, pPaddedData+iTarget+iRowSizeSource-iElementSize, iElementSize);
        iTarget += iRowSizeTarget;
        iSource += iRowSizeSource;
      }
      if (vPaddedSize[1] > vSize[1]) {
     //   memcpy(pPaddedData+iTarget, pPaddedData+iTarget-iRowSizeTarget, iRowSizeTarget);
        iTarget += (vPaddedSize[1]-vSize[1])*iRowSizeTarget;
      }
    }
    // if the z sizes differ copy one more slice to make the texture behave like clamp
   // if (vPaddedSize[2] > vSize[2])
     // memcpy(pPaddedData+iTarget, pPaddedData+iTarget-vPaddedSize[1]*iRowSizeTarget, vPaddedSize[1]*iRowSizeTarget);

    pTexture = new GLTexture3D(GLuint(vPaddedSize[0]), GLuint(vPaddedSize[1]), GLuint(vPaddedSize[2]), glInternalformat, glFormat, glType, (unsigned int)(iBitWidth*iCompCount), pPaddedData, GL_LINEAR, GL_LINEAR);

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
