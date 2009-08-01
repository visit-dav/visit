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
  \file    GeometryGenerator.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    Januar 2009
*/

#include "GeometryGenerator.h"
#include<cmath>
#define CONST_PI 3.14159265358979323846f 

using namespace std;

vector<Triangle> GeometryGenerator::GenArrow(float  fOverallLength, 
                                             float  fShaftToHeadRatio,
                                             float  fShaftRadius,
                                             float  fHeadRadius,
                                             UINT32 iSegments) {
  vector<Triangle> vGeometry;

  // compute helper vars  
  float fDegreePerSegment = (CONST_PI*2.0f)/float(iSegments);
  float fLengthShaft      = fOverallLength * fShaftToHeadRatio;

  // compute coords on a circle
  vector<PosNormalVertex> vDisk;
  for (UINT32 i = 0;i<iSegments;i++) {
    PosNormalVertex point(FLOATVECTOR3(cos(fDegreePerSegment*i),sin(fDegreePerSegment*i),0),
                          FLOATVECTOR3(cos(fDegreePerSegment*i),sin(fDegreePerSegment*i),0));
    vDisk.push_back(point);
  }


  // transfer one side open cylinder to target vector
  // endcap
  PosNormalVertex center(FLOATVECTOR3(0,0,0), FLOATVECTOR3(0,0,-1));
  center.m_vPos.z = -fOverallLength/2.0f;
  for (UINT32 i = 0;i<iSegments;i++) {
    PosNormalVertex a(vDisk[i].m_vPos*fShaftRadius, FLOATVECTOR3(0,0,-1));
    PosNormalVertex b(vDisk[(i+1)%iSegments].m_vPos*fShaftRadius, FLOATVECTOR3(0,0,-1));

    a.m_vPos.z = -fOverallLength/2.0f;
    b.m_vPos.z = -fOverallLength/2.0f;
    vGeometry.push_back(Triangle(center, b, a));
  }
  // shaft
  for (UINT32 i = 0;i<iSegments;i++) {
    PosNormalVertex a  = vDisk[i];
    PosNormalVertex b  = vDisk[(i+1)%iSegments];
    PosNormalVertex c1 = vDisk[i];

    a.m_vPos = a.m_vPos*fShaftRadius;
    b.m_vPos = b.m_vPos*fShaftRadius;
    c1.m_vPos = c1.m_vPos*fShaftRadius;

    a.m_vPos.z = -fOverallLength/2.0f;
    b.m_vPos.z = -fOverallLength/2.0f;
    c1.m_vPos.z = -fOverallLength/2.0f + fLengthShaft;
    vGeometry.push_back(Triangle(a, b, c1));

    PosNormalVertex c2 = vDisk[(i+1)%iSegments];
    c2.m_vPos = c2.m_vPos*fShaftRadius;
    c2.m_vPos.z = -fOverallLength/2.0f + fLengthShaft;
    vGeometry.push_back(Triangle(c1, b, c2));
  }

  // arrowhead
  center.m_vPos.z = fOverallLength/2.0f;
  center.m_vNormal = FLOATVECTOR3(0,0,1);
  for (UINT32 i = 0;i<iSegments;i++) {

    PosNormalVertex a(vDisk[i].m_vPos*fShaftRadius, FLOATVECTOR3(0,0,-1));
    PosNormalVertex b(vDisk[(i+1)%iSegments].m_vPos*fShaftRadius, FLOATVECTOR3(0,0,-1));
    PosNormalVertex c1(vDisk[i].m_vPos*fHeadRadius, FLOATVECTOR3(0,0,-1));

    a.m_vPos.z = -fOverallLength/2.0f + fLengthShaft;
    b.m_vPos.z = -fOverallLength/2.0f + fLengthShaft;
    c1.m_vPos.z = -fOverallLength/2.0f + fLengthShaft;
    vGeometry.push_back(Triangle(a, b, c1));
    
    PosNormalVertex c2(vDisk[(i+1)%iSegments].m_vPos*fHeadRadius, FLOATVECTOR3(0,0,-1));
    c2.m_vPos.z = -fOverallLength/2.0f + fLengthShaft;
    vGeometry.push_back(Triangle(c1, b, c2));

    c1.m_vNormal = vDisk[i].m_vNormal;
    c2.m_vNormal = vDisk[(i+1)%iSegments].m_vNormal;

    vGeometry.push_back(Triangle(c1, c2, center));
  }

  return vGeometry;
}
