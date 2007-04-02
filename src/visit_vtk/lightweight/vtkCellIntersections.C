/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "vtkCellIntersections.h"
#include <float.h>

#include <vtkBox.h>
#include <vtkCell.h>
#include <vtkCellType.h>
#include <vtkGenericCell.h>
#include <vtkHexahedron.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPixel.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkPolyLine.h>
#include <vtkPolyVertex.h>
#include <vtkPyramid.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkTriangle.h>
#include <vtkTriangleStrip.h>
#include <vtkVertex.h>
#include <vtkVoxel.h>
#include <vtkWedge.h>

#include <stdlib.h>
#include <math.h>

vtkCxxRevisionMacro(vtkCellIntersections, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkCellIntersections);



int
LineLineIsect(const double *, const double *, const double *, const double *, double *);

int
EdgeLineIsect(vtkCell *cell, const double *, const double *, double *);

bool 
SlabTest(const double d, const double o, const double lo, 
         const double hi, double &tnear, double &tfar);

// Construct with automatic computation of divisions, averaging
// 25 cells per bucket.
//
// Modificatons:
//   Kathleen Bonnell, Tue Jun  3 15:26:52 PDT 2003
//   Initialize MinCellLength.
//
//   Kathleen Bonnell, Wed Jun 18 18:27:18 PDT 2003 
//   Initialize triangle and quad. 
//
//   Kathleen Bonnell, Thu Nov  6 08:18:54 PST 2003 
//   Initialize UserBounds. 
//
vtkCellIntersections::vtkCellIntersections()
{
  this->triangle = vtkTriangle::New();
  this->quad = vtkQuad::New();

  this->TestCoPlanar = true;
}

//
// Modificatons:
//   Kathleen Bonnell, Wed Jun 18 18:27:18 PDT 2003 
//   Delete triangle and quad. 
//

vtkCellIntersections::~vtkCellIntersections()
{
  if (this->triangle)
    {
    this->triangle->Delete();
    this->triangle = NULL;
    }
  if (this->quad)
    {
    this->quad->Delete();
    this->quad = NULL;
    }
}

void vtkCellIntersections::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "TestCoPlanar: " 
     << this->TestCoPlanar << "\n";
}



int
vtkCellIntersections::CellIntersectWithLine(vtkCell *cell, 
    double p1[3], double p2[3], double& t, double x[3])
{
  switch(cell->GetCellType())
    {
    case VTK_EMPTY_CELL : return 0;
    case VTK_VERTEX : 
      return VertexIntersectWithLine((vtkVertex*)cell, p1, p2, t, x);
    case VTK_POLY_VERTEX : 
      return PolyVertexIntersectWithLine((vtkPolyVertex*)cell, p1, p2, t, x);
    case VTK_LINE : 
      return LineIntersectWithLine((vtkLine*)cell, p1, p2, t, x);
    case VTK_POLY_LINE : 
      return PolyLineIntersectWithLine((vtkPolyLine*)cell, p1, p2, t, x);
    case VTK_TRIANGLE : 
      return TriangleIntersectWithLine((vtkTriangle*)cell, p1, p2, t, x);
    case VTK_TRIANGLE_STRIP : 
      return TriStripIntersectWithLine((vtkTriangleStrip*)cell, p1, p2, t, x);
    case VTK_POLYGON : 
      return PolygonIntersectWithLine((vtkPolygon*)cell, p1, p2, t, x);
    case VTK_PIXEL : 
      return PixelIntersectWithLine((vtkPixel*)cell, p1, p2, t, x);
    case VTK_QUAD : 
      return QuadIntersectWithLine((vtkQuad*)cell, p1, p2, t, x);
    case VTK_TETRA : 
      return TetraIntersectWithLine((vtkTetra*)cell, p1, p2, t, x);
    case VTK_VOXEL : 
      return VoxelIntersectWithLine((vtkVoxel*)cell, p1, p2, t, x);
    case VTK_HEXAHEDRON : 
      return HexIntersectWithLine((vtkHexahedron*)cell, p1, p2, t, x);
    case VTK_WEDGE : 
      return WedgeIntersectWithLine((vtkWedge*)cell, p1, p2, t, x);
    case VTK_PYRAMID : 
      return PyramidIntersectWithLine((vtkPyramid*)cell, p1, p2, t, x);
    default:
      vtkDebugMacro( << "CellType  " << cell->GetCellType() 
                     << "not yet supported for CellIntersectWithLine ..." );
      return 0;
    }
}

