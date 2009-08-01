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
  \file    TransferFunction1D.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    September 2008
*/

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory.h>
#include <sstream>
#include "TransferFunction1D.h"
#include "Controller/Controller.h"

using namespace std;

TransferFunction1D::TransferFunction1D(size_t iSize) :
  m_vValueBBox(0,0)
{
  Resize(iSize);
}

TransferFunction1D::TransferFunction1D(const std::string& filename) {
  Load(filename);
}

TransferFunction1D::~TransferFunction1D(void)
{
}

void TransferFunction1D::Resize(size_t iSize) {
  vColorData.resize(iSize);
}

float TransferFunction1D::Smoothstep(float x) const {
  return 3*x*x-2*x*x*x;
}

void TransferFunction1D::SetStdFunction(float fCenterPoint, float fInvGradient) {
  SetStdFunction(fCenterPoint, fInvGradient,0, false);
  SetStdFunction(fCenterPoint, fInvGradient,1, false);
  SetStdFunction(fCenterPoint, fInvGradient,2, false);
  SetStdFunction(fCenterPoint, fInvGradient,3, false);
}

void TransferFunction1D::SetStdFunction(float fCenterPoint, float fInvGradient, int iComponent, bool bInvertedStep) {
  size_t iCenterPoint = size_t((vColorData.size()-1) * float(std::min<float>(std::max<float>(0,fCenterPoint),1)));
  size_t iInvGradient = size_t((vColorData.size()-1) * float(std::min<float>(std::max<float>(0,fInvGradient),1)));

  size_t iRampStartPoint = (iInvGradient/2 > iCenterPoint)                      ? 0                 : iCenterPoint-(iInvGradient/2); 
  size_t iRampEndPoint   = (iInvGradient/2 + iCenterPoint > vColorData.size())  ? vColorData.size() : iCenterPoint+(iInvGradient/2);

  if (bInvertedStep) {
    for (size_t i = 0;i<iRampStartPoint;i++)                  
      vColorData[i][iComponent] = 1;

    for (size_t i = iRampStartPoint;i<iRampEndPoint;i++) {
      float fValue = Smoothstep(float(i-iCenterPoint+(iInvGradient/2))/float(iInvGradient));
      vColorData[i][iComponent] = 1.0f-fValue;
    }

    for (size_t i = iRampEndPoint;i<vColorData.size();i++)
      vColorData[i][iComponent] = 0;
  } else {
    for (size_t i = 0;i<iRampStartPoint;i++)                  
      vColorData[i][iComponent] = 0;

    for (size_t i = iRampStartPoint;i<iRampEndPoint;i++) {
      float fValue = Smoothstep(float(i-iCenterPoint+(iInvGradient/2))/float(iInvGradient));
      vColorData[i][iComponent] = fValue;
    }

    for (size_t i = iRampEndPoint;i<vColorData.size();i++)
      vColorData[i][iComponent] = 1;
  }

  ComputeNonZeroLimits();
}

template<typename T>
static bool is_nan(T) { std::abort(); } // Rely on specialization.
template<>
bool is_nan(float v) {
  // This is only valid for ieee754.
  return (v != v);
}

template<typename T>
static bool is_infinite(T) { abort(); } // Rely on specialization.
template<>
bool is_infinite(float v) {
  return (v ==  std::numeric_limits<float>::infinity() ||
          v == -std::numeric_limits<float>::infinity());
}

template<typename in, typename out>
static inline out
lerp(in value, in imin, in imax, out omin, out omax)
{
  out ret = out(omin + (value-imin) * (static_cast<double>(omax-omin) /
                                                      (imax-imin)));
#if 0
  // Very useful while debugging.
  if(is_nan(ret) || is_infinite(ret)) { return 0; }
#endif
  return ret;
}

