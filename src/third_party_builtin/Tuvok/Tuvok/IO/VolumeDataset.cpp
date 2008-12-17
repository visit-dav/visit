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
  \file    VolumeDataset.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#include "VolumeDataset.h"
#include "IOManager.h"  // for the size defines
#include "../Controller/MasterController.h"
#include <cstdlib> 
#include <string>
#include <sstream>

using namespace std;

VolumeDatasetInfo::VolumeDatasetInfo(RasterDataBlock* pVolumeDataBlock, MaxMinDataBlock* pMaxMinData, bool bIsSameEndianess) : 
  m_pVolumeDataBlock(pVolumeDataBlock), 
  m_pMaxMinData(pMaxMinData),
  m_bIsSameEndianess(bIsSameEndianess)
{
  vector<double> vfScale;  
  size_t iSize = m_pVolumeDataBlock->ulDomainSize.size();

  // we require the data to be at least 3D
  assert(iSize >= 3);

  // we also assume that x,y,z are in the first 3 components and we have no anisotropy (i.e. ulLODLevelCount.size=1)
  m_iLODLevel = m_pVolumeDataBlock->ulLODLevelCount[0];
  for (size_t i = 0;i<3;i++) {
    m_aOverlap[i] = m_pVolumeDataBlock->ulBrickOverlap[i];
    m_aMaxBrickSize[i] = m_pVolumeDataBlock->ulBrickSize[i];
    m_aScale[i] = m_pVolumeDataBlock->dDomainTransformation[i+(iSize+1)*i];
  }

  m_vvaBrickSize.resize(m_iLODLevel);
  if (m_pMaxMinData) m_vvaMaxMin.resize(m_iLODLevel);
  for (size_t j = 0;j<m_iLODLevel;j++) {

    vector<UINT64> vLOD;  vLOD.push_back(j);
    vector<UINT64> vDomSize = m_pVolumeDataBlock->GetLODDomainSize(vLOD);
    m_aDomainSize.push_back(UINT64VECTOR3(vDomSize[0], vDomSize[1], vDomSize[2]));

    vector<UINT64> vBrickCount = m_pVolumeDataBlock->GetBrickCount(vLOD);
    m_vaBrickCount.push_back(UINT64VECTOR3(vBrickCount[0], vBrickCount[1], vBrickCount[2]));

    m_vvaBrickSize[j].resize(m_vaBrickCount[j].x);
    if (m_pMaxMinData) m_vvaMaxMin[j].resize(m_vaBrickCount[j].x);
    for (UINT64 x = 0;x<m_vaBrickCount[j].x;x++) {
      m_vvaBrickSize[j][x].resize(m_vaBrickCount[j].y);
      if (m_pMaxMinData) m_vvaMaxMin[j][x].resize(m_vaBrickCount[j].y);
      for (UINT64 y = 0;y<m_vaBrickCount[j].y;y++) {
        if (m_pMaxMinData) m_vvaMaxMin[j][x][y].resize(m_vaBrickCount[j].z);
        for (UINT64 z = 0;z<m_vaBrickCount[j].z;z++) {

          vector<UINT64> vBrick;
          vBrick.push_back(x); vBrick.push_back(y); vBrick.push_back(z);
          vector<UINT64> vBrickSize = m_pVolumeDataBlock->GetBrickSize(vLOD, vBrick);
          
          m_vvaBrickSize[j][x][y].push_back(UINT64VECTOR3(vBrickSize[0], vBrickSize[1], vBrickSize[2]));
        }
      }
    }
  }

  size_t iSerial = 0;
  if (m_pMaxMinData) {
    for (size_t j = 0;j<m_iLODLevel;j++) {
      for (UINT64 z = 0;z<m_vaBrickCount[j].z;z++) {
        for (UINT64 y = 0;y<m_vaBrickCount[j].y;y++) {
          for (UINT64 x = 0;x<m_vaBrickCount[j].x;x++) {
            m_vvaMaxMin[j][x][y][z] = m_pMaxMinData->GetValue(iSerial++);
          }
        }
      }
    }
  }

  m_vfRescale.resize(m_pVolumeDataBlock->ulDomainSemantics.size());
  for (size_t i = 0;i<m_pVolumeDataBlock->ulDomainSemantics.size();i++) {
    m_vfRescale[i] = 1.0;
  }
}

UINT64VECTOR3 VolumeDatasetInfo::GetBrickCount(const UINT64 iLOD) const {
  return m_vaBrickCount[iLOD];
}

UINT64VECTOR3 VolumeDatasetInfo::GetBrickSize(const UINT64 iLOD, const UINT64VECTOR3& vBrick) const {
  return m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z];
}