int
vtkCellIntersections::VertexIntersectWithLine(vtkVertex *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  double cp[3], X[3];
  cell->Points->GetPoint(0, X);
  // DistanceToLine sets t as parametric distance.
  double dist = vtkLine::DistanceToLine(X, p1, p2, t, cp);
  t = VTK_DOUBLE_MAX;
  if (dist == 0)
    {
    // return the vertex
    for (int i = 0; i < 3; i++)
      {
      x[i] = X[i];
      }
    t = dist;
    return 1;
    }
  return 0;
}

int
vtkCellIntersections::PolyVertexIntersectWithLine(vtkPolyVertex *cell, 
    double p1[3], double p2[3], double& t, double x[3])
{
  int numPts=cell->Points->GetNumberOfPoints();
  
  vtkVertex *vertex = vtkVertex::New();
  double pt[3], tTemp, xTemp[3];
  t = VTK_DOUBLE_MAX;
  int intersection = 0;

  // if more than 1 vertex lies on the line, they will all
  // return tTemp of 0, so just take first intersection found.
  for (int subId=0; subId < numPts && !intersection; subId++)
    {
    cell->Points->GetPoint(subId, pt);
    vertex->Points->SetPoint(0, pt);
    if (VertexIntersectWithLine(vertex, p1, p2, tTemp, xTemp))
      {
      intersection = 1;
      t = tTemp;
      for (int i = 0; i < 3; i++)
        {
        x[i] = xTemp[i];
        }
      }
    }
  vertex->Delete();
  return intersection;
}

int
vtkCellIntersections::LineIntersectWithLine(vtkLine *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  double a1[3], a2[3], u, v;

  cell->Points->GetPoint(0, a1);
  cell->Points->GetPoint(1, a2);

  t = VTK_DOUBLE_MAX;
  if (vtkLine::Intersection(p1, p2, a1, a2, u, v) == 2)
    {
    for (int i=0; i<3; i++)
      {
      x[i] = a1[i] + v*(a2[i]-a1[i]);
      }
    t = vtkMath::Distance2BetweenPoints(p1, x);
    return 1;
    }
  return 0;
}

int
vtkCellIntersections::PolyLineIntersectWithLine(vtkPolyLine *cell, double p1[3],
    double p2[3], double& t, double x[3])
{
  int i, numLines=cell->Points->GetNumberOfPoints() - 1;
  double a1[3], a2[3];
  int intersection = 0;
  double xTemp[3], tTemp, u = VTK_DOUBLE_MAX, v = VTK_DOUBLE_MAX;
  t = VTK_DOUBLE_MAX;
  for (int subId=0; subId < numLines; subId++)
    {
    cell->Points->GetPoint(subId, a1);
    cell->Points->GetPoint(subId+1, a2);
    if (vtkLine::Intersection(p1, p2, a1, a2, u, v) == 2)
      {
      for (i=0; i<3; i++)
        {
        xTemp[i] = a1[i] + v*(a2[i]-a1[i]);
        }

      tTemp = vtkMath::Distance2BetweenPoints(p1, xTemp);

      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        for (i=0; i<3; i++)
          {
          x[i] = xTemp[i];
          }
        }
      }
    }
  return intersection;
}

#define SUB(dest,v1,v2){dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2];}

