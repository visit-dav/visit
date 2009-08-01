/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
  \file    uvfDataset.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#include <sstream>

#include "uvfDataset.h"

#include "IOManager.h"
#include "uvfMetadata.h"
#include "Controller/Controller.h"
#include "UVF/UVF.h"
#include "UVF/Histogram1DDataBlock.h"
#include "UVF/Histogram2DDataBlock.h"

namespace tuvok {

UVFDataset::UVFDataset(const std::string& strFilename, bool bVerify) :
  m_fMaxGradMagnitude(0.0f),
  m_pVolumeDataBlock(NULL),
  m_pHist1DDataBlock(NULL),
  m_pHist2DDataBlock(NULL),
  m_pMaxMinData(NULL),
  m_pDatasetFile(NULL),
  m_bIsOpen(false),
  m_strFilename(strFilename)
{
  Open(bVerify);
}

UVFDataset::UVFDataset() :
  m_fMaxGradMagnitude(0.0f),
  m_pVolumeDataBlock(NULL),
  m_pHist1DDataBlock(NULL),
  m_pHist2DDataBlock(NULL),
  m_pMaxMinData(NULL),
  m_pDatasetFile(NULL),
  m_bIsOpen(false),
  m_strFilename("")
{
}

UVFDataset::~UVFDataset()
{
  delete m_pDatasetFile;
}

bool UVFDataset::Open(bool bVerify)
{
  std::wstring wstrFilename(m_strFilename.begin(),m_strFilename.end());
  m_pDatasetFile = new UVF(wstrFilename);
  std::string strError;
  m_bIsOpen = m_pDatasetFile->Open(true, bVerify,false, &strError);

  if (!m_bIsOpen) {
    T_ERROR(strError.c_str());
    return false;
  }

  UINT64 iRasterBlockIndex = UINT64(-1);
  for (size_t iBlocks = 0;
       iBlocks < m_pDatasetFile->GetDataBlockCount();
       iBlocks++) {
    if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() ==
        UVFTables::BS_1D_Histogram) {
      if (m_pHist1DDataBlock != NULL) {
        WARNING("Multiple 1D Histograms found using last block.");
      }
      m_pHist1DDataBlock = (Histogram1DDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);
    } else if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() ==
               UVFTables::BS_2D_Histogram) {
      if (m_pHist2DDataBlock != NULL) {
        WARNING("Multiple 2D Histograms found using last block.");
      }
      m_pHist2DDataBlock = (Histogram2DDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);
    } else if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() ==
               UVFTables::BS_MAXMIN_VALUES) {
      if (m_pMaxMinData != NULL) {
        WARNING("Multiple MaxMinData Blocks found using last block.");
      }
      m_pMaxMinData = (MaxMinDataBlock*)m_pDatasetFile->GetDataBlock(iBlocks);
    } else if (m_pDatasetFile->GetDataBlock(iBlocks)->GetBlockSemantic() ==
               UVFTables::BS_REG_NDIM_GRID) {
      const RasterDataBlock* pVolumeDataBlock =
                         static_cast<const RasterDataBlock*>
                                    (m_pDatasetFile->GetDataBlock(iBlocks));

      // check if the block is at least 3 dimensional
      if (pVolumeDataBlock->ulDomainSize.size() < 3) {
        MESSAGE("%i-D raster data block found in UVF file, skipping.",
                int(pVolumeDataBlock->ulDomainSize.size()));
        continue;
      }

      // check if the ulElementDimension = 1 e.g. we can deal with scalars and vectors
      if (pVolumeDataBlock->ulElementDimension != 1) {
        MESSAGE("Non scalar/vector raster data block found in UVF file,"
                " skipping.");
        continue;
      }

      /// \todo: rethink this for time dependent data
      if (pVolumeDataBlock->ulLODGroups[0] != pVolumeDataBlock->ulLODGroups[1] ||
          pVolumeDataBlock->ulLODGroups[1] != pVolumeDataBlock->ulLODGroups[2]) {
        MESSAGE("Raster data block with unsupported LOD layout found in "
                "UVF file, skipping.");
        continue;
      }

      /// \todo: change this if we want to support vector data
      // check if we have anything other than scalars or color
      if (pVolumeDataBlock->ulElementDimensionSize[0] != 1 &&
          pVolumeDataBlock->ulElementDimensionSize[0] != 4) {
        MESSAGE("Skipping UVF raster data block with %u elements; "
                "only know how to handle scalar and color data.",
                pVolumeDataBlock->ulElementDimensionSize[0]);
        continue;
      }

      // check if the data's smallest LOD level is not larger than our bricksize
      /// \todo: if this fails we may want to convert the dataset
      std::vector<UINT64> vSmallLODBrick = pVolumeDataBlock->GetSmallestBrickSize();
      bool bToFewLODLevels = false;
      for (size_t i = 0;i<vSmallLODBrick.size();i++) {
        if (vSmallLODBrick[i] > BRICKSIZE) {
          MESSAGE("Raster data block with insufficient LOD levels found in "
                  "UVF file, skipping.");
          bToFewLODLevels = true;
          break;
        }
      }
      if (bToFewLODLevels) continue;

      if (iRasterBlockIndex != UINT64(-1)) {
        WARNING("Multiple volume blocks found using last block.");
      }
      iRasterBlockIndex = iBlocks;
    } else {
      MESSAGE("Non-volume block found in UVF file, skipping.");
    }
  }

  if (iRasterBlockIndex == UINT64(-1)) {
    T_ERROR("No suitable volume block found in UVF file.  Check previous "
            "messages for rejected blocks.");
    return false;
  }

  MESSAGE("Open successfully found a suitable data block in the UVF file,"
          " analyzing data...");

  m_pVolumeDataBlock = (RasterDataBlock*)m_pDatasetFile->GetDataBlock(iRasterBlockIndex);
  m_pVolumeDatasetInfo = new UVFMetadata(
                           m_pVolumeDataBlock,
                           m_pMaxMinData,
                           m_pDatasetFile->GetGlobalHeader().bIsBigEndian ==
                            EndianConvert::IsBigEndian());

  std::stringstream sStreamDomain, sStreamBrick;
  const UVFMetadata *md = dynamic_cast<const UVFMetadata*>
                                      (m_pVolumeDatasetInfo);

  for (size_t i = 0;i<md->GetDomainSizeND().size();i++) {
    if (i == 0)
      sStreamDomain << md->GetDomainSizeND()[i];
    else
      sStreamDomain << " x " << m_pVolumeDatasetInfo->GetDomainSize()[i];
  }

  std::vector<UINT64> vSmallLODBrick = m_pVolumeDataBlock->GetSmallestBrickSize();
  for (size_t i = 0;i<vSmallLODBrick.size();i++) {
    if (i == 0)
      sStreamBrick << vSmallLODBrick[i];
    else
      sStreamBrick << " x " << vSmallLODBrick[i];
  }

  m_pHist1D = NULL;
  if (m_pHist1DDataBlock != NULL) {
    const std::vector<UINT64>& vHist1D = m_pHist1DDataBlock->GetHistogram();
    m_pHist1D = new Histogram1D(vHist1D.size());
    for (size_t i = 0;i<m_pHist1D->GetSize();i++) {
      m_pHist1D->Set(i, UINT32(vHist1D[i]));
    }
  } else {
    // generate a zero 1D histogram (max 4k) if none is found in the file
    m_pHist1D = new Histogram1D(std::min(4096, 1<<m_pVolumeDatasetInfo->GetBitWidth()));
    for (size_t i = 0;i<m_pHist1D->GetSize();i++) {
      // set all values to one so "getFilledsize" later does not return a
      // completely empty dataset
      m_pHist1D->Set(i, 1);
    }
  }

  m_pHist2D = NULL;
  if (m_pHist2DDataBlock != NULL) {
    const std::vector< std::vector<UINT64> >& vHist2D = m_pHist2DDataBlock->GetHistogram();

    VECTOR2<size_t> vSize(vHist2D.size(),vHist2D[0].size());

    m_pHist2D = new Histogram2D(vSize);
    for (size_t y = 0;y<m_pHist2D->GetSize().y;y++)
      for (size_t x = 0;x<m_pHist2D->GetSize().x;x++)
        m_pHist2D->Set(x,y,UINT32(vHist2D[x][y]));

    m_fMaxGradMagnitude = m_pHist2DDataBlock->GetMaxGradMagnitude();
  } else {
    // generate a zero 2D histogram (max 4k) if none is found in the file
    VECTOR2<size_t> vec(256, std::min(4096,
                                      1<<m_pVolumeDatasetInfo->GetBitWidth()));
    m_pHist2D = new Histogram2D(vec);
    for (size_t y = 0;y<m_pHist2D->GetSize().y;y++) {
      for (size_t x = 0;x<m_pHist2D->GetSize().x;x++) {
        // set all values to one so "getFilledsize" later does not return a
        // completely empty dataset
        m_pHist2D->Set(x,y,1);
      }
    }

    m_fMaxGradMagnitude = 0;
  }

  MESSAGE("  Size %s", sStreamDomain.str().c_str());
  MESSAGE("  %i Bit, %i components", int(m_pVolumeDatasetInfo->GetBitWidth()),
                                     int(m_pVolumeDatasetInfo->GetComponentCount()));
  MESSAGE("  LOD down to %s found", sStreamBrick.str().c_str());

  return true;
}