FLOATVECTOR3 VolumeDatasetInfo::GetEffectiveBrickSize(const UINT64 iLOD, const UINT64VECTOR3& vBrick) const {
  FLOATVECTOR3 vBrickSize(m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z].x,
                          m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z].y,
                          m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z].z);

  if (vBrick.x > 0) vBrickSize.x -= m_aOverlap.x/2.0f;
  if (vBrick.y > 0) vBrickSize.y -= m_aOverlap.y/2.0f;
  if (vBrick.z > 0) vBrickSize.z -= m_aOverlap.z/2.0f;

  if (vBrick.x < m_vaBrickCount[iLOD].x-1) vBrickSize.x -= m_aOverlap.x/2.0f;
  if (vBrick.y < m_vaBrickCount[iLOD].y-1) vBrickSize.y -= m_aOverlap.y/2.0f;
  if (vBrick.z < m_vaBrickCount[iLOD].z-1) vBrickSize.z -= m_aOverlap.z/2.0f;

  return vBrickSize;
}

UINT64VECTOR3 VolumeDatasetInfo::GetDomainSize(const UINT64 iLOD) const {
  return m_aDomainSize[iLOD];
}

UINT64VECTOR3 VolumeDatasetInfo::GetMaxBrickSize() const {
  return m_aMaxBrickSize;
}

UINT64VECTOR3 VolumeDatasetInfo::GetBrickOverlapSize() const {
  return m_aOverlap;
}

UINT64 VolumeDatasetInfo::GetLODLevelCount() const {
  return m_iLODLevel;
}

DOUBLEVECTOR3 VolumeDatasetInfo::GetScale() const {
  return m_aScale * DOUBLEVECTOR3(m_vfRescale[0], m_vfRescale[1], m_vfRescale[2]);
}

const vector<UINT64>& VolumeDatasetInfo::GetBrickCountND(const vector<UINT64>& vLOD) const {
  return m_pVolumeDataBlock->GetBrickCount(vLOD);
}

const vector<UINT64>& VolumeDatasetInfo::GetBrickSizeND(const vector<UINT64>& vLOD, const vector<UINT64>& vBrick) const {
  return m_pVolumeDataBlock->GetBrickSize(vLOD, vBrick);
}

const vector<UINT64>& VolumeDatasetInfo::GetDomainSizeND() const {
  return m_pVolumeDataBlock->ulDomainSize;
}

const vector<UINT64>& VolumeDatasetInfo::GetMaxBrickSizeND() const {
  return m_pVolumeDataBlock->ulBrickSize;
}

const vector<UINT64>& VolumeDatasetInfo::GetBrickOverlapSizeND() const {
  return m_pVolumeDataBlock->ulBrickOverlap;
}    

const vector<UINT64>& VolumeDatasetInfo::GetLODLevelCountND() const {
  return m_pVolumeDataBlock->ulLODLevelCount;
}

const vector<double> VolumeDatasetInfo::GetScaleND() const {
  vector<double> vfScale;  
  size_t iSize = m_pVolumeDataBlock->ulDomainSize.size();
  for (size_t i = 0;i<iSize;i++) vfScale.push_back(m_pVolumeDataBlock->dDomainTransformation[i+(iSize+1)*i] * m_vfRescale[i]);
  return vfScale;
}

bool VolumeDatasetInfo::ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick, double fMin, double fMax, double fMinGrad, double fMaxGrad) const {
  // if we have no max min data we have to assume that every block is visible
  if (!m_pMaxMinData) return true;

  const InternalMaxMinElemen& maxMinElement = m_vvaMaxMin[iLOD][vBrick.x][vBrick.y][vBrick.z];
  return (fMax >= maxMinElement.minScalar && fMin <= maxMinElement.maxScalar) && (fMaxGrad >= maxMinElement.minGradient && fMinGrad <= maxMinElement.maxGradient);
}

bool VolumeDatasetInfo::ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick, double fMin, double fMax) const {
  // if we have no max min data we have to assume that every block is visible
  if (!m_pMaxMinData) return true;

  const InternalMaxMinElemen& maxMinElement = m_vvaMaxMin[iLOD][vBrick.x][vBrick.y][vBrick.z];
  return (fMax >= maxMinElement.minScalar && fMin <= maxMinElement.maxScalar);
}

// *********************************************************************************************************************************************

