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
  \file    SBVRGeogen.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    September 2008
*/

#include "SBVRGeogen.h"
#include <Basics/MathTools.h>

SBVRGeogen::SBVRGeogen(void) :
  m_fSamplingModifier(1.0f),
	m_fMinZ(0),
	m_vAspect(1,1,1),
	m_vSize(1,1,1),
  m_vTexCoordMin(0,0,0),
  m_vTexCoordMax(1,1,1),
  m_iMinLayers(100),
  m_vGlobalAspect(1,1,1),
  m_vGlobalSize(1,1,1),
  m_vLODSize(1,1,1)
{
	m_pfBBOXStaticVertex[0] = FLOATVECTOR3(-0.5, 0.5,-0.5);
	m_pfBBOXStaticVertex[1] = FLOATVECTOR3( 0.5, 0.5,-0.5);
	m_pfBBOXStaticVertex[2] = FLOATVECTOR3( 0.5, 0.5, 0.5);
	m_pfBBOXStaticVertex[3] = FLOATVECTOR3(-0.5, 0.5, 0.5);
	m_pfBBOXStaticVertex[4] = FLOATVECTOR3(-0.5,-0.5,-0.5);
	m_pfBBOXStaticVertex[5] = FLOATVECTOR3( 0.5,-0.5,-0.5);
	m_pfBBOXStaticVertex[6] = FLOATVECTOR3( 0.5,-0.5, 0.5);
	m_pfBBOXStaticVertex[7] = FLOATVECTOR3(-0.5,-0.5, 0.5);

	m_pfBBOXVertex[0] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[1] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[2] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[3] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[4] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[5] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[6] = FLOATVECTOR3(0,0,0);
	m_pfBBOXVertex[7] = FLOATVECTOR3(0,0,0);
}

SBVRGeogen::~SBVRGeogen(void)
{
}

void SBVRGeogen::SetTransformation(const FLOATMATRIX4& matTransform, bool bForceUpdate) {
	if (bForceUpdate || m_matTransform != matTransform)	{
		m_matTransform = matTransform;
		InitBBOX();
		ComputeGeometry();
	}
}

void SBVRGeogen::InitBBOX() {

	FLOATVECTOR3 vVertexScale(m_vAspect);

  m_pfBBOXVertex[0] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[0]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMax.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[1] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[1]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMax.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[2] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[2]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMax.y,m_vTexCoordMax.z));
  m_pfBBOXVertex[3] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[3]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMax.y,m_vTexCoordMax.z));
  m_pfBBOXVertex[4] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[4]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMin.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[5] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[5]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMin.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[6] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[6]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMin.y,m_vTexCoordMax.z));
  m_pfBBOXVertex[7] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[7]*vVertexScale,1.0f) * m_matTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMin.y,m_vTexCoordMax.z));

  
  // find the minimum z value
	m_fMinZ = m_pfBBOXVertex[0].m_vPos.z;
	
	for (int i = 1;i<8;++i) m_fMinZ= MIN(m_fMinZ, m_pfBBOXVertex[i].m_vPos.z);
}

bool SBVRGeogen::EpsilonEqual(float a, float b) {
	return fabs(a-b) < 0.00001;
}

void SBVRGeogen::ComputeIntersection(float z, uint indexA, uint indexB, POS3TEX3_VERTEX& vHit, uint &count) {
	/* 
	   return NO INTERSECTION if the line of the 2 points a,b is
	   1. in front of the intersection plane
	   2. behind the intersection plane
	   3. parallel to the intersection plane (both points have "pretty much" the same z)	
	*/  
	if ((z > m_pfBBOXVertex[indexA].m_vPos.z && z > m_pfBBOXVertex[indexB].m_vPos.z) ||
		(z < m_pfBBOXVertex[indexA].m_vPos.z && z < m_pfBBOXVertex[indexB].m_vPos.z) || 
		(EpsilonEqual(m_pfBBOXVertex[indexA].m_vPos.z,m_pfBBOXVertex[indexB].m_vPos.z))) return;

	float fAlpha = (z-m_pfBBOXVertex[indexA].m_vPos.z)/(m_pfBBOXVertex[indexA].m_vPos.z-m_pfBBOXVertex[indexB].m_vPos.z);

	vHit.m_vPos.x = m_pfBBOXVertex[indexA].m_vPos.x + (m_pfBBOXVertex[indexA].m_vPos.x-m_pfBBOXVertex[indexB].m_vPos.x)*fAlpha;
	vHit.m_vPos.y = m_pfBBOXVertex[indexA].m_vPos.y + (m_pfBBOXVertex[indexA].m_vPos.y-m_pfBBOXVertex[indexB].m_vPos.y)*fAlpha;
	vHit.m_vPos.z = z;

	vHit.m_vTex = m_pfBBOXVertex[indexA].m_vTex + (m_pfBBOXVertex[indexA].m_vTex-m_pfBBOXVertex[indexB].m_vTex)*fAlpha;

	count++;
}


