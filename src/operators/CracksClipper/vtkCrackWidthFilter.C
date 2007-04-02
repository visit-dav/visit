#include "vtkCrackWidthFilter.h"
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>

#include <vtkBox.h>
#include <vtkHexahedron.h>
#include <vtkLine.h>
#include <vtkMath.h>
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

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <vtkVisItUtility.h>

#include <math.h>


#ifdef SUNOS
#include <ieeefp.h> // for 'finite'
#endif


vtkCxxRevisionMacro(vtkCrackWidthFilter, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkCrackWidthFilter);

vtkCrackWidthFilter::vtkCrackWidthFilter()
{
  this->triangle = vtkTriangle::New();
  this->quad = vtkQuad::New();
  this->MaxCrack1Width = 0.f;
  this->MaxCrack2Width = 0.f;
  this->MaxCrack3Width = 0.f;
  this->Crack1Var = NULL;
  this->Crack2Var = NULL;
  this->Crack3Var = NULL;
  this->StrainVar = NULL;
}

vtkCrackWidthFilter::~vtkCrackWidthFilter()
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
   this->SetCrack1Var(NULL);
   this->SetCrack2Var(NULL);
   this->SetCrack3Var(NULL);
   this->SetStrainVar(NULL);
}

void
vtkCrackWidthFilter::Execute()
{
  if (this->StrainVar == NULL)
    EXCEPTION0(ImproperUseException); 

  if (this->Crack1Var == NULL &&
      this->Crack2Var == NULL &&
      this->Crack3Var == NULL)
    EXCEPTION0(ImproperUseException); 

  
  vtkDataSet *input = GetInput();
  vtkCellData *inCD = input->GetCellData();

  vtkFloatArray *cd1 = NULL;
  if (this->Crack1Var != NULL)
    cd1 = (vtkFloatArray*)inCD->GetArray(this->Crack1Var);
  vtkFloatArray *cd2 = NULL;
  if (this->Crack2Var != NULL)
    cd2 = (vtkFloatArray*)inCD->GetArray(this->Crack2Var);
  vtkFloatArray *cd3 = NULL;
  if (this->Crack3Var != NULL)
    cd3 = (vtkFloatArray*)inCD->GetArray(this->Crack3Var);
  vtkFloatArray *strain = (vtkFloatArray*)inCD->GetArray(this->StrainVar);

  if (strain == NULL)
    EXCEPTION0(ImproperUseException); 
  if (cd1 == NULL && cd2 == NULL && cd3 == NULL)
    EXCEPTION0(ImproperUseException); 

  vtkDataSet *output = GetOutput();
  output->DeepCopy(input);

  int numCells = input->GetNumberOfCells();

  // Prepare the arrays
  vtkFloatArray *crack1Width = NULL; 
  if (cd1)
    {
    crack1Width = vtkFloatArray::New();
    crack1Width->SetName("avtCrack1Width");
    crack1Width->SetNumberOfComponents(1);
    crack1Width->SetNumberOfTuples(numCells);
    }

  vtkFloatArray *crack2Width = NULL;
  if (cd2)
    {
    crack2Width = vtkFloatArray::New();
    crack2Width->SetName("avtCrack2Width");
    crack2Width->SetNumberOfComponents(1);
    crack2Width->SetNumberOfTuples(numCells);
    }
 
  vtkFloatArray *crack3Width = NULL;
  if (cd3)
    {
    crack3Width = vtkFloatArray::New();
    crack3Width->SetName("avtCrack3Width");
    crack3Width->SetNumberOfComponents(1);
    crack3Width->SetNumberOfTuples(numCells);
    }

  vtkFloatArray *cellCenters = vtkFloatArray::New();
  cellCenters->SetName("avtCellCenters");
  cellCenters->SetNumberOfComponents(3);
  cellCenters->SetNumberOfTuples(numCells);


  //
  // formula for calculating crack width:
  //    crackwidth = delta * (L / (1.+delta))
  //    where: 
  //      L = length of zone along crack direction
  //        find L by intersecting line through cell center along
  //        crack direction, L = distance between 2 intersection points
  //
  //      delta = T11 for crack dir1 = component 0 of strain_tensor 
  //              T22 for crack dir2 = component 4 of strain_tensor
  //              T33 for crack dir3 = component 8 of strain_tensor
  //


  // 
  // step through cells, calculating cell centers and crack widths for 
  // each required crack direction.  Terminate early when possible.
  // 
  for (int cellId = 0; cellId < numCells; cellId++)
    {
    // won't calculate cell center unless it is really needed, so create a 
    // default (invalid) value to store in the array. Clipping of this cell 
    // will be contingent upon a non-zero crack width
    double center[3] = {VTK_LARGE_FLOAT, VTK_LARGE_FLOAT, VTK_LARGE_FLOAT};
    bool do_cd1 = (cd1 != NULL);
    bool do_cd2 = (cd2 != NULL);
    bool do_cd3 = (cd3 != NULL);
    double *dir1 = NULL;
    double *dir2 = NULL;
    double *dir3 = NULL;
    double delta1 = 0.f;
    double delta2 = 0.f;
    double delta3 = 0.f;
    if (do_cd1)
      {
      // t11 is component 0 in the strain array
      delta1 = strain->GetComponent(cellId, 0);
      if (delta1 == 0)
        {
        do_cd1 = false;
        crack1Width->SetValue(cellId, 0);
        }
      else  
        {
        dir1 = cd1->GetTuple(cellId);
        if (dir1[0] == 0 && dir1[1] == 0 && dir1[2] == 0)
          {
          do_cd1 = false;
          crack1Width->SetValue(cellId, 0);
          }
        }
      }
    if (do_cd2)
      {
      // t22 is component 4 in the strain array
      delta2 = strain->GetComponent(cellId, 4);
      if (delta2 == 0)
        {
        do_cd2 = false;
        crack2Width->SetValue(cellId, 0);
        }
      else  
        {
        dir2 = cd2->GetTuple(cellId);
        if (dir2[0] == 0 && dir2[1] == 0 && dir2[2] == 0)
          {
          do_cd2 = false;
          crack2Width->SetValue(cellId, 0);
          }
        }
      }
    if (do_cd3)
      {
      // t33 is component 8 in the strain array
      delta3 = strain->GetComponent(cellId, 8);
      if (delta3 == 0)
        {
        do_cd3 = false;
        crack3Width->SetValue(cellId, 0);
        }
      else  
        {
        dir3 = cd3->GetTuple(cellId);
        if (dir3[0] == 0 && dir3[1] == 0 && dir3[2] == 0)
          {
          do_cd3 = false;
          crack3Width->SetValue(cellId, 0);
          }
        }
      }

    if (!do_cd1 && !do_cd2 && !do_cd3)
      {
      // no further processing of this cell required, 
      // instead of calculating true cell center, avoid the
      // calculation and store the 'dummy' center.
      cellCenters->SetTuple(cellId, center);
      continue;
      }

    vtkCell *cell = input->GetCell(cellId);

    vtkVisItUtility::GetCellCenter(cell, center);
    cellCenters->SetTuple(cellId, center);

    double cl = cell->GetLength2();
    double crackWidth;

    if (do_cd1)
      {
      crackWidth = CrackWidthForCell(cell, center, cl, delta1, dir1);
      crack1Width->SetValue(cellId, crackWidth);

      if (crackWidth > this->MaxCrack1Width)
        this->MaxCrack1Width = crackWidth;
      } // crack1Width

    if (do_cd2)
      {
      crackWidth = CrackWidthForCell(cell, center, cl, delta2, dir2);
      crack2Width->SetValue(cellId, crackWidth);

      if (crackWidth > this->MaxCrack2Width)
        this->MaxCrack2Width = crackWidth;
      } // crack2Width

    if (do_cd3)
      {
      crackWidth = CrackWidthForCell(cell, center, cl, delta3, dir3);
      crack3Width->SetValue(cellId, crackWidth);

      if (crackWidth > this->MaxCrack3Width)
        this->MaxCrack3Width = crackWidth;
      } // crack3Width

    } // for all cells

  output->GetCellData()->AddArray(cellCenters);
  output->GetCellData()->CopyFieldOn("avtCellCenters");
  cellCenters->Delete();

  if (crack1Width)
    {
    output->GetCellData()->AddArray(crack1Width);
    output->GetCellData()->CopyFieldOn("avtCrack1Width");
    crack1Width->Delete();
    }
  if (crack2Width)
    {
    output->GetCellData()->AddArray(crack2Width);
    output->GetCellData()->CopyFieldOn("avtCrack2Width");
    crack2Width->Delete();
    }
  if (crack3Width)
    {
    output->GetCellData()->AddArray(crack3Width);
    output->GetCellData()->CopyFieldOn("avtCrack3Width");
    crack3Width->Delete();
    }
}