// ****************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Mon Jul  7 15:57:37 PDT 2003 
//   Use smaller eps for testing when DotProduct close to zero.
//
//   Kathleen Bonnell, Thu Sep 18 15:48:54 PDT 2003
//   Cast multiplication to double before setting intersection point. 
//
//   Kathleen Bonnell, Fri Oct 10 10:46:48 PDT 2003 
//   Remove eps for testing DotProduct.
//
//   Kathleen Bonnell, Tue Jul 27 11:06:24 PDT 2004 
//   If the line is coplanar with the triangle, and we should test for
//   intersection in this case, call EdgeLineIsect. 
//
// ****************************************************************************
int
vtkCellIntersections::TriangleIntersectWithLine(vtkTriangle *cell, 
    double p1[3], double p2[3], double& t, double x[3])
{
  double pt1[3], pt2[3], pt3[3], xTemp[3];
  cell->Points->GetPoint(0, pt1);
  cell->Points->GetPoint(1, pt2);
  cell->Points->GetPoint(2, pt3);

  double e1[3], e2[3], p[3], s[3], q[3];
  double rayDir[3];
  double u, v, tmp, dt;

  int i;
  SUB(rayDir, p2, p1);
  SUB(e1, pt2, pt1);
  SUB(e2, pt3, pt1);

  vtkMath::Cross(rayDir, e2, p);

  tmp = vtkMath::Dot(p, e1);
  if (tmp == 0.)
    {
    if (this->TestCoPlanar)
      {
      int success =  EdgeLineIsect(cell, p1, p2, xTemp);
      if (success)
        {
        for (i = 0; i < 3; i++)
          {
          x[i] = xTemp[i];
          }
        t = vtkMath::Distance2BetweenPoints(p1, x);
        }
      return success;
      }
    else
      return 0;
    }

  tmp = 1.0/tmp;
  SUB(s, p1, pt1);

  u = tmp * vtkMath::Dot(s, p);

  if (u < 0.0 || u > 1.0)
    {
    return 0;
    }
 
  vtkMath::Cross(s, e1, q);
  v = tmp * vtkMath::Dot(rayDir, q);

  if (v < 0. || v > 1.) 
    {
    return 0;
    }
  if ((u+v) > 1.)
    {
    return 0;
    }
  dt = tmp * vtkMath::Dot(e2, q);

  for (i = 0; i < 3; i++)
    {
    x[i] = p1[i] + (dt*rayDir[i]);
    }
  t = vtkMath::Distance2BetweenPoints(p1, x);

  return 1;
}

int
vtkCellIntersections::TriStripIntersectWithLine(vtkTriangleStrip *cell, 
    double p1[3], double p2[3], double& t, double x[3])
{
  int numTris = cell->Points->GetNumberOfPoints()-2;
  double pt1[3], pt2[3], pt3[3];
  double tTemp, xTemp[3];
  int i, intersection = 0;
  t = VTK_DOUBLE_MAX;
  for (int subId = 0; subId < numTris; subId++)
    {
    tTemp = VTK_DOUBLE_MAX;
    cell->Points->GetPoint(subId,   pt1);
    cell->Points->GetPoint(subId+1, pt2);
    cell->Points->GetPoint(subId+2, pt3);
    this->triangle->Points->SetPoint(0, pt1);
    this->triangle->Points->SetPoint(1, pt2);
    this->triangle->Points->SetPoint(2, pt3);
 
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        for (i = 0; i < 3 ; i++)
          {
          x[i] = xTemp[i]; 
          }
        t = tTemp;
        intersection = 1;
        }
      }
    }
 
  return intersection;
}

// ****************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 27 11:06:24 PDT 2004 
//   If the line is coplanar with the polygon, and we should test for
//   intersection in this case, call EdgeLineIsect. 
//
// ****************************************************************************

int
vtkCellIntersections::PolygonIntersectWithLine(vtkPolygon *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  //
  // Only change from Polygon::IntersectWithLine is dependence upon tol.
  //
  double pt1[3], n[3];
  double closestPoint[3], xTemp[3];
  double dist2;
  int npts = cell->GetNumberOfPoints();
  int i, success;
  double *weights = NULL;;
 
  // Define a plane to intersect with
  //
  cell->Points->GetPoint(1, pt1);
  cell->ComputeNormal(cell->Points,n);
  double tTemp = -1; 
  // Intersect plane of the polygon with line
  //
  if (!vtkPlane::IntersectWithLine(p1,p2,n,pt1,tTemp,xTemp))
    {
    if (tTemp != VTK_DOUBLE_MAX || !this->TestCoPlanar)
          return 0;

    success = EdgeLineIsect(cell, p1, p2, xTemp);
    if (success)
      {
      for (i = 0; i < 3; i++)
        {
        x[i] = xTemp[i];
        }
      t = vtkMath::Distance2BetweenPoints(p1, x); 
      }
    return success;
    }

  // Evaluate position
  //
  weights = new double[npts];
  double pc[3] = {0., 0., 0.};
  int subId;
  success = 0;
  if (cell->EvaluatePosition(xTemp, closestPoint, subId, pc, dist2, weights))
    {
    for (i = 0; i < 3; i++)
      {
      x[i] = xTemp[i];
      }
    t = vtkMath::Distance2BetweenPoints(p1, x);
    success = 1;
    }
  delete [] weights;
  return success;
}

