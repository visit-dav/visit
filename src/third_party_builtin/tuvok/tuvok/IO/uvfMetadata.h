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
  \file    uvfMetadata.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   UVF-specific implementation of metadata.
*/
#pragma once
#ifndef TUVOK_UVF_METADATA_H
#define TUVOK_UVF_METADATA_H

#include "Metadata.h"
#include "Basics/Vectors.h"
#include "UVF/RasterDataBlock.h"
#include "UVF/MaxMinDataBlock.h"

namespace tuvok {

class UVFMetadata : public Metadata {
  public:
    /// For UVF, a brick key has to be a list for the LOD indicators and a
    /// list of brick indices for the brick itself.
    typedef std::pair<std::vector<UINT64>,
                      std::vector<UINT64VECTOR3> > NDBrickKey;

    UVFMetadata();
    UVFMetadata(RasterDataBlock* pVolumeDataBlock,
                MaxMinDataBlock* pMaxMinData, bool bIsSameEndianness);
    virtual ~UVFMetadata() {}

    /// Brick-specific information:
    ///@{
    virtual UINT64VECTOR3 GetBrickCount(const UINT64 lod) const;
    virtual UINT64VECTOR3 GetBrickSize(const BrickKey &k) const {
      return this->GetBrickSize(this->KeyToNDKey(k));
    }
    UINT64VECTOR3 GetBrickSize(const NDBrickKey &) const;
    /// Gives the size of a brick in real space.
    virtual UINT64VECTOR3 GetEffectiveBrickSize(const BrickKey &k) const {
      return this->GetEffectiveBrickSize(this->KeyToNDKey(k));
    }
    UINT64VECTOR3 GetEffectiveBrickSize(const NDBrickKey &) const;
    ///@}

    /// Per-dataset information.
    ///@{
    virtual UINT64VECTOR3 GetDomainSize(const UINT64 lod=0) const;
    virtual UINT64VECTOR3 GetMaxBrickSize() const;
    virtual UINT64VECTOR3 GetBrickOverlapSize() const;
    virtual UINT64 GetLODLevelCount() const;
    virtual DOUBLEVECTOR3 GetScale() const;
    ///@}

    /// Data type inquiry methods.
    ///@{
    virtual UINT64 GetBitWidth() const;
    virtual UINT64 GetComponentCount() const;
    virtual bool GetIsSigned() const;
    virtual bool GetIsFloat() const;
    virtual bool IsSameEndianness() const;
    /// Unsupported for UVF!  Wouldn't be hard to implement, though, just not
    /// currently needed.
    std::pair<double,double> GetRange() const {
      assert(1 == 0);
      return std::make_pair(0.0, 0.0);
    }
    ///@}

    const std::vector<UINT64>& GetBrickCountND(const std::vector<UINT64>& vLOD)
                                               const;
    const std::vector<UINT64>& GetBrickSizeND(const std::vector<UINT64>& vLOD,
                                              const std::vector<UINT64>& vBrick
                                             ) const;
    const std::vector<UINT64>& GetDomainSizeND() const;
    const std::vector<UINT64>& GetMaxBrickSizeND() const;
    const std::vector<UINT64>& GetBrickOverlapSizeND() const;
    const std::vector<UINT64>& GetLODLevelCountND() const;
    const std::vector<double> GetScaleND() const;
    ///@}

    /// Acceleration queries.
    ///@{
    virtual bool ContainsData(const BrickKey &k, double isoval) const {
      return this->ContainsData(this->KeyToNDKey(k), isoval);
    }
    virtual bool ContainsData(const BrickKey &k,
                              double fMin,double fMax) const {
      return this->ContainsData(this->KeyToNDKey(k), fMin,fMax);
    }
    virtual bool ContainsData(const BrickKey &k, double fMin,double fMax,
                              double fMinGradient,double fMaxGradient)
                              const {
      return this->ContainsData(this->KeyToNDKey(k), fMin,fMax,
                                fMinGradient,fMaxGradient);
    }

    bool ContainsData(const NDBrickKey &, double isoval) const;
    bool ContainsData(const NDBrickKey &,
                      double fMin,double fMax) const;
    bool ContainsData(const NDBrickKey &, double fMin,double fMax,
                      double fMinGradient,double fMaxGradient) const;
    ///@}
  private:
    // converts normal key scheme to ND one needed by UVF.
    NDBrickKey KeyToNDKey(const BrickKey &k) const {
      std::vector<UINT64> vLOD;
      std::vector<UINT64VECTOR3> vBrick;
      vLOD.push_back(k.first);
      vBrick.push_back(k.second);
      return NDBrickKey(vLOD, vBrick);
    }

  private:
    RasterDataBlock* m_pVolumeDataBlock;
    MaxMinDataBlock* m_pMaxMinData;

    UINT64VECTOR3              m_aOverlap;

    /// read from the dataset
    ///@{
    bool                       m_bIsSameEndianness;
    std::vector<UINT64VECTOR3> m_aDomainSize;
    UINT64VECTOR3              m_aMaxBrickSize;
    size_t                     m_iLODLevel;
    DOUBLEVECTOR3              m_aScale;
    std::vector<UINT64VECTOR3> m_vaBrickCount;
    std::vector<std::vector<std::vector<std::vector<UINT64VECTOR3> > > >
               m_vvaBrickSize;
    /// @todo Ugh.  FIXME; we actually know the number of elements in some of
    ///       these dimensions statically.
    std::vector<std::vector<
                std::vector<
                std::vector<InternalMaxMinElement> > > > m_vvaMaxMin;
    ///@}
};

}; // namespace tuvok

#endif // TUVOK_UVF_METADATA_H