double
vtkCrackWidthFilter::CrackWidthForCell(vtkCell *cell, const double *center,
  const double cellLength, const double delta, const double *dir)
{
  double L, t, crackWidth, p1[3], p2[3], x1[3], x2[3];

  double cl = cellLength *10;
  for (int i = 0; i < 3; i++)
    {
    p1[i] = center[i] + cl*dir[i];
    p2[i] = center[i] - cl*dir[i];
    }

  int success = CellIntersectWithLine(cell, p1, p2, t, x1);

  if (success)
    {
    success = CellIntersectWithLine(cell, p2, p1, t, x2);
    }

  if (!success)
    {
    crackWidth =  0;
    }
  else
    {
    L = vtkMath::Distance2BetweenPoints(x1, x2);
    if (!finite(L) || L <= 0)
      {
      crackWidth =  0;
      }
    else
      {
      if (L > cellLength) L = cellLength;
      crackWidth = delta * (L / (1.+delta));
      crackWidth = crackWidth < 0 ? 0 : crackWidth;
      } 
    } 
  return crackWidth;
}



int
vtkCrackWidthFilter::CellIntersectWithLine(vtkCell *cell, double p1[3], 
    double p2[3], double &t, double x[3])
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
vtkCrackWidthFilter::VertexIntersectWithLine(vtkVertex *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  double cp[3];
  double X[3];
  cell->Points->GetPoint(0, X);
  double dist = vtkLine::DistanceToLine(X, p1, p2, t, cp);
  t = VTK_LARGE_FLOAT; 
  if (dist == 0)
    {
    x[0] = X[0];
    x[1] = X[1];
    x[2] = X[2];
    t = dist;
    return 1;
    }
  return 0;
}

