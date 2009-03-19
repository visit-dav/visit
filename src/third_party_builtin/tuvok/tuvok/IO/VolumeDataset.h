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
  \file    VolumeDataset.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/


#pragma once

#ifndef VOLUMEDATASET_H
#define VOLUMEDATASET_H

#include "../StdTuvokDefines.h"
#include <string>
#include "TransferFunction1D.h"
#include "TransferFunction2D.h"
#include "../IO/UVF/UVF.h"
#include "../Basics/Vectors.h"

typedef VECTOR2<UINT64> UINT64VECTOR2;
typedef VECTOR3<UINT64> UINT64VECTOR3;
typedef VECTOR4<UINT64> UINT64VECTOR4;

class VolumeDataset;

/// A wrapper around UVF data to provide easy access to 3D-only data.  UVF data
/// is n-dimensional, making it hard to work with directly.
class VolumeDatasetInfo {
  public:
    VolumeDatasetInfo() : m_pVolumeDataBlock(NULL) {}

    void SetRescaleFactors(const DOUBLEVECTOR3& vfRescale) {
      m_vfRescale[0] = vfRescale[0];
      m_vfRescale[1] = vfRescale[1];
      m_vfRescale[2] = vfRescale[2];
    }
    DOUBLEVECTOR3 GetRescaleFactors() const {
      return DOUBLEVECTOR3(m_vfRescale[0], m_vfRescale[1], m_vfRescale[2]);
    }

    UINT64VECTOR3 GetBrickCount(const UINT64 iLOD) const;
    UINT64VECTOR3 GetBrickSize(const UINT64 iLOD,
                               const UINT64VECTOR3& vBrick) const;
    FLOATVECTOR3 GetEffectiveBrickSize(const UINT64 iLOD,
                                       const UINT64VECTOR3& vBrick) const;
    UINT64VECTOR3 GetDomainSize(const UINT64 iLOD=0) const;
    UINT64VECTOR3 GetMaxBrickSize() const;
    UINT64VECTOR3 GetBrickOverlapSize() const;
    UINT64 GetLODLevelCount() const;
    DOUBLEVECTOR3 GetScale() const;
    bool ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick,
                      double fIsoval) const;
    bool ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick,
                      double fMin, double fMax) const;
    bool ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick,
                      double fMin, double fMax,
                      double fMinGrad, double fMaxGrad) const;

    void SetRescaleFactorsND(std::vector<double> vfRescale) {
      m_vfRescale = vfRescale;
    }
    const std::vector<double>& GetRescaleFactorsND() const {
      return m_vfRescale;
    }

    const std::vector<UINT64>& GetBrickCountND(const std::vector<UINT64>& vLOD) const;
    const std::vector<UINT64>& GetBrickSizeND(const std::vector<UINT64>& vLOD,
                                              const std::vector<UINT64>& vBrick) const;
    const std::vector<UINT64>& GetDomainSizeND() const;
    const std::vector<UINT64>& GetMaxBrickSizeND() const;
    const std::vector<UINT64>& GetBrickOverlapSizeND() const;
    const std::vector<UINT64>& GetLODLevelCountND() const;
    const std::vector<double> GetScaleND() const;

    /// \todo change this if we want to support color data
    UINT64 GetBitWidth() const {
      return m_pVolumeDataBlock->ulElementBitSize[0][0];
    }

    /// \todo change this if we want to support color data
    UINT64 GetComponentCount() const {return 1;}

    bool GetIsSigned() const {return m_pVolumeDataBlock->bSignedElement[0][0];}
    bool GetIsFloat() const {
      return GetBitWidth() != m_pVolumeDataBlock->ulElementBitSize[0][0];
    }
    bool IsSameEndianess() const {return m_bIsSameEndianess;}

  private:
    VolumeDatasetInfo(RasterDataBlock* pVolumeDataBlock,
                      MaxMinDataBlock* pMaxMinData, bool bIsSameEndianess);
    RasterDataBlock*         m_pVolumeDataBlock;
    MaxMinDataBlock*         m_pMaxMinData;

    // read from the dataset
    bool                        m_bIsSameEndianess;
    std::vector<UINT64VECTOR3>  m_aDomainSize;
    UINT64VECTOR3               m_aOverlap;
    UINT64VECTOR3               m_aMaxBrickSize;
    UINT64                      m_iLODLevel;
    DOUBLEVECTOR3               m_aScale;
    std::vector<UINT64VECTOR3>  m_vaBrickCount;
    std::vector< std::vector< std::vector<std::vector<UINT64VECTOR3> > > >m_vvaBrickSize;
    std::vector< std::vector< std::vector<std::vector<InternalMaxMinElemen> > > >m_vvaMaxMin;

    // set externally by the user
    std::vector<double>         m_vfRescale;

    friend class VolumeDataset;
};

class VolumeDataset {
public:
  VolumeDataset(const std::string& strFilename, bool bVerify);
  VolumeDataset();
  virtual ~VolumeDataset();

  virtual bool IsOpen() const {return m_bIsOpen;}
  virtual std::string Filename() const {return m_strFilename;}

  const Histogram1D* Get1DHistogram() const {return m_pHist1D;}
  const Histogram2D* Get2DHistogram() const {return m_pHist2D;}
  float GetMaxGradMagnitude() const {
    return m_pHist2DDataBlock->GetMaxGradMagnitude();
  }

  VolumeDatasetInfo* GetInfo() {return m_pVolumeDatasetInfo;}
  const VolumeDatasetInfo* GetInfo() const { return m_pVolumeDatasetInfo; }

  UINTVECTOR3 GetBrickSize(const std::vector<UINT64>& vLOD,
                           const std::vector<UINT64>& vBrick) const;
  virtual bool GetBrick(unsigned char** ppData,
                        const std::vector<UINT64>& vLOD,
                        const std::vector<UINT64>& vBrick) const {
    return m_pVolumeDataBlock->GetData(ppData, vLOD, vBrick);
  }

  virtual bool Export(UINT64 iLODlevel, const std::string& strTargetFilename,
                      bool bAppend,
                      bool (*brickFunc)(LargeRAWFile* pSourceFile,
                                        const std::vector<UINT64> vBrickSize,
                                        const std::vector<UINT64> vBrickOffset,
                                        void* pUserContext) = NULL,
                      void* pUserContext = NULL,
                      UINT64 iOverlap=0) const;

protected:
  Histogram1D*        m_pHist1D;
  Histogram2D*        m_pHist2D;

private:
  RasterDataBlock*         m_pVolumeDataBlock;
  Histogram1DDataBlock*    m_pHist1DDataBlock;
  Histogram2DDataBlock*    m_pHist2DDataBlock;
  MaxMinDataBlock*         m_pMaxMinData;
  UVF*                     m_pDatasetFile;

  bool                m_bIsOpen;
  std::string         m_strFilename;
  VolumeDatasetInfo*  m_pVolumeDatasetInfo;

  bool Open(bool bVerify);
};

#endif // VOLUMEDATASET_H
