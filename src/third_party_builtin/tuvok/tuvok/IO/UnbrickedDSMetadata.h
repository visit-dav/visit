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

#ifndef TUVOK_UNBRICKED_DS_METADATA_H
#define TUVOK_UNBRICKED_DS_METADATA_H

#include "Metadata.h"
#include "Controller/Controller.h"

namespace tuvok {

/** UnbrickedDSMetadata provides metadata information for a dataset which exists
 * entirely in memory -- not backed by a file.  These volumes do not have
 * varying LODs; just one large brick. */
class UnbrickedDSMetadata : public Metadata {
public:
  UnbrickedDSMetadata();
  virtual ~UnbrickedDSMetadata() {}

  UINT64VECTOR3 GetBrickCount(const UINT64) const {
    return UINT64VECTOR3(1,1,1);
  }
  UINT64VECTOR3 GetBrickSize(const BrickKey &) const {
    return GetDomainSize(); // one brick for the whole domain
  }
  UINT64VECTOR3 GetEffectiveBrickSize(const BrickKey &) const {
    return GetDomainSize();
  }

  UINT64VECTOR3 GetDomainSize(const UINT64 = 0) const {
    return m_vDomainSize;
  }

  UINT64VECTOR3 GetMaxBrickSize() const;
  UINT64VECTOR3 GetBrickOverlapSize() const;
  UINT64 GetLODLevelCount() const;
  DOUBLEVECTOR3 GetScale() const;

  /// Number of components per data point.
  UINT64 GetComponentCount() const {
    WARNING("Assuming single-component data.");
    return 1;
  }
  bool GetIsSigned() const {
    switch(m_dataType) {
      case MDT_FLOAT: return true;
      case MDT_BYTE:  return false;
    }
    return true;
  }
  bool GetIsFloat() const {
    if(m_dataType == MDT_FLOAT) {
      return true;
    }
    return false;
  }
  bool IsSameEndianness() const {
    return true;
  }

  bool ContainsData(const BrickKey &, double isoval) const;
  bool ContainsData(const BrickKey &,
                    double fMin,double fMax) const;
  bool ContainsData(const BrickKey &, double fMin,double fMax,
                    double fMinGradient,double fMaxGradient) const;

  void SetRange(std::pair<double,double> trouble);
  void SetRange(double, double);

  void SetDomainSize(UINT64 x, UINT64 y, UINT64 z) {
    m_vDomainSize = UINT64VECTOR3(x,y,z);
  }

  enum MD_Data_Type {
    MDT_FLOAT=0,
    MDT_BYTE,
  };
  void SetDataType(MD_Data_Type dt) { this->m_dataType = dt; }

  /// Number of bits in the data representation.
  virtual UINT64 GetBitWidth() const {
    switch(m_dataType) {
      case MDT_FLOAT: return 32;
      case MDT_BYTE:  return  8;
    }
    assert(1==0);
    return 42;
  }

private:
  UINT64VECTOR3 m_vDomainSize;
  MD_Data_Type m_dataType;
  std::pair<double, double> range;
};

}; //namespace tuvok

#endif // TUVOK_UNBRICKED_DS_METADATA_H