int
vtkCrackWidthFilter::PolyVertexIntersectWithLine(vtkPolyVertex *cell, 
    double p1[3], double p2[3], double &t, double x[3])
{
  int numPts=cell->Points->GetNumberOfPoints();

  double tTemp, xTemp[3], a1[3]; 
  t = VTK_LARGE_FLOAT; 
  vtkVertex *vertex = vtkVertex::New();
  int intersection = 0;
  for (int subId=0; subId < numPts; subId++)
    {
    tTemp = VTK_LARGE_FLOAT; 
    cell->Points->GetPoint(subId, a1);
    vertex->Points->SetPoint(0, a1);

    if (VertexIntersectWithLine(vertex, p1, p2, tTemp, xTemp));
      {
      if (tTemp < t)
        {
        intersection = 1; 
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        }
      }
    }
  vertex->Delete();
  return intersection;
}

int
vtkCrackWidthFilter::LineIntersectWithLine(vtkLine *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  double a1[3], a2[3], tmp;
  int i;

  cell->Points->GetPoint(0, a1);
  cell->Points->GetPoint(1, a2);

  if ( vtkLine::Intersection(p1, p2, a1, a2, t, tmp) == 2 )
    {
    for (i=0; i<3; i++)
      {
      x[i] = a1[i] + tmp*(a2[i]-a1[i]);
      }
    return 1;
    }
  return 0;
}