UINT64VECTOR3 UVFDataset::GetBrickSize(const BrickKey&k) const
{
  const NDBrickKey& key = KeyToNDKey(k);
  const UVFMetadata *md = dynamic_cast<const UVFMetadata*>
                                      (m_pVolumeDatasetInfo);
  std::vector<UINT64> vSizeUVF = md->GetBrickSizeND(key.first, key.second);

  /// \todo: this code assumes that x,y,z are the first coords in the dataset
  // which is not strictly true; should check this at load time.
  return UINT64VECTOR3(vSizeUVF[0], vSizeUVF[1], vSizeUVF[2]);
}

bool UVFDataset::Export(UINT64 iLODLevel, const std::string& targetFilename,
                        bool bAppend,
                        bool (*brickFunc)(LargeRAWFile* pSourceFile,
                                          const std::vector<UINT64> vBrickSize,
                                          const std::vector<UINT64> vBrickOffset,
                                          void* pUserContext),
                        void *pUserContext,
                        UINT64 iOverlap) const
{
  std::vector<UINT64> vLOD; vLOD.push_back(iLODLevel);
  return m_pVolumeDataBlock->BrickedLODToFlatData(vLOD, targetFilename,
                                                  bAppend,
                                                  &Controller::Debug::Out(),
                                                  brickFunc, pUserContext,
                                                  iOverlap);
}

}; // tuvok namespace.
