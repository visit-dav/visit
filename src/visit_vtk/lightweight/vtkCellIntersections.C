// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <vtkQuadraticHexahedron.h>
#include <vtkTetra.h>
#include <vtkTriangle.h>
#include <vtkTriangleStrip.h>
#include <vtkVertex.h>
#include <vtkVoxel.h>
#include <vtkWedge.h>

#include <stdlib.h>
#include <math.h>

vtkStandardNewMacro(vtkCellIntersections)



int
EdgeLineIsect(vtkCell *cell, const double *, const double *, double *);

static bool
PointOnLineSegment(const double *, const double *, const double *, double *);

static void
CheckClosestIntersection(const double *, const double *, double &, double *, bool &);

bool
SlabTest(const double d, const double o, const double lo,
         const double hi, double &tnear, double &tfar);

static bool
SamePoint(const double *p0, const double *p1)
{
  return p0[0] == p1[0] && p0[1] == p1[1] && p0[2] == p1[2];
}

static void
CopyPoint(double *dest, const double *src)
{
  dest[0] = src[0];
  dest[1] = src[1];
  dest[2] = src[2];
}

static int
GetUniqueQuadPoints(const double *pt0, const double *pt1,
    const double *pt2, const double *pt3, double uniquePts[4][3])
{
  const double *pts[4] = {pt0, pt1, pt2, pt3};
  int numUnique = 0;

  for (int i = 0; i < 4; i++)
    {
    bool found = false;
    for (int j = 0; j < numUnique && !found; j++)
      {
      found = SamePoint(pts[i], uniquePts[j]);
      }
    if (!found)
      {
      CopyPoint(uniquePts[numUnique], pts[i]);
      numUnique++;
      }
    }

  return numUnique;
}

static bool
PointsAreCollinear(double pts[4][3], int numPts)
{
  if (numPts < 3)
    {
    return true;
    }

  double v0[3];
  for (int i = 0; i < 3; i++)
    {
    v0[i] = pts[1][i] - pts[0][i];
    }
  double len0 = vtkMath::Dot(v0, v0);

  for (int pt = 2; pt < numPts; pt++)
    {
    double v1[3], cross[3];
    for (int i = 0; i < 3; i++)
      {
      v1[i] = pts[pt][i] - pts[0][i];
      }

    vtkMath::Cross(v0, v1, cross);

    double cross2 = vtkMath::Dot(cross, cross);
    double len1 = vtkMath::Dot(v1, v1);

    if (cross2 > DBL_EPSILON * DBL_EPSILON * len0 * len1)
      {
      return false;
      }
    }

  return true;
}

static void
GetLongestSegment(double pts[4][3], int numPts, double p0[3], double p1[3])
{
  double maxDist = -1.;

  for (int i = 0; i < numPts; i++)
    {
    for (int j = i + 1; j < numPts; j++)
      {
      double dist = vtkMath::Distance2BetweenPoints(pts[i], pts[j]);
      if (dist > maxDist)
        {
        maxDist = dist;
        CopyPoint(p0, pts[i]);
        CopyPoint(p1, pts[j]);
        }
      }
    }
}

static int
PointIntersectWithLine(const double *pt, const double *p1, const double *p2,
    double &t, double *x)
{
  double lineT = 0.;
  double cp[3];
  double dist = vtkLine::DistanceToLine(pt, p1, p2, lineT, cp);
  t = VTK_DOUBLE_MAX;

  if (dist == 0.)
    {
    CopyPoint(x, pt);
    t = vtkMath::Distance2BetweenPoints(p1, x);
    return 1;
    }

  return 0;
}