/// Finds the minimum and maximum per-channel of a 4-component packed vector.
template<typename ForwardIter, typename Out>
void minmax_component4(ForwardIter first, ForwardIter last,
                       Out c_min[4], Out c_max[4])
{
  c_min[0] = c_min[1] = c_min[2] = c_min[3] = *first;
  c_max[0] = c_max[1] = c_max[2] = c_max[3] = *first;
  if(first == last) { return; }
  while(first < last) {
    if(*(first+0) < c_min[0]) { c_min[0] = *(first+0); }
    if(*(first+1) < c_min[1]) { c_min[1] = *(first+1); }
    if(*(first+2) < c_min[2]) { c_min[2] = *(first+2); }
    if(*(first+3) < c_min[3]) { c_min[3] = *(first+3); }

    if(*(first+0) > c_max[0]) { c_max[0] = *(first+0); }
    if(*(first+1) > c_max[1]) { c_max[1] = *(first+1); }
    if(*(first+2) > c_max[2]) { c_max[2] = *(first+2); }
    if(*(first+3) > c_max[3]) { c_max[3] = *(first+3); }

    // Ugh.  Bail out if incrementing the iterator would go beyond the end.
    // We'd never actually deref the iterator in that case (because of the
    // while conditional), but we hit internal libstdc++ asserts anyway.
    if(static_cast<size_t>(std::distance(first, last)) < 4) {
      break;
    }
    std::advance(first, 4);
  }
}

/// Set the transfer function from an external source.  Assumes the vector
/// has 4-components per element, in RGBA order.
void TransferFunction1D::Set(const std::vector<unsigned char>& tf)
{
  assert(!tf.empty());
  vColorData.resize(tf.size()/4);

  unsigned char tfmin[4];
  unsigned char tfmax[4];
  // A bit tricky.  We need the min/max of our vector so that we know how to
  // interpolate, but it needs to be a per-channel min/max.
  minmax_component4(tf.begin(),tf.end(), tfmin,tfmax);

  // Similarly, we need the min/max of our output format.
  const float fmin = 0.0;
  const float fmax = 1.0;

  assert(tfmin[0] <= tfmax[0]);
  assert(tfmin[1] <= tfmax[1]);
  assert(tfmin[2] <= tfmax[2]);
  assert(tfmin[3] <= tfmax[3]);

  for(size_t i=0; i < vColorData.size(); ++i) {
    vColorData[i] = FLOATVECTOR4(
      lerp(tf[4*i+0], tfmin[0],tfmax[0], fmin,fmax),
      lerp(tf[4*i+1], tfmin[1],tfmax[1], fmin,fmax),
      lerp(tf[4*i+2], tfmin[2],tfmax[2], fmin,fmax),
      lerp(tf[4*i+3], static_cast<unsigned char>(0),
                      static_cast<unsigned char>(255), fmin,fmax)
    );
  }

  ComputeNonZeroLimits();
}

void TransferFunction1D::Clear() {
  for (size_t i = 0;i<vColorData.size();i++)
    vColorData[i] = FLOATVECTOR4(0,0,0,0);

  m_vValueBBox = UINT64VECTOR2(0,0);
}

void TransferFunction1D::Resample(size_t iTargetSize) {
  size_t iSourceSize = vColorData.size();

  if (iTargetSize == iSourceSize) return;

  vector< FLOATVECTOR4 > vTmpColorData(iTargetSize);

  if (iTargetSize < iSourceSize) {
    // downsample
    size_t iFrom = 0;
    for (size_t i = 0;i<vTmpColorData.size();i++) {

      size_t iTo = iFrom + iSourceSize/iTargetSize;

      vTmpColorData[i] = 0;
      for (size_t j = iFrom;j<iTo;j++) {
        vTmpColorData[i] += vColorData[j];
      }
      vTmpColorData[i] /= float(iTo-iFrom);

      iTargetSize -= 1;
      iSourceSize -= iTo-iFrom;

      iFrom = iTo;
    }
  } else {
    // upsample
    for (size_t i = 0;i<vTmpColorData.size();i++) {
      float fPos = float(i) * float(iSourceSize-1)/float(iTargetSize);
      size_t iFloor = size_t(floor(fPos));
      size_t iCeil  = std::min(iFloor+1, vColorData.size()-1);
      float fInterp = fPos - float(iFloor);

      vTmpColorData[i] = vColorData[iFloor] * (1-fInterp) + vColorData[iCeil] * fInterp;
    }

  }

  vColorData = vTmpColorData;
  ComputeNonZeroLimits();
}

