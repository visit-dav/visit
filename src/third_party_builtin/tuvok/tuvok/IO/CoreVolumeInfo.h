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
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#pragma once

#ifndef TUVOK_CORE_VOLUME_INFO__H
#define TUVOK_CORE_VOLUME_INFO_H

#include "VolumeDataset.h"

/** CoreVolumeInfo provides metadata information for a dataset which exists
 * entirely in memory -- not backed by a file.  These volumes do not have
 * varying LODs; just one large brick. */
class CoreVolumeInfo : public VolumeDatasetInfo {
public:
  CoreVolumeInfo();
  virtual ~CoreVolumeInfo() {}

  UINT64VECTOR3 GetBrickCount(const UINT64) const {
    return UINT64VECTOR3(1,1,1);
  }
  UINT64VECTOR3 GetBrickSize(const UINT64,
                             const UINT64VECTOR3&) const {
    return GetDomainSize(); // one brick for the whole domain
  }
  UINT64VECTOR3 GetBrickSize() const {
    UINT64VECTOR3 unused(0,0,0);
    return GetBrickSize(0, unused);
  }
  FLOATVECTOR3 GetEffectiveBrickSize(const UINT64,
                                     const UINT64VECTOR3&) const {
    const UINT64VECTOR3 domain = GetDomainSize();
    return FLOATVECTOR3(static_cast<float>(domain[0]),
                        static_cast<float>(domain[1]),
                        static_cast<float>(domain[2]));
  }
  UINT64VECTOR3 GetDomainSize(const UINT64 = 0) const {
    return m_vDomainSize;
  }

  bool ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick,
                    double fIsoval) const;
  bool ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick,
                    double fMin, double fMax) const;
  bool ContainsData(const UINT64 iLOD, const UINT64VECTOR3& vBrick,
                    double fMin, double fMax,
                    double fMinGrad, double fMaxGrad) const;

  void SetDomainSize(UINT64 x, UINT64 y, UINT64 z) {
    m_vDomainSize = UINT64VECTOR3(x,y,z);
  }

  std::vector<UINT64> GetBrickCountND(const std::vector<UINT64>& vLOD) const;
  std::vector<UINT64> GetBrickSizeND(const std::vector<UINT64>& vLOD,
                                     const std::vector<UINT64>& vBrick) const;
  std::vector<UINT64> GetDomainSizeND() const;
  std::vector<UINT64> GetMaxBrickSizeND() const;
  std::vector<UINT64> GetBrickOverlapSizeND() const;
  std::vector<UINT64> GetLODLevelCountND() const;
  std::vector<double> GetScaleND() const;

  virtual UINT64 GetBitWidth() const { return 32; }
  virtual bool GetIsSigned() const { return true; }
  virtual bool GetIsFloat() const { return true; }

private:
  UINT64VECTOR3 m_vDomainSize;
};

#endif // TUVOK_CORE_VOLUME_INFO_H