int
vtkCrackWidthFilter::PolyLineIntersectWithLine(vtkPolyLine *cell, double p1[3],
    double p2[3], double &t, double x[3])
{
  int i, subId, numLines=cell->Points->GetNumberOfPoints() - 1;
  double a1[3], a2[3], tmp;
  double tTemp;
  int intersection = 0;
  for (subId=0; subId < numLines; subId++)
    {
    tmp = VTK_LARGE_FLOAT;
    cell->Points->GetPoint(subId, a1);
    cell->Points->GetPoint(subId+1, a1);
    if ( vtkLine::Intersection(p1, p2, a1, a2, tTemp, tmp) == 2)
      {
      if (tmp < t)
        {
        intersection = 1;
        t = tmp;
        for (i=0; i<3; i++)
          {
          x[i] = a1[i] + tmp*(a2[i]-a1[i]);
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
vtkCrackWidthFilter::TriangleIntersectWithLine(vtkTriangle *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  double pt1[3], pt2[3], pt3[3];
 
  cell->Points->GetPoint(0, pt1);
  cell->Points->GetPoint(1, pt2);
  cell->Points->GetPoint(2, pt3);

  double dp1[3] = {p1[0], p1[1], p1[2]};
  double dp2[3] = {p2[0], p2[1], p2[2]};

  double e1[3], e2[3], p[3], s[3], q[3];
  double rayDir[3];
  double u, v, tmp, dt;

  int i;

  // find first intersection
  SUB(rayDir, dp2, dp1);
  SUB(e1, pt2, pt1);
  SUB(e2, pt3, pt1);

  vtkMath::Cross(rayDir, e2, p);
  tmp = vtkMath::Dot(p, e1);

  if (tmp == 0.)
    {
    int success = EdgeLineIsect(cell, p1, p2, x);
    if (success)
        t = vtkMath::Distance2BetweenPoints(p1, x);
    return success;
    }

  tmp = 1.0/tmp;
  SUB(s, dp1, pt1);

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
    x[i] = (double) (dp1[i] + (double)(dt*rayDir[i]));
    }
  t = dt;
  return 1;
}

int
vtkCrackWidthFilter::TriStripIntersectWithLine(vtkTriangleStrip *cell, 
    double p1[3], double p2[3], double &t, double x[3])
{
  int subTest, numTris = cell->Points->GetNumberOfPoints()-2;
  int intersection = 0;
  double tTemp;
  double xTemp[3], a1[3], a2[3], a3[3];
  t = VTK_LARGE_FLOAT;
  for (int subId = 0; subId < numTris; subId++)
    {
    cell->Points->GetPoint(subId, a1);
    cell->Points->GetPoint(subId+1, a2);
    cell->Points->GetPoint(subId+2, a3);
    this->triangle->Points->SetPoint(0,a1);
    this->triangle->Points->SetPoint(1,a2);
    this->triangle->Points->SetPoint(2,a3);

    tTemp = VTK_LARGE_FLOAT; 
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
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
vtkCrackWidthFilter::PolygonIntersectWithLine(vtkPolygon *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  //
  // Only change from Polygon::IntersectWithLine is dependence upon tol.
  //
  double pt1[3], n[3];
  double closestPoint[3];
  double dist2;
  int npts = cell->GetNumberOfPoints();
  double *weights;
 
  int subId = 0;
  double pcoords[3] = {0., 0., 0.};
 
  // Define a plane to intersect with
  //
  cell->Points->GetPoint(1, pt1);
  cell->ComputeNormal(cell->Points,n);
  double tmpt = -1; 
  // Intersect plane of the polygon with line
  //
  if ( ! vtkPlane::IntersectWithLine(p1,p2,n,pt1,tmpt,x) )
    {
    if (tmpt != VTK_LARGE_FLOAT)
      return 0;
 
    int success =  EdgeLineIsect(cell, p1, p2, x);
    if (success)
        t = vtkMath::Distance2BetweenPoints(p1, x);
    return success;
    }

  // Evaluate position
  //
  weights = new double[npts];
  if ( cell->EvaluatePosition(x, closestPoint, subId, pcoords, dist2, weights))
    {
      delete [] weights;
      t = dist2;
      return 1;
    }
  delete [] weights;
  return 0;
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
vtkCrackWidthFilter::PixelIntersectWithLine(vtkPixel *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{

  // rewriting pixel code only so that tol is not needed.
  double pt1[3], pt4[3], n[3];
  double closestPoint[3];
  double dist2, weights[4];
  int i;
 
  int subId = 0;
  double pcoords[3] = {0., 0., 0.};

  // 
  // Get normal for triangle
  // 
  cell->Points->GetPoint(0, pt1);
  cell->Points->GetPoint(3, pt4);
 
  n[0] = n[1] = n[2] = 0.0;
  for (i=0; i<3; i++)
    {
    if ( (pt4[i] - pt1[i]) <= 0.0 )
      {
      n[i] = 1.0;
      break;
      }
    }

  //
  // Intersect plane of pixel with line.
  //
  double tmpt = -1;
  if ( ! vtkPlane::IntersectWithLine(p1,p2,n,pt1,tmpt,x) )
    {
    if (tmpt != VTK_LARGE_FLOAT)
      return 0;

    int success =  EdgeLineIsect(cell, p1, p2, x);
    if (success)
      t = vtkMath::Distance2BetweenPoints(p1, x);
    return success;
    }

  //
  // Does intersection point lie within pixel? 
  //
  if (cell->EvaluatePosition(x, closestPoint, subId, pcoords, dist2, weights) )
    {
    t = dist2;
    return 1;
    }
  return 0;

}

int
vtkCrackWidthFilter::QuadIntersectWithLine(vtkQuad *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  int diagonalCase;
  double a1[3], a2[3], a3[3], a4[3];
  cell->Points->GetPoint(0, a1);
  cell->Points->GetPoint(1, a2);
  cell->Points->GetPoint(2, a3);
  cell->Points->GetPoint(3, a4);
  double d1 = vtkMath::Distance2BetweenPoints(a1, a3);
  double d2 = vtkMath::Distance2BetweenPoints(a2, a4);

  // Figure out how to uniquely tessellate the quad. Watch out for 
  // equivalent triangulations (i.e., the triangulation is equivalent
  // no matter where the diagonal). In this case use the point ids as 
  // a tie breaker to insure unique triangulation across the quad.
  //
  if ( d1 == d2 ) //rare case; discriminate based on point id
    {
    int i, id, maxId=0, maxIdx=0;
    for (i=0; i<4; i++) //find the maximum id
      {
      if ( (id=cell->PointIds->GetId(i)) > maxId )
        {
        maxId = id;
        maxIdx = i;
        }
      }
    if ( maxIdx == 0 || maxIdx == 2) diagonalCase = 0;
    else diagonalCase = 1;
    }
  else if ( d1 < d2 )
    {
    diagonalCase = 0;
    }
  else //d2 < d1
    {
    diagonalCase = 1;
    }

  // Note: in the following code the parametric coords must be adjusted to
  // reflect the use of the triangle parametric coordinate system.
  switch (diagonalCase)
    {
    case 0:
      this->triangle->Points->SetPoint(0,a1);
      this->triangle->Points->SetPoint(1,a2);
      this->triangle->Points->SetPoint(2,a3);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x))
        {
        return 1;
        }
      this->triangle->Points->SetPoint(0,a3);
      this->triangle->Points->SetPoint(1,a4);
      this->triangle->Points->SetPoint(2,a1);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x))
        {
        return 1;
        }
      return 0;

    case 1:
      this->triangle->Points->SetPoint(0,a1);
      this->triangle->Points->SetPoint(1,a2);
      this->triangle->Points->SetPoint(2,a4);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x))
        {  
        return 1;
        }
      this->triangle->Points->SetPoint(0,a3);
      this->triangle->Points->SetPoint(1,a4);
      this->triangle->Points->SetPoint(2,a2);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x))
        {
        return 1;
        }
      return 0;
    }  
    return 0;
}