// ****************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 27 11:06:24 PDT 2004 
//   If the line is coplanar with the pixel, and we should test for
//   intersection in this case, call EdgeLineIsect. 
//
// ****************************************************************************
int
vtkCellIntersections::PixelIntersectWithLine(vtkPixel *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{

  // rewriting pixel code only so that tol is not needed.
  double pt1[3], pt4[3], n[3], xTemp[3];
  double closestPoint[3];
  double dist2, weights[4];
  int i;
 
  // 
  // Get normal for triangle
  // 
  cell->Points->GetPoint(0, pt1);
  cell->Points->GetPoint(3, pt4);
 
  n[0] = n[1] = n[2] = 0.0;
  for (i=0; i<3; i++)
    {
    if ((pt4[i] - pt1[i]) <= 0.0)
      {
      n[i] = 1.0;
      break;
      }
    }

  //
  // Intersect plane of pixel with line.
  //
  double tTemp = -1;
  int success = 0;
  if (!vtkPlane::IntersectWithLine(p1,p2,n,pt1,tTemp,xTemp))
    {
    // tTemp == VTK_DOUBLE_MAX if plane and line are parallel
    if (tTemp != VTK_DOUBLE_MAX || !this->TestCoPlanar)
      return 0;

    success =  EdgeLineIsect(cell, p1, p2, xTemp);
    if (success)
      {
      for (i = 0; i < 3; i++)
        {
        x[i] = xTemp[i];
        }
      t = vtkMath::Distance2BetweenPoints(p1, x);
      }
    return success;
    }

  //
  // Does intersection point lie within pixel? 
  //
  double pc[3] = {0., 0., 0.};
  int subId;
  success = 0;
  if (cell->EvaluatePosition(xTemp, closestPoint, subId, pc, dist2, weights))
    {
    for (i = 0; i < 3; i++)
      {
      x[i] = xTemp[i];
      }
    t = vtkMath::Distance2BetweenPoints(p1, x);
    success =  1;
    }

  return success;
}

// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Tue Aug  8 13:48:48 PDT 2006
//   Test both triangles for both diagonal cases, and return the closest
//   intersection point.
// ****************************************************************************
int
vtkCellIntersections::QuadIntersectWithLine(vtkQuad *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  double pt0[3], pt1[3], pt2[3], pt3[3];
  cell->Points->GetPoint(0, pt0);
  cell->Points->GetPoint(1, pt1);
  cell->Points->GetPoint(2, pt2);
  cell->Points->GetPoint(3, pt3);
  double tTemp = VTK_DOUBLE_MAX;
  double xTemp[3];
  t = VTK_DOUBLE_MAX;
  // Figure out how to uniquely tessellate the quad. Watch out for 
  // equivalent triangulations (i.e., the triangulation is equivalent
  // no matter where the diagonal). In this case use the point ids as 
  // a tie breaker to insure unique triangulation across the quad.
  //
  double dist = VTK_DOUBLE_MAX;
  int i, intersection = 0;

  for (int diagCase = 0; diagCase < 4; diagCase++)
    {
    switch (diagCase)
      {
      case 0 :
        // First Triangle, diagonal case 0
        this->triangle->Points->SetPoint(0, pt0);
        this->triangle->Points->SetPoint(1, pt1);
        this->triangle->Points->SetPoint(2, pt2);
        break; 
      case 1 :
        // Second Triangle, diagonal case 0
        this->triangle->Points->SetPoint(0, pt2);
        this->triangle->Points->SetPoint(1, pt3);
        this->triangle->Points->SetPoint(2, pt0);
        break; 
      case 2 :
        // First Triangle, diagonal case 1
        this->triangle->Points->SetPoint(0, pt0);
        this->triangle->Points->SetPoint(1, pt1);
        this->triangle->Points->SetPoint(2, pt3);
        break; 
      case 3 :
        // Second Triangle, diagonal case 1
        this->triangle->Points->SetPoint(0, pt2);
        this->triangle->Points->SetPoint(1, pt3);
        this->triangle->Points->SetPoint(2, pt1);
        break; 
      }
    tTemp = VTK_DOUBLE_MAX;

    if (TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        t = tTemp;
        intersection = 1;
        for (i = 0; i < 3; i++)
          {
          x[i] = xTemp[i];
          }
        }
      }
    } // for diagCase
  return intersection;
}

