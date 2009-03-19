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
  \file    CullingLOD.h
  \brief   Simple routines for filename handling
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.4
  \date    July 2008
*/

#pragma once

#ifndef CULLINGLOD_H
#define CULLINGLOD_H

#include "../Basics/Vectors.h"
#include "../StdTuvokDefines.h"

class CullingLOD
{
  public:
    CullingLOD(float fScreenSpaceError=1.0f);
    void SetScreenParams(float fFOVY, float fAspect, float fNearPlane, float fFarPlane, UINT32 iPixelCountY);
    void SetProjectionMatrix(const FLOATMATRIX4& mProjectionMatrix);
    void SetModelMatrix(const FLOATMATRIX4& mModelMatrix);
    void SetViewMatrix(const FLOATMATRIX4& mViewMatrix);
    void Update();
    void SetPassAll(bool bPassAll) {m_bPassAll = bPassAll;}

    int GetLODLevel(const FLOATVECTOR3& vfCenter, const FLOATVECTOR3& vfExtent, const UINTVECTOR3& viVoxelCount) const;
    bool IsVisible(const FLOATVECTOR3& vCenter, const FLOATVECTOR3& vfExtent) const;

    FLOATVECTOR2 GetDepthScaleParams() const;
    float        GetNearPlane() const {return m_fNearPlane;}
    float        GetFarPlane() const {return m_fFarPlane;}

  private:
    FLOATMATRIX4 m_mModelViewProjectionMatrix;
    FLOATMATRIX4 m_mModelViewMatrix;
    FLOATMATRIX4 m_mProjectionMatrix;
    FLOATMATRIX4 m_mViewMatrix;
    FLOATMATRIX4 m_mModelMatrix;
    FLOATVECTOR4 m_Planes[6];
    float        m_fFOVY;
    float        m_fAspect;
    float        m_fNearPlane;
    float        m_fFarPlane;
    UINT32       m_iPixelCountY;
    float        m_fScreenSpaceError;
    float        m_fLODFactor;
    bool         m_bPassAll;
};

#endif // CULLINGLOD_H
