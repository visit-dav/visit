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
  \file    Metadata.cpp
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Format-agnostic dataset metadata.
*/
#include "Metadata.h"

namespace tuvok {

Metadata::Metadata() :
  range(std::make_pair(0.0,0.0))
{
  m_Rescale[0] = m_Rescale[1] = m_Rescale[2] = 0.0;
};

std::pair<double,double> Metadata::GetRange() const { return this->range; }

    /// Rescaling information, for handling anisotropic datasets.
DOUBLEVECTOR3 Metadata::GetRescaleFactors() const {
  return DOUBLEVECTOR3(m_Rescale);
}

#ifdef DETECTED_OS_WINDOWS
  #pragma warning(disable:4996)
#endif

void Metadata::GetRescaleFactors(double rescale[3]) const {
  std::copy(m_Rescale, m_Rescale+3, rescale);
}

void Metadata::SetRescaleFactors(const double rescale[3]) {
  std::copy(rescale,rescale+3, this->m_Rescale);
}

#ifdef DETECTED_OS_WINDOWS
  #pragma warning(default:4996)
#endif

bool
Metadata::ContainsData(const BrickKey &key, const double range[2]) const {
  return this->ContainsData(key, range[0],range[1]);
}
bool
Metadata::ContainsData(const BrickKey &key, const DOUBLEVECTOR2& range) const {
  return this->ContainsData(key, range[0],range[1]);
}

bool Metadata::ContainsData(const BrickKey &key, const double valRange[2],
                            const double gradientRange[2]) const {
  return this->ContainsData(key, valRange[0],valRange[1],
                            gradientRange[0],gradientRange[1]);
}
bool Metadata::ContainsData(const BrickKey &key, const DOUBLEVECTOR2& valRange,
                            const DOUBLEVECTOR2& gradientRange) const {
  return this->ContainsData(key, valRange[0],valRange[1],
                            gradientRange[0],gradientRange[1]);
}

void Metadata::SetRescaleFactors(const DOUBLEVECTOR3& rescale) {
  this->m_Rescale[0] = rescale[0];
  this->m_Rescale[1] = rescale[1];
  this->m_Rescale[2] = rescale[2];
}

}; // namespace