int
vtkCrackWidthFilter::TetraIntersectWithLine(vtkTetra *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  int intersection = 0;
  double tTemp, pc[3], xTemp[3], a1[3], a2[3], a3[3];
  int faceNum, *faceIds;

  t = VTK_LARGE_FLOAT;

  for (faceNum = 0; faceNum < 4; faceNum++)
    {
    tTemp = VTK_LARGE_FLOAT;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        }
      }
    }
  return intersection;
}

int
vtkCrackWidthFilter::VoxelIntersectWithLine(vtkVoxel *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  double minPt[3], maxPt[3];
  double bounds[6], p21[3];
  int i;
 
  cell->Points->GetPoint(0, minPt);
  cell->Points->GetPoint(7, maxPt);
 
  for (i=0; i<3; i++)
    {
    p21[i] = p2[i] - p1[i];
    bounds[2*i] = minPt[i];
    bounds[2*i+1] = maxPt[i];
    }
 
  if ( ! vtkBox::IntersectBox(bounds, p1, p21, x, t) )
    {
    return 0;
    }
 
  return 1;
}

int
vtkCrackWidthFilter::HexIntersectWithLine(vtkHexahedron *cell, double p1[3], 
    double p2[3], double &t, double x[3])
{
  int intersection = 0, faceNum, *faceIds;
  double tTemp, xTemp[3] = {0., 0., 0}, pc[3] = { 0., 0., 0.};
  double a1[3], a2[3], a3[3], a4[3];
  t = VTK_LARGE_FLOAT;
  for (faceNum = 0; faceNum < 6; faceNum++)
    {
    tTemp = VTK_LARGE_FLOAT;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    cell->Points->GetPoint(faceIds[3], a4);
    this->quad->Points->SetPoint(0, a1);
    this->quad->Points->SetPoint(1, a2);
    this->quad->Points->SetPoint(2, a3);
    this->quad->Points->SetPoint(3, a4);
    if (this->QuadIntersectWithLine(quad, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        }
      }
    }
  return intersection;  
}

