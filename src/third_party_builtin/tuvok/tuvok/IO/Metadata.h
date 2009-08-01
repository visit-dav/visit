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
  \file    Metadata.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Format-agnostic dataset metadata.
*/
#pragma once
#ifndef TUVOK_METADATA_H
#define TUVOK_METADATA_H

#include <utility>
#include "Basics/Vectors.h"
#include <StdTuvokDefines.h>

namespace tuvok {

class UnbrickedDataset;

class Metadata {
  public:
    /// A key for a brick is composed of an LOD indicator and a spatial index
    /// (x,y,z coordinate) for the brick.  The spatial index is logical, only
    /// corresponding with real space in a relative manner.
    typedef std::pair<size_t, UINT64VECTOR3> BrickKey;

    Metadata();
    virtual ~Metadata() {}

    /// Brick-specific information:
    ///@{
    /// The number of bricks at a given LOD.
    virtual UINT64VECTOR3 GetBrickCount(const UINT64 lod) const = 0;
    virtual UINT64VECTOR3 GetBrickSize(const BrickKey &) const = 0;
    /// Gives the size of a brick in real space.
    virtual UINT64VECTOR3 GetEffectiveBrickSize(const BrickKey &) const = 0;
    ///@}

    /// Per-dataset information.
    ///@{
    virtual UINT64VECTOR3 GetDomainSize(const UINT64 lod=0) const = 0;
    virtual UINT64VECTOR3 GetMaxBrickSize() const = 0;
    virtual UINT64VECTOR3 GetBrickOverlapSize() const = 0;
    virtual UINT64 GetLODLevelCount() const = 0;
    virtual DOUBLEVECTOR3 GetScale() const = 0;
    ///@}

    /// Data inquiry methods.
    ///@{
    /// Number of bits in the data representation.
    virtual UINT64 GetBitWidth() const = 0;
    /// Number of components per data point.
    virtual UINT64 GetComponentCount() const = 0;
    virtual bool GetIsSigned() const = 0;
    virtual bool GetIsFloat() const = 0;
    virtual bool IsSameEndianness() const = 0;
    // Min/Max of the data across the entire dataset
    std::pair<double,double> GetRange() const;
    ///@}

    /// Rescaling information, for handling anisotropic datasets.
    DOUBLEVECTOR3 GetRescaleFactors() const;
    void GetRescaleFactors(double[3]) const;

    /// Metadata acceleration queries.  Allows us to skip processing a brick if
    /// we know it has no values of interest, where `values of interest' are
    /// arguments.
    ///@{
    virtual bool ContainsData(const BrickKey &, double isoval) const = 0;

    virtual bool ContainsData(const BrickKey &,
                              double fMin,double fMax) const = 0;
    bool ContainsData(const BrickKey &, const double range[2]) const;
    bool ContainsData(const BrickKey &, const DOUBLEVECTOR2&) const;

    virtual bool ContainsData(const BrickKey &, double fMin,double fMax,
                              double fMinGradient,double fMaxGradient)
                              const = 0;
    bool ContainsData(const BrickKey &, const double valRange[2],
                      const double gradientRange[2]) const;
    bool ContainsData(const BrickKey &, const DOUBLEVECTOR2& valRange,
                      const DOUBLEVECTOR2& gradientRange) const;
    ///@}

    void SetRescaleFactors(const double[3]);
    void SetRescaleFactors(const DOUBLEVECTOR3&);

  protected:
    void SetRange(std::pair<double,double> r) { this->range = r; }

  private:
    double m_Rescale[3];
    std::pair<double,double> range;
};

}; // namespace tuvok

#endif // TUVOK_METADATA_H
