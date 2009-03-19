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
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    September 2008
*/

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include "SBVRGeogen.h"

#include <Basics/MathTools.h>

static bool CheckOrdering(const FLOATVECTOR3& a, const FLOATVECTOR3& b,
                          const FLOATVECTOR3& c);
static void SortPoints(std::vector<POS3TEX3_VERTEX> &fArray);

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
  m_vLODSize(1,1,1),
  m_bClipPlaneEnabled(false)
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

// Should be called after updating the world or view matrices.  Causes geometry
// to be recomputed with the updated matrices.
void SBVRGeogen::MatricesUpdated()
{
  m_matViewTransform = m_matWorld * m_matView;
  InitBBOX();
  ComputeGeometry();
}

void SBVRGeogen::SetWorld(const FLOATMATRIX4& matWorld, bool bForceUpdate) {
  if (bForceUpdate || m_matWorld != matWorld) {
    m_matWorld = matWorld;
    MatricesUpdated();
  }
}
void SBVRGeogen::SetView(const FLOATMATRIX4& mTransform,
                         bool forceUpdate)
{
  if(forceUpdate || m_matView != mTransform) {
    m_matView = mTransform;
    MatricesUpdated();
  }
}

// Finds the point with the minimum position in Z.
struct vertex_min_z : public std::binary_function<POS3TEX3_VERTEX,
                                                  POS3TEX3_VERTEX,
                                                  bool> {
  bool operator()(const POS3TEX3_VERTEX &a, const POS3TEX3_VERTEX &b) const {
    return (a.m_vPos.z < b.m_vPos.z);
  }
};

void SBVRGeogen::InitBBOX() {

  FLOATVECTOR3 vVertexScale(m_vAspect);

  m_pfBBOXVertex[0] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[0]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMax.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[1] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[1]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMax.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[2] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[2]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMax.y,m_vTexCoordMax.z));
  m_pfBBOXVertex[3] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[3]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMax.y,m_vTexCoordMax.z));
  m_pfBBOXVertex[4] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[4]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMin.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[5] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[5]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMin.y,m_vTexCoordMin.z));
  m_pfBBOXVertex[6] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[6]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMax.x,m_vTexCoordMin.y,m_vTexCoordMax.z));
  m_pfBBOXVertex[7] = POS3TEX3_VERTEX(FLOATVECTOR4(m_pfBBOXStaticVertex[7]*vVertexScale,1.0f) * m_matViewTransform, FLOATVECTOR3(m_vTexCoordMin.x,m_vTexCoordMin.y,m_vTexCoordMax.z));

  // find the minimum z value
  m_fMinZ = (*std::min_element(m_pfBBOXVertex, m_pfBBOXVertex+8,
                               vertex_min_z())).m_vPos.z;
}

static bool ComputeIntersection(float z,
                                const POS3TEX3_VERTEX &plA,
                                const POS3TEX3_VERTEX &plB,
                                std::vector<POS3TEX3_VERTEX>& vHits)
{
  /*
     returns NO INTERSECTION if the line of the 2 points a,b is
     1. in front of the intersection plane
     2. behind the intersection plane
     3. parallel to the intersection plane (both points have "pretty much" the same z)
  */
  if ((z > plA.m_vPos.z && z > plB.m_vPos.z) ||
      (z < plA.m_vPos.z && z < plB.m_vPos.z) ||
      (EpsilonEqual(plA.m_vPos.z, plB.m_vPos.z))) {
    return false;
  }

  float fAlpha = (z - plA.m_vPos.z) /
                 (plA.m_vPos.z - plB.m_vPos.z);

  POS3TEX3_VERTEX vHit;

  vHit.m_vPos.x = plA.m_vPos.x + (plA.m_vPos.x - plB.m_vPos.x) * fAlpha;
  vHit.m_vPos.y = plA.m_vPos.y + (plA.m_vPos.y - plB.m_vPos.y) * fAlpha;
  vHit.m_vPos.z = z;

  vHit.m_vTex = plA.m_vTex + (plA.m_vTex - plB.m_vTex) * fAlpha;

  vHits.push_back(vHit);

  return true;
}