bool SBVRGeogen::CheckOrdering(FLOATVECTOR3& a, FLOATVECTOR3& b, FLOATVECTOR3& c) {
	float g1 = (a[1]-c[1])/(a[0]-c[0]),
		  g2 = (b[1]-c[1])/(b[0]-c[0]);

	if (EpsilonEqual(a[0],c[0])) return (g2 < 0) || (EpsilonEqual(g2,0) && b[0] < c[0]);
	if (EpsilonEqual(b[0],c[0])) return (g1 > 0) || (EpsilonEqual(g1,0) && a[0] > c[0]);

	if (a[0] < c[0])
		if (b[0] < c[0]) return g1 < g2; else return false;
	else
		if (b[0] < c[0]) return true; else return g1 < g2;
}

void SBVRGeogen::Swap(POS3TEX3_VERTEX& a, POS3TEX3_VERTEX& b) {
	POS3TEX3_VERTEX temp(a);
	a = b;
	b = temp;
}

void SBVRGeogen::SortPoints(POS3TEX3_VERTEX fArray[12], uint iCount) {
	// use bubble sort here, because array is very small which makes bubble sort faster than QSort
	for (uint i= 1;i<iCount;++i) 
		for (uint j = 1;j<iCount-i;++j) 
			if (!CheckOrdering(fArray[j].m_vPos,fArray[j+1].m_vPos,fArray[0].m_vPos)) Swap(fArray[j],fArray[j+1]);
}


int SBVRGeogen::FindMinPoint(POS3TEX3_VERTEX fArray[12], uint iCount) {
	int iIndex = 0;
	for (uint i = 1;i<iCount;++i) if (fArray[i].m_vPos.y < fArray[iIndex].m_vPos.y) iIndex = i;
	return iIndex;
}


void SBVRGeogen::Triangulate(POS3TEX3_VERTEX fArray[12], uint iCount) {
	// move bottom element to front of array
	Swap(fArray[0],fArray[FindMinPoint(fArray,iCount)]);
	// sort points according to gradient
	SortPoints(fArray,iCount);
	
	// convert to triangles
	for (uint i = 0;i<iCount-2;i++) {
		m_vSliceTriangles.push_back(fArray[0]); 
		m_vSliceTriangles.push_back(fArray[i+1]); 
		m_vSliceTriangles.push_back(fArray[i+2]);
	}
}


uint SBVRGeogen::ComputeLayerGeometry(float fDepth, POS3TEX3_VERTEX pfLayerPoints[12]) {
	uint iCount = 0;

	ComputeIntersection(fDepth,0,1,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,1,2,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,2,3,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,3,0,pfLayerPoints[iCount],iCount);
			
	ComputeIntersection(fDepth,4,5,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,5,6,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,6,7,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,7,4,pfLayerPoints[iCount],iCount);
		
	ComputeIntersection(fDepth,4,0,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,5,1,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,6,2,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,7,3,pfLayerPoints[iCount],iCount);

	if (iCount > 2) {
		// move bottom element to front of array
		Swap(pfLayerPoints[0],pfLayerPoints[FindMinPoint(pfLayerPoints,iCount)]);
		// sort points according to gradient
		SortPoints(pfLayerPoints,iCount);
	} 

	return iCount;
}


bool SBVRGeogen::ComputeLayerGeometry(float fDepth) {
	uint iCount = 0;
	POS3TEX3_VERTEX pfLayerPoints[12];

	ComputeIntersection(fDepth,0,1,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,1,2,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,2,3,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,3,0,pfLayerPoints[iCount],iCount);
			
	ComputeIntersection(fDepth,4,5,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,5,6,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,6,7,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,7,4,pfLayerPoints[iCount],iCount);
		
	ComputeIntersection(fDepth,4,0,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,5,1,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,6,2,pfLayerPoints[iCount],iCount);
	ComputeIntersection(fDepth,7,3,pfLayerPoints[iCount],iCount);

	if (iCount > 2) {
		Triangulate(pfLayerPoints,iCount);
		return true;
	} else return false;
}

float SBVRGeogen::GetLayerDistance() {
  return (0.5f * 1.0f/m_fSamplingModifier * (m_vAspect/FLOATVECTOR3(m_vSize))).minVal(); //float(m_vAspect.minVal())/float(std::max(m_vSize.maxVal(),m_iMinLayers));
}


float SBVRGeogen::GetOpacityCorrection() {
  return 1.0f/m_fSamplingModifier * (FLOATVECTOR3(m_vGlobalSize)/FLOATVECTOR3(m_vLODSize)).maxVal();//  GetLayerDistance()*m_vSize.maxVal();
}

void SBVRGeogen::ComputeGeometry() {
	m_vSliceTriangles.clear();

	float fDepth = m_fMinZ;
  float fLayerDistance = GetLayerDistance();

	while (ComputeLayerGeometry(fDepth)) fDepth += fLayerDistance;
}

void SBVRGeogen::SetVolumeData(const FLOATVECTOR3& vAspect, const UINTVECTOR3& vSize) {
  m_vGlobalAspect = vAspect;
  m_vGlobalSize = vSize;
}

void SBVRGeogen::SetLODData(const UINTVECTOR3& vSize) {
  m_vLODSize = vSize;
}


void SBVRGeogen::SetBrickData(const FLOATVECTOR3& vAspect, const UINTVECTOR3& vSize, const FLOATVECTOR3& vTexCoordMin, const FLOATVECTOR3& vTexCoordMax) {
  m_vAspect       = vAspect; 
  m_vSize         = vSize;
  m_vTexCoordMin  = vTexCoordMin;
  m_vTexCoordMax  = vTexCoordMax;
  InitBBOX(); 
}
