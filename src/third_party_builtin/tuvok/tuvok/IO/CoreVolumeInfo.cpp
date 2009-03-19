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

#include "CoreVolumeInfo.h"

template <typename T>
static std::vector<T> vector3_to_vector(const VECTOR3<T> &);

CoreVolumeInfo::CoreVolumeInfo()
{
  m_aOverlap = UINT64VECTOR3(0,0,0);
  m_vfRescale.push_back(1); // data should not be scaled
  m_vfRescale.push_back(1);
  m_vfRescale.push_back(1);
  m_aScale = DOUBLEVECTOR3(1,1,1);
}

bool CoreVolumeInfo::ContainsData(const UINT64,
                                  const UINT64VECTOR3&,
                                  double) const
{
  return true;
}

bool CoreVolumeInfo::ContainsData(const UINT64,
                                  const UINT64VECTOR3&,
                                  double, double) const
{
  return true;
}

bool CoreVolumeInfo::ContainsData(const UINT64,
                                  const UINT64VECTOR3&,
                                  double, double,
                                  double, double) const
{
  return true;
}

std::vector<UINT64>
CoreVolumeInfo::GetBrickCountND(const std::vector<UINT64>&) const
{
    std::vector<UINT64> ret;
    ret.push_back(1);
    return ret;
}
std::vector<UINT64>
CoreVolumeInfo::GetBrickSizeND(const std::vector<UINT64>&,
                               const std::vector<UINT64>&) const
{
  return vector3_to_vector(GetBrickSize());
}

std::vector<UINT64>
CoreVolumeInfo::GetDomainSizeND() const
{
  return vector3_to_vector(GetDomainSize());
}

std::vector<UINT64>
CoreVolumeInfo::GetMaxBrickSizeND() const
{
  return vector3_to_vector(GetBrickSize());
}

std::vector<UINT64>
CoreVolumeInfo::GetBrickOverlapSizeND() const
{
  return vector3_to_vector(UINT64VECTOR3(1,1,1));
}

std::vector<UINT64>
CoreVolumeInfo::GetLODLevelCountND() const
{
  std::vector<UINT64> ret;
  ret.push_back(0);
  return ret;
}

std::vector<double>
CoreVolumeInfo::GetScaleND() const
{
  return vector3_to_vector(VECTOR3<double>(1.,1.,1.));
}

template <typename T>
static std::vector<T>
vector3_to_vector(const VECTOR3<T> &src)
{
  std::vector<T> target;
  target.push_back(src[0]);
  target.push_back(src[1]);
  target.push_back(src[2]);
  return target;
}