// Calculates the intersection point of a line segment lb->la which crosses the
// plane with normal `n'.
static bool intersection(const POS3TEX3_VERTEX &la,
                         const POS3TEX3_VERTEX &lb,
                         const FLOATVECTOR3 &n, const float D,
                         POS3TEX3_VERTEX &hit)
{
  const FLOATVECTOR3 &va = la.m_vPos;
  const FLOATVECTOR3 &vb = lb.m_vPos;
  const float denom = n ^ (va - vb);
  if(EpsilonEqual(denom, 0.0f)) {
    return false;
  }
  const float t = ((n ^ va) + D) / denom;

  hit.m_vPos = va + (t*(vb - va));
  hit.m_vTex = la.m_vTex + t*(lb.m_vTex - la.m_vTex);

  return true;
}

// Functor to identify the point with the lowest `y' coordinate.
struct vertex_min : public std::binary_function<POS3TEX3_VERTEX,
                                                POS3TEX3_VERTEX,
                                                bool> {
  bool operator()(const POS3TEX3_VERTEX &a, const POS3TEX3_VERTEX &b) const {
    return (a.m_vPos.y < b.m_vPos.y);
  }
};

// Sorts a vector
static void SortByGradient(std::vector<POS3TEX3_VERTEX>& fArray)
{
  // move bottom element to front of array
  if(fArray.empty()) { return; }
  std::swap(fArray[0],
            *std::min_element(fArray.begin(), fArray.end(), vertex_min()));
  if(fArray.size() > 2) {
    // sort points according to gradient
    SortPoints(fArray);
  }
}

void SBVRGeogen::Triangulate(std::vector<POS3TEX3_VERTEX> &fArray) {
  SortByGradient(fArray);

  // convert to triangles
  for (UINT32 i=0; i<(fArray.size()-2) ; i++) {
    m_vSliceTriangles.push_back(fArray[0]);
    m_vSliceTriangles.push_back(fArray[i+1]);
    m_vSliceTriangles.push_back(fArray[i+2]);
  }
}

// Splits a triangle along a plane with the given normal.
// Assumes: plane's D == 0.
//          triangle does span the plane.
static std::vector<POS3TEX3_VERTEX> SplitTriangle(POS3TEX3_VERTEX a,
                                                  POS3TEX3_VERTEX b,
                                                  POS3TEX3_VERTEX c,
                                                  const VECTOR3<float> &normal,
                                                  const float D)
{
  std::vector<POS3TEX3_VERTEX> out;
  // We'll always throw away at least one of the generated triangles.
  out.reserve(2);
  float fa = (normal ^ a.m_vPos) + D;
  float fb = (normal ^ b.m_vPos) + D;
  float fc = (normal ^ c.m_vPos) + D;
  if(fabs(fa) < (2 * std::numeric_limits<float>::epsilon())) { fa = 0; }
  if(fabs(fb) < (2 * std::numeric_limits<float>::epsilon())) { fb = 0; }
  if(fabs(fc) < (2 * std::numeric_limits<float>::epsilon())) { fc = 0; }

  // rotation / mirroring.
  //            c
  //           o          Push `c' to be alone on one side of the plane, making
  //          / \         `a' and `b' on the other.  Later we'll be able to
  // plane ---------      assume that there will be an intersection with the
  //        /     \       clip plane along the lines `ac' and `bc'.  This
  //       o-------o      reduces the number of cases below.
  //      a         b

  // if fa*fc is non-negative, both have the same sign -- and thus are on the
  // same side of the plane.
  if(fa*fc >= 0) {
    std::swap(fb, fc);
    std::swap(b, c);
    std::swap(fa, fb);
    std::swap(a, b);
  } else if(fb*fc >= 0) {
    std::swap(fa, fc);
    std::swap(a, c);
    std::swap(fa, fb);
    std::swap(a, b);
  }

  // Find the intersection points.
  POS3TEX3_VERTEX A, B;
#ifdef _DEBUG
  const bool isect_a = intersection(a,c, normal,D, A);
  const bool isect_b = intersection(b,c, normal,D, B);
  assert(isect_a); // lines must cross plane
  assert(isect_b);
#else
  intersection(a,c, normal,D, A);
  intersection(b,c, normal,D, B);
#endif

  if(fc >= 0) {
    out.push_back(a); out.push_back(b); out.push_back(A);
    out.push_back(b); out.push_back(B); out.push_back(A);
  } else {
    out.push_back(A); out.push_back(B); out.push_back(c);
  }
  return out;
}