int
vtkCellIntersections::TetraIntersectWithLine(vtkTetra *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  int i, intersection = 0;
  double tTemp, xTemp[3];
  int faceNum, *faceIds;

  double pt0[3], pt1[3], pt2[3], pt3[3];
  t = VTK_DOUBLE_MAX;

  for (faceNum = 0; faceNum < 4; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], pt0);
    cell->Points->GetPoint(faceIds[1], pt1);
    cell->Points->GetPoint(faceIds[2], pt2);
    this->triangle->Points->SetPoint(0, pt0);
    this->triangle->Points->SetPoint(1, pt1);
    this->triangle->Points->SetPoint(2, pt2);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        for (i = 0; i < 3; i++)
          {
          x[i] = xTemp[i];
          }
        }
      }
    }
  return intersection;
}

int
vtkCellIntersections::VoxelIntersectWithLine(vtkVoxel *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  double minPt[3], maxPt[3], xTemp[3];;
  double bounds[6], p21[3];
  int i;
 
  t = VTK_DOUBLE_MAX;
 
  cell->Points->GetPoint(0, minPt);
  cell->Points->GetPoint(7, maxPt);
 
  for (i=0; i<3; i++)
    {
    p21[i] = p2[i] - p1[i];
    bounds[2*i] = minPt[i];
    bounds[2*i+1] = maxPt[i];
    }
 
  if (!IntersectBox(bounds, p1, p21, xTemp))
    {
    return 0;
    }

  for (i=0; i<3; i++)
    {
    x[i] = xTemp[i];
    }
  t = vtkMath::Distance2BetweenPoints(p1, x); 
  return 1;
}

int
vtkCellIntersections::HexIntersectWithLine(vtkHexahedron *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  int  i, intersection = 0, faceNum, *faceIds;
  double tTemp, xTemp[3];
  double pt0[3], pt1[3], pt2[3], pt3[3];
  t = VTK_DOUBLE_MAX;

  for (faceNum = 0; faceNum < 6; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], pt0);
    cell->Points->GetPoint(faceIds[1], pt1);
    cell->Points->GetPoint(faceIds[2], pt2);
    cell->Points->GetPoint(faceIds[3], pt3);
    this->quad->Points->SetPoint(0, pt0);
    this->quad->Points->SetPoint(1, pt1);
    this->quad->Points->SetPoint(2, pt2);
    this->quad->Points->SetPoint(3, pt3);
    if (this->QuadIntersectWithLine(quad, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        for (i = 0; i < 3; i++)
          {
          x[i] = xTemp[i];
          }
        }
      }
    }
  return intersection;  
}

int
vtkCellIntersections::WedgeIntersectWithLine(vtkWedge *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  int i, intersection = 0;
  double tTemp, xTemp[3];
  double pt0[3], pt1[3], pt2[3], pt3[3];
  int faceNum, *faceIds;

  t = VTK_DOUBLE_MAX;

  // faces 0 & 1 are triangle-faces, intersect them first
  for (faceNum = 0; faceNum < 2; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], pt0);
    cell->Points->GetPoint(faceIds[1], pt1);
    cell->Points->GetPoint(faceIds[2], pt2);
    this->triangle->Points->SetPoint(0, pt0);
    this->triangle->Points->SetPoint(1, pt1);
    this->triangle->Points->SetPoint(2, pt2);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        for (i = 0; i < 3; i ++) 
          {
          x[i] = xTemp[i];
          }
        }
      }
    }

  // faces 2 -4 are quads, now intersect them
  for (faceNum = 2; faceNum < 5; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], pt0);
    cell->Points->GetPoint(faceIds[1], pt1);
    cell->Points->GetPoint(faceIds[2], pt2);
    cell->Points->GetPoint(faceIds[3], pt3);
    this->quad->Points->SetPoint(0, pt0);
    this->quad->Points->SetPoint(1, pt1);
    this->quad->Points->SetPoint(2, pt2);
    this->quad->Points->SetPoint(3, pt3);
    if (this->QuadIntersectWithLine(this->quad, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        for (i = 0; i < 3; i ++) 
          {
          x[i] = xTemp[i];
          }
        }
      }
    }
  return intersection;
}

