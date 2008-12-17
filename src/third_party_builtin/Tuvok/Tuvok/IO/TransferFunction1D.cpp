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
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    September 2008
*/

#include "TransferFunction1D.h"
#include <fstream>
#include <memory.h>

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

float TransferFunction1D::Smoothstep(float x) {
  return 3*x*x-2*x*x*x;
}

void TransferFunction1D::SetStdFunction(float fCenterPoint, float fInvGradient) {
  SetStdFunction(fCenterPoint, fInvGradient,0);
  SetStdFunction(fCenterPoint, fInvGradient,1);
  SetStdFunction(fCenterPoint, fInvGradient,2);
  SetStdFunction(fCenterPoint, fInvGradient,3);
}

void TransferFunction1D::SetStdFunction(float fCenterPoint, float fInvGradient, int iComponent) {
  size_t iCenterPoint = size_t((vColorData.size()-1) * float(std::min<float>(std::max<float>(0,fCenterPoint),1)));
  size_t iInvGradient = size_t((vColorData.size()-1) * float(std::min<float>(std::max<float>(0,fInvGradient),1)));

  size_t iRampStartPoint = (iInvGradient/2 > iCenterPoint)                      ? 0                 : iCenterPoint-(iInvGradient/2); 
  size_t iRampEndPoint   = (iInvGradient/2 + iCenterPoint > vColorData.size())  ? vColorData.size() : iCenterPoint+(iInvGradient/2);

  for (size_t i = 0;i<iRampStartPoint;i++)                  
    vColorData[i][iComponent] = 0;

  for (size_t i = iRampStartPoint;i<iRampEndPoint;i++) {
    float fValue = Smoothstep(float(i-iCenterPoint+(iInvGradient/2))/float(iInvGradient));
    vColorData[i][iComponent] = fValue;
  }

  for (size_t i = iRampEndPoint;i<vColorData.size();i++)
    vColorData[i][iComponent] = 1;
}

void TransferFunction1D::Clear() {
  for (size_t i = 0;i<vColorData.size();i++)
    vColorData[i] = FLOATVECTOR4(0,0,0,0);
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
  return true;
}

bool TransferFunction1D::Load(ifstream& file, size_t iTargetSize) {
  if (!Load(file)) {
    return false;
  } else {
    Resample(iTargetSize);
    return true;
  }
}

bool TransferFunction1D::Save(const std::string& filename) {
  ofstream file(filename.c_str());
  if (!Save(file)) return false;
  file.close();
  return true;
}

bool TransferFunction1D::Load(ifstream& file) {
  unsigned int iSize;
  file >> iSize;
  vColorData.resize(iSize);

  for(unsigned int i=0;i<vColorData.size();++i){
    for(unsigned int j=0;j<4;++j){
      file >> vColorData[i][j];
    }
  }
  return true;
}


bool TransferFunction1D::Save(ofstream& file) {
  if (!file.is_open()) return false;

  file << vColorData.size() << endl;

  for(unsigned int i=0;i<vColorData.size();++i){
    for(unsigned int j=0;j<4;++j){
      file << vColorData[i][j] << " ";
    }
    file << endl;
  }

  return true;
}


void TransferFunction1D::GetByteArray(unsigned char** pcData, unsigned char cUsedRange) {
  if (*pcData == NULL) *pcData = new unsigned char[vColorData.size()*4];

  unsigned char *pcDataIterator = *pcData;
  for (unsigned int i = 0;i<vColorData.size();i++) {
    *pcDataIterator++ = (unsigned char)(vColorData[i][0]*cUsedRange);
    *pcDataIterator++ = (unsigned char)(vColorData[i][1]*cUsedRange);
    *pcDataIterator++ = (unsigned char)(vColorData[i][2]*cUsedRange);
    *pcDataIterator++ = (unsigned char)(vColorData[i][3]*cUsedRange);
  }
}

void TransferFunction1D::GetShortArray(unsigned short** psData, unsigned short sUsedRange) {
  if (*psData == NULL) *psData = new unsigned short[vColorData.size()*4];

  unsigned short *psDataIterator = *psData;
  for (unsigned int i = 0;i<vColorData.size();i++) {
    *psDataIterator++ = (unsigned short)(vColorData[i][0]*sUsedRange);
    *psDataIterator++ = (unsigned short)(vColorData[i][1]*sUsedRange);
    *psDataIterator++ = (unsigned short)(vColorData[i][2]*sUsedRange);
    *psDataIterator++ = (unsigned short)(vColorData[i][3]*sUsedRange);
  }
}

void TransferFunction1D::GetFloatArray(float** pfData) {
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
