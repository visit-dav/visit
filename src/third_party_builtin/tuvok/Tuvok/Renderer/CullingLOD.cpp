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
  \file    CullingLOD.cpp
  \brief    Simple conservative frustum Culling and LOD computation
  \author   Jens Krueger
            SCI Institute
            University of Utah
  \version  1.1
  \date    November 2008
*/

#include "CullingLOD.h"

using namespace std;

CullingLOD::CullingLOD(float fScreenSpaceError) :
    m_mModelViewProjectionMatrix(),
	  m_mModelViewMatrix(),
	  m_mProjectionMatrix(),
	  m_mViewMatrix(),
	  m_mModelMatrix(),
    m_fFOVY(1.0f),
    m_fAspect(1.0f),
    m_fNearPlane(0.1f),
    m_fFarPlane(100.0f),
    m_iPixelCountY(1),
    m_fScreenSpaceError(fScreenSpaceError),
    m_fLODFactor(1.0f)
{
}

void CullingLOD::SetScreenParams(float fFOVY, float fAspect, float fNearPlane, float fFarPlane, unsigned int iPixelCountY) {
  m_fFOVY = fFOVY;
  m_fAspect = fAspect;
  m_fNearPlane = fNearPlane;
  m_fFarPlane = fFarPlane;
  m_iPixelCountY = iPixelCountY;

  m_fLODFactor = 2.0f * tan(fFOVY * ((3.1416f/180.0f) / 2.0f)) * m_fScreenSpaceError / float(m_iPixelCountY);
}

FLOATVECTOR2 CullingLOD::GetDepthScaleParams() const {
  return FLOATVECTOR2(m_fFarPlane / (m_fFarPlane - m_fNearPlane), (m_fFarPlane * m_fNearPlane / (m_fNearPlane - m_fFarPlane)));
}


void CullingLOD::SetProjectionMatrix(const FLOATMATRIX4& mProjectionMatrix)
{
	m_mProjectionMatrix = mProjectionMatrix;
  m_mModelViewProjectionMatrix =  m_mModelViewMatrix * m_mProjectionMatrix;
}

void CullingLOD::SetViewMatrix(const FLOATMATRIX4& mViewMatrix) {
  m_mViewMatrix = mViewMatrix;
}

void CullingLOD::SetModelMatrix(const FLOATMATRIX4& mModelMatrix) {
  m_mModelMatrix = mModelMatrix;
}

void CullingLOD::Update()
{
  m_mModelViewMatrix = m_mModelMatrix * m_mViewMatrix;
  m_mModelViewProjectionMatrix = m_mModelViewMatrix * m_mProjectionMatrix;

	// right clip-plane
	m_Planes[0].x = -m_mModelViewProjectionMatrix.m11 + m_mModelViewProjectionMatrix.m14;
	m_Planes[0].y = -m_mModelViewProjectionMatrix.m21 + m_mModelViewProjectionMatrix.m24;
	m_Planes[0].z = -m_mModelViewProjectionMatrix.m31 + m_mModelViewProjectionMatrix.m34;
	m_Planes[0].w = -m_mModelViewProjectionMatrix.m41 + m_mModelViewProjectionMatrix.m44;
	// left clip-plane
	m_Planes[1].x = m_mModelViewProjectionMatrix.m11 + m_mModelViewProjectionMatrix.m14;
	m_Planes[1].y = m_mModelViewProjectionMatrix.m21 + m_mModelViewProjectionMatrix.m24;
	m_Planes[1].z = m_mModelViewProjectionMatrix.m31 + m_mModelViewProjectionMatrix.m34;
	m_Planes[1].w = m_mModelViewProjectionMatrix.m41 + m_mModelViewProjectionMatrix.m44;
	// top clip-plane
	m_Planes[2].x = -m_mModelViewProjectionMatrix.m12 + m_mModelViewProjectionMatrix.m14;
	m_Planes[2].y = -m_mModelViewProjectionMatrix.m22 + m_mModelViewProjectionMatrix.m24;
	m_Planes[2].z = -m_mModelViewProjectionMatrix.m32 + m_mModelViewProjectionMatrix.m34;
	m_Planes[2].w = -m_mModelViewProjectionMatrix.m42 + m_mModelViewProjectionMatrix.m44;
	// bottom clip-plane
	m_Planes[3].x = m_mModelViewProjectionMatrix.m12 + m_mModelViewProjectionMatrix.m14;
	m_Planes[3].y = m_mModelViewProjectionMatrix.m22 + m_mModelViewProjectionMatrix.m24;
	m_Planes[3].z = m_mModelViewProjectionMatrix.m32 + m_mModelViewProjectionMatrix.m34;
	m_Planes[3].w = m_mModelViewProjectionMatrix.m42 + m_mModelViewProjectionMatrix.m44;
	// far clip-plane
	m_Planes[4].x =  m_mModelViewProjectionMatrix.m13 + m_mModelViewProjectionMatrix.m14;
	m_Planes[4].y =  m_mModelViewProjectionMatrix.m23 + m_mModelViewProjectionMatrix.m24;
	m_Planes[4].z =  m_mModelViewProjectionMatrix.m33 + m_mModelViewProjectionMatrix.m34;
	m_Planes[4].w =  m_mModelViewProjectionMatrix.m43 + m_mModelViewProjectionMatrix.m44;
	// near clip-plane
	m_Planes[5].x = -m_mModelViewProjectionMatrix.m13 + m_mModelViewProjectionMatrix.m14;
	m_Planes[5].y = -m_mModelViewProjectionMatrix.m23 + m_mModelViewProjectionMatrix.m24;
	m_Planes[5].z = -m_mModelViewProjectionMatrix.m33 + m_mModelViewProjectionMatrix.m34;
	m_Planes[5].w = -m_mModelViewProjectionMatrix.m43 + m_mModelViewProjectionMatrix.m44;
}

int CullingLOD::GetLODLevel(const FLOATVECTOR3& vfCenter, const FLOATVECTOR3& vfExtent, const UINTVECTOR3& viVoxelCount) const {
  FLOATVECTOR3 vHalfExtent = 0.5f * vfExtent;
  float fLevelZeroWorldSpaceError = (vfExtent/FLOATVECTOR3(viVoxelCount)).minVal();
  
  float zCenter = (FLOATVECTOR4(vfCenter,1) * m_mModelViewMatrix).z;


  float zMinBrick = -zCenter; // TODO

  float fZ = max(m_fNearPlane, zMinBrick);

  return (int)floor(log(m_fLODFactor * fZ / fLevelZeroWorldSpaceError) / log(2.0f));
}


bool CullingLOD::IsVisible(const FLOATVECTOR3& vCenter, const FLOATVECTOR3& vfExtent)  const
{
  FLOATVECTOR3 vHalfExtent = 0.5f * vfExtent; 

	for (unsigned int uiPlane = 0; uiPlane < 6; uiPlane++)
	{
		FLOATVECTOR4 plane = m_Planes[uiPlane];
		if (
			plane.x * vCenter.x + plane.y * vCenter.y + plane.z * vCenter.z + plane.w
			<= -(vHalfExtent.x * fabs(plane.x) + vHalfExtent.y * fabs(plane.y) + vHalfExtent.z * fabs(plane.z))
			)
		{
			return false;
		}
	}
	return true;
}
