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
#pragma once
#ifndef TUVOK_UVF_DATASET_H
#define TUVOK_UVF_DATASET_H

#include <vector>
#include "Dataset.h"
#include "UVF/RasterDataBlock.h"

class VolumeDatasetInfo;
class Histogram1DDataBlock;
class Histogram2DDataBlock;
class MaxMinDataBlock;
class UVF;

namespace tuvok {

class UVFDataset : public Dataset {
public:
  /// A brick key into UVF is composed of a list of LODs and a list of brick
  /// IDs.  These lists will frequently be one element (currently, they always
  /// are), but UVF uses a vector because they can be N-dimensional indices.
  typedef std::pair<std::vector<UINT64>, std::vector<UINT64> > NDBrickKey;

  UVFDataset(const std::string& strFilename, bool bVerify);
  UVFDataset();
  virtual ~UVFDataset();

  float MaxGradientMagnitude() const { return m_fMaxGradMagnitude; }

  bool IsOpen() const { return m_bIsOpen; }
  std::string Filename() const { return m_strFilename; }

  virtual UINT64VECTOR3 GetBrickSize(const BrickKey&) const;

  virtual bool GetBrick(const BrickKey& k,
                        std::vector<unsigned char>& vData) const {
    const NDBrickKey& key = this->KeyToNDKey(k);
    return m_pVolumeDataBlock->GetData(vData, key.first, key.second);
  }
  bool GetBrick(const NDBrickKey& k,
                std::vector<unsigned char>& vData) const {
    return m_pVolumeDataBlock->GetData(vData, k.first, k.second);
  }

  virtual bool Export(UINT64 iLODLevel, const std::string& targetFilename,
                      bool bAppend,
                      bool (*brickFunc)(LargeRAWFile* pSourceFile,
                                        const std::vector<UINT64> vBrickSize,
                                        const std::vector<UINT64> vBrickOffset,
                                        void* pUserContext) = NULL,
                      void *pUserContext = NULL,
                      UINT64 iOverlap=0) const;

private:
  // converts normal key scheme to ND one needed by UVF.
  NDBrickKey KeyToNDKey(const BrickKey &k) const {
    std::vector<UINT64> vLOD;
    std::vector<UINT64> vBrick;
    vLOD.push_back(k.first);
    vBrick.push_back(k.second);
    return std::make_pair(vLOD, vBrick);
  }
  bool Open(bool bVerify);

private:
  float                 m_fMaxGradMagnitude;
  RasterDataBlock*      m_pVolumeDataBlock;
  Histogram1DDataBlock* m_pHist1DDataBlock;
  Histogram2DDataBlock* m_pHist2DDataBlock;
  MaxMinDataBlock*      m_pMaxMinData;
  UVF*                  m_pDatasetFile;
  bool                  m_bIsOpen;
  std::string           m_strFilename;
};

};

#endif // TUVOK_UVF_DATASET_H