static int
SegmentIntersectWithLine(const double *pt0, const double *pt1,
    const double *p1, const double *p2, double &t, double *x)
{
  double u = VTK_DOUBLE_MAX;
  double v = VTK_DOUBLE_MAX;
  t = VTK_DOUBLE_MAX;

  if (vtkLine::Intersection(p1, p2, pt0, pt1, u, v) == vtkLine::Intersect)
    {
    for (int i = 0; i < 3; i++)
      {
      x[i] = pt0[i] + v*(pt1[i]-pt0[i]);
      }
    t = vtkMath::Distance2BetweenPoints(p1, x);
    return 1;
    }

  return 0;
}

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


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sat Oct  6 09:45:02 PDT 2007
//    Add case for hex20.  Also change debug macro to error macro, since debug
//    macros don't get put into the VisIt debug logs and error macros do ...and
//    I believe the debug statement *should* be in the VisIt debug logs.
//
// ****************************************************************************

int
vtkCellIntersections::CellIntersectWithLine(vtkCell *cell,
    double p1[3], double p2[3], double& t, double x[3])
{
  switch(cell->GetCellType())
    {
    case VTK_EMPTY_CELL : return 0;
    case VTK_VERTEX :
      return VertexIntersectWithLine(
             static_cast<vtkVertex*>(cell), p1, p2, t, x);
    case VTK_POLY_VERTEX :
      return PolyVertexIntersectWithLine(
             static_cast<vtkPolyVertex*>(cell), p1, p2, t, x);
    case VTK_LINE :
      return LineIntersectWithLine(
             static_cast<vtkLine*>(cell), p1, p2, t, x);
    case VTK_POLY_LINE :
      return PolyLineIntersectWithLine(
             static_cast<vtkPolyLine*>(cell), p1, p2, t, x);
    case VTK_TRIANGLE :
      return TriangleIntersectWithLine(
             static_cast<vtkTriangle*>(cell), p1, p2, t, x);
    case VTK_TRIANGLE_STRIP :
      return TriStripIntersectWithLine(
             static_cast<vtkTriangleStrip*>(cell), p1, p2, t, x);
    case VTK_POLYGON :
      return PolygonIntersectWithLine(
             static_cast<vtkPolygon*>(cell), p1, p2, t, x);
    case VTK_PIXEL :
      return PixelIntersectWithLine(
             static_cast<vtkPixel*>(cell), p1, p2, t, x);
    case VTK_QUAD :
      return QuadIntersectWithLine(
             static_cast<vtkQuad*>(cell), p1, p2, t, x);
    case VTK_TETRA :
      return TetraIntersectWithLine(
             static_cast<vtkTetra*>(cell), p1, p2, t, x);
    case VTK_VOXEL :
      return VoxelIntersectWithLine(
             static_cast<vtkVoxel*>(cell), p1, p2, t, x);
    case VTK_HEXAHEDRON :
      return HexIntersectWithLine(
             static_cast<vtkHexahedron*>(cell), p1, p2, t, x);
    case VTK_WEDGE :
      return WedgeIntersectWithLine(
             static_cast<vtkWedge*>(cell), p1, p2, t, x);
    case VTK_PYRAMID :
      return PyramidIntersectWithLine(
             static_cast<vtkPyramid*>(cell), p1, p2, t, x);
    case VTK_QUADRATIC_HEXAHEDRON :
      return QuadraticHexahedronIntersectWithLine(
             static_cast<vtkQuadraticHexahedron*>(cell), p1, p2, t, x);
    default:
      vtkErrorMacro( << "CellType  " << cell->GetCellType()
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
  vtkIdType numPts=cell->Points->GetNumberOfPoints();

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
  int i;
  vtkIdType numLines=cell->Points->GetNumberOfPoints() - 1;
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
//   Hank Childs, Fri Mar  2 15:18:17 PST 2012
//   Add special handling for cases close to 0.  (Needed for 2D.)
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
  if (fabs(dt-0.5) < 1e-7)
    {
    double mag = sqrt(rayDir[0]*rayDir[0]+rayDir[1]*rayDir[1]+rayDir[2]*rayDir[2]);
    if (mag > 1e-3)
      {
      for (i = 0 ; i < 3 ; i++)
        if (fabs(x[i]) < 1e-10)
          x[i] = 0.;
      }
    }
  t = vtkMath::Distance2BetweenPoints(p1, x);

  return 1;
}

int
vtkCellIntersections::TriStripIntersectWithLine(vtkTriangleStrip *cell,
    double p1[3], double p2[3], double& t, double x[3])
{
  vtkIdType numTris = cell->Points->GetNumberOfPoints()-2;
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
  vtkIdType npts = cell->GetNumberOfPoints();
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
//
//   Kathleen Biagas, Wed Sep 2, 2026
//   Using uniquepoints, determine if  Quad is degenerate, and call new
//   Intersect method specifically for such quads.
//
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
  double uniquePts[4][3];
  int numUnique = GetUniqueQuadPoints(pt0, pt1, pt2, pt3, uniquePts);
  if (numUnique < 4 || PointsAreCollinear(uniquePts, numUnique))
    {
    return this->DegenerateQuadIntersectWithLine(uniquePts, numUnique,
                                                 p1, p2, t, x);
    }

  double tTemp = VTK_DOUBLE_MAX;
  double xTemp[3];
  t = VTK_DOUBLE_MAX;
  // Figure out how to uniquely tessellate the quad. Watch out for
  // equivalent triangulations (i.e., the triangulation is equivalent
  // no matter where the diagonal). In this case use the point ids as
  // a tie breaker to insure unique triangulation across the quad.
  //
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
vtkCellIntersections::DegenerateQuadIntersectWithLine(double pts[4][3],
    int numPts, double p1[3], double p2[3], double& t, double x[3])
{
  t = VTK_DOUBLE_MAX;

  if (numPts == 1)
    {
    return PointIntersectWithLine(pts[0], p1, p2, t, x);
    }

  if (numPts == 2 || PointsAreCollinear(pts, numPts))
    {
    double pt0[3], pt1[3];
    GetLongestSegment(pts, numPts, pt0, pt1);
    return SegmentIntersectWithLine(pt0, pt1, p1, p2, t, x);
    }

  if (numPts == 3)
    {
    this->triangle->Points->SetPoint(0, pts[0]);
    this->triangle->Points->SetPoint(1, pts[1]);
    this->triangle->Points->SetPoint(2, pts[2]);
    return this->TriangleIntersectWithLine(this->triangle, p1, p2, t, x);
    }

  return 0;
}

int
vtkCellIntersections::TetraIntersectWithLine(vtkTetra *cell, double p1[3],
    double p2[3], double& t, double x[3])
{
  int i, intersection = 0;
  double tTemp, xTemp[3];

  double pt0[3], pt1[3], pt2[3];
  t = VTK_DOUBLE_MAX;

  for (vtkIdType faceNum = 0; faceNum < 4; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    const vtkIdType *faceIds = cell->GetFaceArray(faceNum);
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
  int  i, intersection = 0;
  double tTemp, xTemp[3];
  double pt0[3], pt1[3], pt2[3], pt3[3];
  t = VTK_DOUBLE_MAX;

  for (vtkIdType faceNum = 0; faceNum < 6; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    const vtkIdType *faceIds = cell->GetFaceArray(faceNum);
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
vtkCellIntersections::QuadraticHexahedronIntersectWithLine(
                             vtkQuadraticHexahedron *cell, double p1[3],
                             double p2[3], double& t, double x[3])
{
  int  i, intersection = 0, faceNum;
  double tTemp, xTemp[3];
  double pt0[3], pt1[3], pt2[3], pt3[3];
  t = VTK_DOUBLE_MAX;

  int allFaces[6][4] = { { 0, 1, 5, 4 }, { 1, 2, 6, 5 },
                    { 4, 5, 6, 7 }, { 3, 0, 4, 7 },
                    { 0, 1, 2, 3 }, { 2, 3, 7, 6 } };
  for (faceNum = 0; faceNum < 6; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    int *faceIds = allFaces[faceNum];
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

  t = VTK_DOUBLE_MAX;

  // faces 0 & 1 are triangle-faces, intersect them first
  for (vtkIdType faceNum = 0; faceNum < 2; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    const vtkIdType *faceIds = cell->GetFaceArray(faceNum);
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
  for (vtkIdType faceNum = 2; faceNum < 5; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    const vtkIdType *faceIds = cell->GetFaceArray(faceNum);
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
  double xTemp[3], tTemp;
  double pt0[3], pt1[3], pt2[3], pt3[3];

  t = VTK_DOUBLE_MAX;

  // faces 1-4 are triangles, intersect them first.
  for (vtkIdType faceNum = 1; faceNum < 5; faceNum++)
    {
    tTemp = VTK_DOUBLE_MAX;
    const vtkIdType *faceIds = cell->GetFaceArray(faceNum);
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
  const vtkIdType *faceIds = cell->GetFaceArray(0);
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
// Method:    PointOnLineSegment
//
// Description:
//   Determines if the given point lies on the finite line segment.
//
// Returns:     true if the point lies on the segment, false otherwise.
//
// Programmer:  Kathleen Biagas
// Creation:    August 27, 2026
//
// ****************************************************************************
static bool
PointOnLineSegment(const double *pt, const double *p1, const double *p2,
                   double *closestPoint)
{
    double t;
    double dist2 = vtkLine::DistanceToLine(pt, p1, p2, t, closestPoint);
    if (dist2 != 0.0)
    {
        return false;
    }

    return (t >= 0.0 && t <= 1.0);
}

// ****************************************************************************
// Method:    CheckClosestIntersection
//
// Description:
//   Updates the intersection point if the candidate is closer to p1.
//
// Programmer:  Kathleen Biagas
// Creation:    August 27, 2026
//
// ****************************************************************************

static void
CheckClosestIntersection(const double *candidate, const double *p1,
                         double &dist, double *x, bool &isectedEdge)
{
    double d2 = vtkMath::Distance2BetweenPoints(p1, candidate);
    if (d2 < dist)
    {
        dist = d2;
        x[0] = candidate[0];
        x[1] = candidate[1];
        x[2] = candidate[2];
        isectedEdge = true;
    }
}


// ****************************************************************************
// Method:    EdgeLineIsect
//
// Description:
//   Determines if the finite line specified by endpoints p1 and p2 intersects
//   any of the edges of the given cell.
//   Tests in 3D so degenerate quad faces on hexes are pickable regardless of
//   view orientation.
//
// Returns:     1 if an intersection is found, 0 otherwise.
//
// Arguments:
//   cell       The cell to test for intersection.
//   p1         The first endpoint of the finite line.
//   p2         The second endpoint of the finite line.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 27, 2004
//
// Modifications:
//   Kathleen Biagas, Wed Sep 2, 2026
//   Replace 2D XY line intersection with 3D finite segment tests and handle
//   collapsed edges.
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

        if (vtkMath::Distance2BetweenPoints(p3, p4) == 0.0)
        {
            if (PointOnLineSegment(p3, p1, p2, xTemp))
            {
                CheckClosestIntersection(xTemp, p1, dist, x, isectedEdge);
            }
            continue;
        }

        double u, v;
        int lineIntersection = vtkLine::Intersection(p1, p2, p3, p4, u, v);
        if (lineIntersection == vtkLine::Intersect)
        {
            for (int j = 0; j < 3; j++)
            {
                xTemp[j] = p3[j] + v*(p4[j]-p3[j]);
            }
            CheckClosestIntersection(xTemp, p1, dist, x, isectedEdge);
        }
        else if (lineIntersection == vtkLine::OnLine)
        {
            double candidate[3];
            if (PointOnLineSegment(p1, p3, p4, candidate))
            {
                CheckClosestIntersection(candidate, p1, dist, x, isectedEdge);
            }
            if (PointOnLineSegment(p2, p3, p4, candidate))
            {
                CheckClosestIntersection(candidate, p1, dist, x, isectedEdge);
            }
            if (PointOnLineSegment(p3, p1, p2, candidate))
            {
                CheckClosestIntersection(candidate, p1, dist, x, isectedEdge);
            }
            if (PointOnLineSegment(p4, p1, p2, candidate))
            {
                CheckClosestIntersection(candidate, p1, dist, x, isectedEdge);
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