int
vtkCrackWidthFilter::WedgeIntersectWithLine(vtkWedge *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  int intersection = 0;
  double tTemp, xTemp[3], pc[3], a1[3], a2[3], a3[3], a4[3];
  int faceNum, *faceIds;

  t = VTK_LARGE_FLOAT;

  // faces 0 & 1 are triangle-faces, intersect them first
  for (faceNum = 0; faceNum < 2; faceNum++)
    {
    tTemp = VTK_LARGE_FLOAT;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1); 
    cell->Points->GetPoint(faceIds[1], a2); 
    cell->Points->GetPoint(faceIds[2], a3); 
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        }
      }
    }

  // faces 2 -4 are quads, now intersect them
  for (faceNum = 2; faceNum < 5; faceNum++)
    {
    tTemp = VTK_LARGE_FLOAT;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    cell->Points->GetPoint(faceIds[3], a4);
    this->quad->Points->SetPoint(0, a1);
    this->quad->Points->SetPoint(1, a2); 
    this->quad->Points->SetPoint(2, a3); 
    this->quad->Points->SetPoint(3, a4); 
    if (this->QuadIntersectWithLine(this->quad, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        }
      }
    }
  return intersection;
}

int
vtkCrackWidthFilter::PyramidIntersectWithLine(vtkPyramid *cell, double p1[3], 
    double p2[3], double& t, double x[3])
{
  int intersection = 0;
  int faceNum;
  int *faceIds;
  double xTemp[3], weights[5], pc[3], dist2, tTemp;
  double a1[3], a2[3], a3[3], a4[3];

  t = VTK_LARGE_FLOAT;

  // faces 1-4 are triangles, intersect them first.
  for (faceNum = 1; faceNum < 5; faceNum++)
    {
    tTemp = VTK_LARGE_FLOAT;
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp))
      {
      if (tTemp < t)
        {
        intersection = 1;
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        }
      }
    }
  // now intersect quad face 
  faceIds = cell->GetFaceArray(0);
  cell->Points->GetPoint(faceIds[0], a1);
  cell->Points->GetPoint(faceIds[1], a2);
  cell->Points->GetPoint(faceIds[2], a3);
  cell->Points->GetPoint(faceIds[3], a4);
  this->quad->Points->SetPoint(0, a1); 
  this->quad->Points->SetPoint(1, a2); 
  this->quad->Points->SetPoint(2, a3); 
  this->quad->Points->SetPoint(3, a4); 

  tTemp = VTK_LARGE_FLOAT;
  if (this->QuadIntersectWithLine(this->quad, p1, p2, tTemp, xTemp))
    {
    if (tTemp < t)
      {
      intersection = 1;
      t = tTemp;
      x[0] = xTemp[0];
      x[1] = xTemp[1];
      x[2] = xTemp[2];
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
vtkCrackWidthFilter::LineLineIsect(const double *p1, const double *p2, 
    const double *p3, const double *p4, double *isect)
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
vtkCrackWidthFilter::EdgeLineIsect(vtkCell *cell, const double *p1, 
                                const double *p2, double *x)
{
    double p3[3], p4[3];
    bool isectedEdge = false;
    int numEdges = cell->GetNumberOfEdges();
    int i;

    for (i = 0; i < numEdges && !isectedEdge; i++)
    {
        cell->GetEdge(i)->Points->GetPoint(0, p3);
        cell->GetEdge(i)->Points->GetPoint(1, p4);
        isectedEdge = (bool) LineLineIsect(p1, p2, p3, p4, x);
    }
    return (isectedEdge ? 1 : 0); 
}

double
vtkCrackWidthFilter::GetMaxCrackWidth(int whichCrack)
{
    switch(whichCrack)
    {
        case 0: return this->MaxCrack1Width;
        case 1: return this->MaxCrack2Width;
        case 2: return this->MaxCrack3Width;
        default: return this->MaxCrack1Width;
    }
}