bool TransferFunction1D::Load(const std::string& filename, size_t iTargetSize) {
  if (!Load(filename)) {
    return false;
  } else {
    Resample(iTargetSize);
    return true;
  }
}


bool TransferFunction1D::Load(const std::string& filename) {
  ifstream file(filename.c_str());
  if (!Load(file)) return false;
  file.close();
  ComputeNonZeroLimits();
  return true;
}

bool TransferFunction1D::Load(std::istream& tf, size_t iTargetSize) {
  if (!Load(tf)) {
    return false;
  } else {
    Resample(iTargetSize);
    return true;
  }
}

bool TransferFunction1D::Save(const std::string& filename) const {
  ofstream file(filename.c_str());
  if (!Save(file)) return false;
  file.close();
  return true;
}

bool TransferFunction1D::Load(std::istream& tf) {
  UINT32 iSize;
  tf >> iSize;
  vColorData.resize(iSize);

  for(size_t i=0;i<vColorData.size();++i){
    for(size_t j=0;j<4;++j){
      tf >> vColorData[i][j];
    }
  }

  return true;
}


bool TransferFunction1D::Save(std::ostream& file) const {
  file << vColorData.size() << endl;

  for(size_t i=0;i<vColorData.size();++i){
    for(size_t j=0;j<4;++j){
      file << vColorData[i][j] << " ";
    }
    file << endl;
  }

  return true;
}

void TransferFunction1D::GetByteArray(std::vector<unsigned char>& vData,
                                      unsigned char cUsedRange) const {
  // bail out immediately if we've got no data
  if(vColorData.empty()) { return; }

  vData.resize(vColorData.size() * 4);

  unsigned char *pcDataIterator = &vData.at(0);
  for (size_t i = 0;i<vColorData.size();i++) {
    *pcDataIterator++ = (unsigned char)(vColorData[i][0]*cUsedRange);
    *pcDataIterator++ = (unsigned char)(vColorData[i][1]*cUsedRange);
    *pcDataIterator++ = (unsigned char)(vColorData[i][2]*cUsedRange);
    *pcDataIterator++ = (unsigned char)(vColorData[i][3]*cUsedRange);
  }
}

void TransferFunction1D::GetShortArray(unsigned short** psData,
                                       unsigned short sUsedRange) const {
  // bail out immediately if we've got no data
  if(vColorData.empty()) { return; }

  if (*psData == NULL) *psData = new unsigned short[vColorData.size()*4];

  unsigned short *psDataIterator = *psData;
  for (size_t i = 0;i<vColorData.size();i++) {
    *psDataIterator++ = (unsigned short)(vColorData[i][0]*sUsedRange);
    *psDataIterator++ = (unsigned short)(vColorData[i][1]*sUsedRange);
    *psDataIterator++ = (unsigned short)(vColorData[i][2]*sUsedRange);
    *psDataIterator++ = (unsigned short)(vColorData[i][3]*sUsedRange);
  }
}

void TransferFunction1D::GetFloatArray(float** pfData) const {
  // bail out immediately if we've got no data
  if(vColorData.empty()) { return; }

  if (*pfData == NULL) *pfData = new float[4*vColorData.size()];
  memcpy(*pfData, &pfData[0], sizeof(float)*4*vColorData.size());
}


void TransferFunction1D::ComputeNonZeroLimits() {   
  m_vValueBBox = UINT64VECTOR2(UINT64(vColorData.size()),0);

  for (size_t i = 0;i<vColorData.size();i++) {
    if (vColorData[i][3] != 0) {
      m_vValueBBox.x = MIN(m_vValueBBox.x, i);
      m_vValueBBox.y = i;
    }
  }
}