static std::vector<POS3TEX3_VERTEX>
ClipTriangles(const std::vector<POS3TEX3_VERTEX> &in,
              const VECTOR3<float> &normal, const float D)
{
  std::vector<POS3TEX3_VERTEX> out;
  assert(!in.empty() && in.size() > 2);
  out.reserve(in.size());

  for(std::vector<POS3TEX3_VERTEX>::const_iterator iter = in.begin();
      iter < (in.end()-2);
      iter += 3) {
    const POS3TEX3_VERTEX &a = (*iter);
    const POS3TEX3_VERTEX &b = (*(iter+1));
    const POS3TEX3_VERTEX &c = (*(iter+2));
    float fa = (normal ^ a.m_vPos) + D;
    float fb = (normal ^ b.m_vPos) + D;
    float fc = (normal ^ c.m_vPos) + D;
    if(fabs(fa) < (2 * std::numeric_limits<float>::epsilon())) { fa = 0; }
    if(fabs(fb) < (2 * std::numeric_limits<float>::epsilon())) { fb = 0; }
    if(fabs(fc) < (2 * std::numeric_limits<float>::epsilon())) { fc = 0; }
    if(fa >= 0 && fb >= 0 && fc >= 0) {        // trivial reject
      // discard -- i.e. do nothing / ignore tri.
    } else if(fa <= 0 && fb <= 0 && fc <= 0) { // trivial accept
      out.push_back(a);
      out.push_back(b);
      out.push_back(c);
    } else { // triangle spans plane -- must be split.
      const std::vector<POS3TEX3_VERTEX>& tris = SplitTriangle(a,b,c,
                                                               normal,D);
      assert(!tris.empty());
      assert(tris.size() <= 6); // vector is actually of points, not tris.

      for(std::vector<POS3TEX3_VERTEX>::const_iterator tri = tris.begin();
          tri != tris.end();
          ++tri) {
        out.push_back(*tri);
      }
    }
  }
  return out;
}

bool SBVRGeogen::ComputeLayerGeometry(float fDepth) {
  std::vector<POS3TEX3_VERTEX> vLayerPoints;
  vLayerPoints.reserve(12);

  ComputeIntersection(fDepth, m_pfBBOXVertex[0], m_pfBBOXVertex[1],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[1], m_pfBBOXVertex[2],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[2], m_pfBBOXVertex[3],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[3], m_pfBBOXVertex[0],
                      vLayerPoints);

  ComputeIntersection(fDepth, m_pfBBOXVertex[4], m_pfBBOXVertex[5],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[5], m_pfBBOXVertex[6],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[6], m_pfBBOXVertex[7],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[7], m_pfBBOXVertex[4],
                      vLayerPoints);

  ComputeIntersection(fDepth, m_pfBBOXVertex[4], m_pfBBOXVertex[0],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[5], m_pfBBOXVertex[1],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[6], m_pfBBOXVertex[2],
                      vLayerPoints);
  ComputeIntersection(fDepth, m_pfBBOXVertex[7], m_pfBBOXVertex[3],
                      vLayerPoints);

  if (vLayerPoints.size() <= 2) {
    return false;
  }

  Triangulate(vLayerPoints);
  return true;
}