int
vtkCellIntersections::PyramidIntersectWithLine(vtkPyramid *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  int i, intersection = 0;
  int faceNum;
  int *faceIds;
  double xTemp[3], weights[5], dist2, tTemp;
  double pt0[3], pt1[3], pt2[3], pt3[3];

  t = VTK_DOUBLE_MAX;

  // faces 1-4 are triangles, intersect them first.
  for (faceNum = 1; faceNum < 5; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], pt0);
    cell->Points->GetPoint(faceIds[1], pt1);
    cell->Points->GetPoint(faceIds[2], pt2);
    this->triangle->Points->SetPoint(0, pt0);
    this->triangle->Points->SetPoint(1, pt1);
    this->triangle->Points->SetPoint(2, pt2);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        for (i = 0; i < 3; i++)
          {
          x[i] = xTemp[i];
          }
        }
      }
    }
  // now intersect quad face 
  faceIds = cell->GetFaceArray(0);
  cell->Points->GetPoint(faceIds[0], pt0);
  cell->Points->GetPoint(faceIds[1], pt1);
  cell->Points->GetPoint(faceIds[2], pt2);
  cell->Points->GetPoint(faceIds[3], pt3);
  this->quad->Points->SetPoint(0,  pt0);
  this->quad->Points->SetPoint(1,  pt1);
  this->quad->Points->SetPoint(2,  pt2);
  this->quad->Points->SetPoint(3,  pt3);
  tTemp = VTK_DOUBLE_MAX;

  if (this->QuadIntersectWithLine(this->quad, p1, p2, tTemp, xTemp))
    {
    if (tTemp < t)
      {
      intersection = 1;
      t = tTemp;
      for (i = 0; i < 3; i++)
        {
        x[i] = xTemp[i];
        }
      }
    }

  return intersection; 
}


// ****************************************************************************
// Method:    LineLineIsect
//
// Description:
//   Determines if two line segments intersect.
//
//   From Graphics Gems II, "Intersection of Line Segments" by Mukesh Prasad, 
//   p. 7-9, code: p. 473-476, xlines.c.
//
// Returns:     1 if an intersection is found, 0 otherwise.
//
// Arguments:
//   p1         The first endpoint of the first line segment. 
//   p2         The second endpoint of the first line segment.
//   p3         The first endpoint of the second line segment. 
//   p4         The second endpoint of the second line segment.
//   isect      A place to store the intersection point, if any. 
//
// Programmer:  Kathleen Bonnell
// Creation:    July 27, 2004 
//
// ****************************************************************************

int
LineLineIsect(const double *p1, const double *p2, const double *p3, 
              const double *p4, double *isect)
{
    double a1, a2, b1, b2, c1, c2, r1, r2, r3, r4;
    double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0]; 
    double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1]; 

    a1 = y2 - y1; 
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    r3 = a1 * x3 + b1 *y3 + c1;
    r4 = a1 * x4 + b1 *y4 + c1;
           
    if ((r3 < 0 && r4 < 0) || (r3 > 0 && r4 > 0))
    {
        return 0;
    }

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    r1 = a2 * x1 + b2 * y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;
            
    if ((r1 < 0 && r2 < 0) || (r1 > 0 && r2 > 0))
    {
        return 0;
    }

    double denom = a1 * b2 - a2 * b1;
    if (denom == 0)
    { // COLLINEAR
        return 0;
    }

    isect[0] = (b1 * c2 - b2 * c1) / denom;
    isect[1] = (a2 * c1 - a1 * c2) / denom;
    isect[2] = 0;

    return 1;    
}