VolumeDataset::VolumeDataset(const string& strFilename, bool bVerify, MasterController* pMasterController) : 
  m_pMasterController(pMasterController),
  m_pVolumeDataBlock(NULL),
  m_pHist1DDataBlock(NULL),
  m_pHist2DDataBlock(NULL),
  m_pMaxMinData(NULL),
  m_pDatasetFile(NULL),
  m_bIsOpen(false),
  m_strFilename(strFilename),
  m_pVolumeDatasetInfo(NULL),
  m_pHist1D(NULL), 
  m_pHist2D(NULL)
{
  Open(bVerify);
}

VolumeDataset::~VolumeDataset()
{
  delete m_pHist1D;
  delete m_pHist2D;
  delete m_pVolumeDatasetInfo;

  if (m_pDatasetFile != NULL) {
     m_pDatasetFile->Close();
    delete m_pDatasetFile;
  }
}



bool VolumeDataset::Open(bool bVerify) 
{
  wstring wstrFilename(m_strFilename.begin(),m_strFilename.end());
  m_pDatasetFile = new UVF(wstrFilename);
  m_bIsOpen = m_pDatasetFile->Open(bVerify);

  if (!m_bIsOpen) return false;

  UINT64 iRasterBlockIndex = UINT64(-1);
  for (size_t iBlocks = 0;iBlocks<m_pDatasetFile->GetDataBlockCount();iBlocks++) { 
    if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() == UVFTables::BS_1D_Histogram) {
      if (m_pHist1DDataBlock != NULL) {
        m_pMasterController->DebugOut()->Warning("VolumeDataset::Open","Multiple 1D Histograms found using last block.");
      }
      m_pHist1DDataBlock = (Histogram1DDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);
    } else
    if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() == UVFTables::BS_2D_Histogram) {
      if (m_pHist2DDataBlock != NULL) {
        m_pMasterController->DebugOut()->Warning("VolumeDataset::Open","Multiple 2D Histograms found using last block.");
      }
      m_pHist2DDataBlock = (Histogram2DDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);
    } else
    if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() == UVFTables::BS_MAXMIN_VALUES) {
      if (m_pMaxMinData != NULL) {
        m_pMasterController->DebugOut()->Warning("VolumeDataset::Open","Multiple MaxMinData Blocks found using last block.");
      }
      m_pMaxMinData = (MaxMinDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);
    } else
    if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() == UVFTables::BS_REG_NDIM_GRID) {
      RasterDataBlock* pVolumeDataBlock = (RasterDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);

      // check if the block is at least 3 dimensional
      if (pVolumeDataBlock->ulDomainSize.size() < 3) {
        m_pMasterController->DebugOut()->Message("VolumeDataset::Open","%i-D raster data block found in UVF file, skipping.", int(pVolumeDataBlock->ulDomainSize.size()));
        continue;
      }

      // check if the ulElementDimension = 1 e.g. we can deal with scalars and vectors
      if (pVolumeDataBlock->ulElementDimension != 1) {
        m_pMasterController->DebugOut()->Message("VolumeDataset::Open","Non scalar/vector raster data block found in UVF file, skipping.");
        continue;
      }

      // TODO: rethink this for time dependent data
      if (pVolumeDataBlock->ulLODGroups[0] != pVolumeDataBlock->ulLODGroups[1] || pVolumeDataBlock->ulLODGroups[1] != pVolumeDataBlock->ulLODGroups[2]) {
        m_pMasterController->DebugOut()->Message("VolumeDataset::Open","Raster data block with unsupported LOD layout found in UVF file, skipping.");
        continue;
      }      

      // TODO: change this if we want to support color/vector data
      // check if we have anything other than scalars 
      if (pVolumeDataBlock->ulElementDimensionSize[0] != 1) {
        m_pMasterController->DebugOut()->Message("VolumeDataset::Open","Non scalar raster data block found in UVF file, skipping.");
        continue;
      }

      // check if the data's smallest LOD level is not larger than our bricksize
      // TODO: if this fails we may want to convert the dataset
      vector<UINT64> vSmallLODBrick = pVolumeDataBlock->GetSmallestBrickSize();
      bool bToFewLODLevels = false;
      for (size_t i = 0;i<vSmallLODBrick.size();i++) {
        if (vSmallLODBrick[i] > BRICKSIZE) {
          m_pMasterController->DebugOut()->Message("VolumeDataset::Open","Raster data block with insufficient LOD levels found in UVF file, skipping.");
          bToFewLODLevels = true;
          break;
        }
      }
      if (bToFewLODLevels) continue;

      if (iRasterBlockIndex != UINT64(-1)) {
        m_pMasterController->DebugOut()->Warning("VolumeDataset::Open","Multiple volume blocks found using last block.");
      }
      iRasterBlockIndex = iBlocks;
    } else {
      m_pMasterController->DebugOut()->Message("VolumeDataset::Open","Non-volume block found in UVF file, skipping.");
    }
  }

  if (iRasterBlockIndex == UINT64(-1)) {
    m_pMasterController->DebugOut()->Error("VolumeDataset::Open","No suitable volume block found in UVF file. Check previous messages for rejected blocks.");
    return false;
  }

  m_pMasterController->DebugOut()->Message("VolumeDataset::Open","Open successfully found a suitable data block in the UVF file, analysing data...");

  m_pVolumeDataBlock = (RasterDataBlock*)m_pDatasetFile->GetDataBlock(iRasterBlockIndex);
  m_pVolumeDatasetInfo = new VolumeDatasetInfo(m_pVolumeDataBlock, m_pMaxMinData, m_pDatasetFile->GetGlobalHeader().bIsBigEndian == EndianConvert::IsBigEndian());

  stringstream sStreamDomain, sStreamBrick;

  for (size_t i = 0;i<m_pVolumeDatasetInfo->GetDomainSizeND().size();i++) {
    if (i == 0)
      sStreamDomain << m_pVolumeDatasetInfo->GetDomainSizeND()[i];
    else
      sStreamDomain << " x " << m_pVolumeDatasetInfo->GetDomainSize()[i];
  }

  vector<UINT64> vSmallLODBrick = m_pVolumeDataBlock->GetSmallestBrickSize();
  for (size_t i = 0;i<vSmallLODBrick.size();i++) {
    if (i == 0)
      sStreamBrick << vSmallLODBrick[i];
    else
      sStreamBrick << " x " << vSmallLODBrick[i];
  }

  m_pHist1D = NULL;
  if (m_pHist1DDataBlock != NULL) {
    const vector<UINT64>& vHist1D = m_pHist1DDataBlock->GetHistogram();
    m_pHist1D = new Histogram1D(vHist1D.size());
    for (size_t i = 0;i<m_pHist1D->GetSize();i++) {
      m_pHist1D->Set(i, (unsigned int)vHist1D[i]);
    }
  } else {
    // generate a zero 1D histogram (max 4k) if none is found in the file
    m_pHist1D = new Histogram1D(min(4096, 1<<m_pVolumeDatasetInfo->GetBitwith()));
    for (size_t i = 0;i<m_pHist1D->GetSize();i++) {
      m_pHist1D->Set(i, 0);
    }
  }

  m_pHist2D = NULL;
  if (m_pHist2DDataBlock != NULL) {
    const vector< vector<UINT64> >& vHist2D = m_pHist2DDataBlock->GetHistogram();

    VECTOR2<size_t> vSize(vHist2D.size(),vHist2D[0].size());

    m_pHist2D = new Histogram2D(vSize);
    for (size_t y = 0;y<m_pHist2D->GetSize().y;y++)
      for (size_t x = 0;x<m_pHist2D->GetSize().x;x++) 
        m_pHist2D->Set(x,y,(unsigned int)vHist2D[x][y]);

  } else {
    // generate a zero 2D histogram (max 4k) if none is found in the file
    m_pHist2D = new Histogram2D(VECTOR2<size_t>(256,min(4096, 1<<m_pVolumeDatasetInfo->GetBitwith())));
    for (size_t y = 0;y<m_pHist2D->GetSize().y;y++)
      for (size_t x = 0;x<m_pHist2D->GetSize().x;x++) 
        m_pHist2D->Set(x,y,0);
  }

  m_pMasterController->DebugOut()->Message("VolumeDataset::Open","  Size %s", sStreamDomain.str().c_str());
  m_pMasterController->DebugOut()->Message("VolumeDataset::Open","  %i Bit, %i components", int(m_pVolumeDatasetInfo->GetBitwith()), int(m_pVolumeDatasetInfo->GetComponentCount()));
  m_pMasterController->DebugOut()->Message("VolumeDataset::Open","  LOD down to %s found", sStreamBrick.str().c_str());

  return true;
}

UINTVECTOR3 VolumeDataset::GetBrickSize(const vector<UINT64>& vLOD, const vector<UINT64>& vBrick) {
  UINTVECTOR3 vSize;
  vector<UINT64> vSizeUVF = m_pVolumeDatasetInfo->GetBrickSizeND(vLOD, vBrick);

  // TODO: this code assumes that x,y,z are the first coords in the dataset which does not have to be, so better check this at load time
  vSize[0] = (unsigned int)(vSizeUVF[0]);
  vSize[1] = (unsigned int)(vSizeUVF[1]);
  vSize[2] = (unsigned int)(vSizeUVF[2]);

  return vSize;
}