float SBVRGeogen::GetLayerDistance() const {
  return (0.5f * 1.0f/m_fSamplingModifier * (m_vAspect/FLOATVECTOR3(m_vSize))).minVal(); //float(m_vAspect.minVal())/float(std::max(m_vSize.maxVal(),m_iMinLayers));
}


float SBVRGeogen::GetOpacityCorrection() const {
  return 1.0f/m_fSamplingModifier * (FLOATVECTOR3(m_vGlobalSize)/FLOATVECTOR3(m_vLODSize)).maxVal();//  GetLayerDistance()*m_vSize.maxVal();
}

void SBVRGeogen::ComputeGeometry() {
  m_vSliceTriangles.clear();

  float fDepth = m_fMinZ;
  float fLayerDistance = GetLayerDistance();

  while (ComputeLayerGeometry(fDepth)) fDepth += fLayerDistance;

  if(m_bClipPlaneEnabled) {
    PLANE<float> transformed = m_ClipPlane * m_matView;
    const FLOATVECTOR3 normal(transformed.xyz());
    const float d = transformed.d();
    m_vSliceTriangles = ClipTriangles(m_vSliceTriangles, normal, d);
  }
}

void SBVRGeogen::SetVolumeData(const FLOATVECTOR3& vAspect, const UINTVECTOR3& vSize) {
  m_vGlobalAspect = vAspect;
  m_vGlobalSize = vSize;
}

void SBVRGeogen::SetLODData(const UINTVECTOR3& vSize) {
  m_vLODSize = vSize;
}

#include "Controller/Controller.h"
void SBVRGeogen::SetBrickData(const FLOATVECTOR3& vAspect,
                              const UINTVECTOR3& vSize,
                              const FLOATVECTOR3& vTexCoordMin,
                              const FLOATVECTOR3& vTexCoordMax) {
  m_vAspect       = vAspect;
  m_vSize         = vSize;
  m_vTexCoordMin  = vTexCoordMin;
  m_vTexCoordMax  = vTexCoordMax;
  MESSAGE("SBVRgeom: aspect (%f,%f,%f)", vAspect.x, vAspect.y, vAspect.z);
  MESSAGE("SBVRgeom: size   (%u,%u,%u)", vSize.x, vSize.y, vSize.z);
  MESSAGE("SBVRgeom: texmin (%f,%f,%f)", vTexCoordMin.x, vTexCoordMin.y, vTexCoordMin.z);
  MESSAGE("SBVRgeom: texmax (%f,%f,%f)", vTexCoordMax.x, vTexCoordMax.y, vTexCoordMax.z);
  InitBBOX();
}

// Checks the ordering of two points relative to a third.
static bool CheckOrdering(const FLOATVECTOR3& a,
                          const FLOATVECTOR3& b,
                          const FLOATVECTOR3& c) {
  float g1 = (a.y-c.y)/(a.x-c.x),
        g2 = (b.y-c.y)/(b.x-c.x);

  if (EpsilonEqual(a.x,c.x))
    return (g2 < 0) || (EpsilonEqual(g2,0.0f) && b.x < c.x);
  if (EpsilonEqual(b.x,c.x))
    return (g1 > 0) || (EpsilonEqual(g1,0.0f) && a.x > c.x);


  if (a.x < c.x)
    if (b.x < c.x) return g1 < g2; else return false;
  else
    if (b.x < c.x) return true; else return g1 < g2;
}

/// @todo: should be replaced with std::sort.
static void SortPoints(std::vector<POS3TEX3_VERTEX> &fArray) {
  // for small arrays, this bubble sort actually beats qsort.
  for (UINT32 i= 1;i<fArray.size();++i)
    for (UINT32 j = 1;j<fArray.size()-i;++j)
      if (!CheckOrdering(fArray[j].m_vPos,fArray[j+1].m_vPos,fArray[0].m_vPos))
        std::swap(fArray[j], fArray[j+1]);
}