// ****************************************************************************
// Method:    EdgeLineIsect
//
// Description:
//   Determines if the finit line specified by endpoints p1 and p2 intersects
//   any of the edges of the given cell.
//
// Returns:     1 if an intersection is found, 0 otherwise.A
//
// Arguments:
//   cell       The cell to test for intersection.
//   p1         The first endpoint of the finite line.
//   p2         The second endpoint of the finite line.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 27, 2004 
//
// ****************************************************************************
int
EdgeLineIsect(vtkCell *cell, const double *p1, const double *p2, double *x)
{
    double p3[3], p4[3];
    bool isectedEdge = false;
    int numEdges = cell->GetNumberOfEdges();
    int i;
    double xTemp[3];
    double dist = VTK_DOUBLE_MAX;
    for (i = 0; i < numEdges ; i++)
    {
        cell->GetEdge(i)->Points->GetPoint(0, p3);
        cell->GetEdge(i)->Points->GetPoint(1, p4);
        if (LineLineIsect(p1, p2, p3, p4, xTemp))
        {
            double d2 = vtkMath::Distance2BetweenPoints(p1, xTemp);
            if (d2 < dist)
            {
                dist = d2;
                x[0] = xTemp[0];
                x[1] = xTemp[1];
                x[2] = xTemp[2];
                isectedEdge = true;
            }
        }
    }
    return (isectedEdge ? 1 : 0); 
}


bool 
SlabTest(const double d, const double o, const double lo, 
         const double hi, double &tnear, double &tfar)
{
  if (d == 0) 
    { 
    if (o < lo || o > hi) 
      return false; 
    } 
  else 
    { 
    double T1 = (lo - o) / d; 
    double T2 = (hi - o) / d; 
    if (T1 > T2) 
      { 
      double temp = T1; 
      T1 = T2; 
      T2 = temp; 
      } 
    if (T1 > tnear)  
      { 
      tnear = T1; 
      } 
    if (T2 < tfar)  
      { 
      tfar = T2; 
      } 
    if (tnear > tfar) 
      return false; 
    if (tfar < 0) 
      return false; 
    if (tnear == tfar) 
      return false; 
    } 
  return true; 
} 

int
vtkCellIntersections::LineIntersectBox(const double bounds[6], 
    const double pt1[3], const double pt2[3], double coord[3])
{
    double si, ei, bmin, bmax, t;
    double st, et, fst = 0, fet = 1;

    for (int i = 0; i < 3; i++)
    {
        si = pt1[i];
        ei = pt2[i];
        bmin = bounds[2*i];
        bmax = bounds[2*i+1];
        if (si < ei)
        {
            if (si > bmax || ei < bmin)
            {
                return false;
            }
            double di = ei - si;
            st = (si < bmin) ? (bmin -si) / di : 0;
            et = (ei > bmax) ? (bmax -si) / di : 1;
        }
        else
        {
            if (ei > bmax || si < bmin)
            {
                return false;
            }
            double di = ei - si;
            
            st = (si > bmax) ? (bmax -si) / di : 0;
            et = (ei < bmin) ? (bmin -si) / di : 1;
        }
        if (st > fst) fst = st;
        if (et < fet) fet = et;
        if (fet < fst)
        {
            return false;
        }
    }
    t = fst;
    coord[0] = pt1[0] + t * (pt2[0]-pt1[0]);
    coord[1] = pt1[1] + t * (pt2[1]-pt1[1]);
    coord[2] = pt1[2] + t * (pt2[2]-pt1[2]);
    return true;
}

int
vtkCellIntersections::IntersectBox(const double bounds[6], 
    const double origin[3], const double dir[3], double coord[3]) 
{
  double Tnear = -DBL_MAX;
  double Tfar = DBL_MAX;
  if (!SlabTest(dir[0], origin[0], bounds[0], bounds[1], Tnear, Tfar))
  {
    return false;
  }
  if (!SlabTest(dir[1], origin[1], bounds[2], bounds[3], Tnear, Tfar))
  {
    return false;
  }
  if (!SlabTest(dir[2], origin[2], bounds[4], bounds[5], Tnear, Tfar))
  {
    return false;
  }

  coord[0] = origin[0] + Tnear *dir[0];
  coord[1] = origin[1] + Tnear *dir[1];
  coord[2] = origin[2] + Tnear *dir[2];

  return true;
}

