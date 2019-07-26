// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkCSGCell.h"
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCSGCell);

int vtkCSGCell::EvaluatePosition(double  vtkNotUsed(x)[3], 
                                  double  vtkNotUsed(closestPoint)[3],
                                  int&   vtkNotUsed(subId), 
                                  double  vtkNotUsed(pcoords)[3], 
                                  double& vtkNotUsed(dist2), 
                                  double  *vtkNotUsed(weights))
{
  return 0;
}

void vtkCSGCell::EvaluateLocation(int&  vtkNotUsed(subId), 
                                   double vtkNotUsed(pcoords)[3],
                                   double vtkNotUsed(x)[3],
                                   double *vtkNotUsed(weights))
{
}

int vtkCSGCell::CellBoundary(int vtkNotUsed(subId), 
                            double vtkNotUsed(pcoords)[3], 
                            vtkIdList *vtkNotUsed(pts))
{
  return 0;
}

void
vtkCSGCell::Contour(double, vtkDataArray*, vtkIncrementalPointLocator*, vtkCellArray*, vtkCellArray*, vtkCellArray*, vtkPointData*, vtkPointData*, vtkCellData*, vtkIdType, vtkCellData*)
{
}

void
vtkCSGCell::Clip(double, vtkDataArray*, vtkIncrementalPointLocator*, vtkCellArray*, vtkPointData*, vtkPointData*, vtkCellData*, vtkIdType, vtkCellData*, int)
{
}

// Project point on line. If it lies between 0<=t<=1 and distance off line
// is less than tolerance, intersection detected.
int vtkCSGCell::IntersectWithLine(double vtkNotUsed(p1)[3], 
                                   double vtkNotUsed(p2)[3], 
                                   double vtkNotUsed(tol), 
                                   double& vtkNotUsed(t),
                                   double vtkNotUsed(x)[3], 
                                   double pcoords[3], 
                                   int& vtkNotUsed(subId))
{
  pcoords[0] = -10.0;
  return 0;
}

int vtkCSGCell::Triangulate(int vtkNotUsed(index),
                             vtkIdList *ptIds, vtkPoints *pts)
{
  pts->Reset();
  ptIds->Reset();

  return 1;
}

void vtkCSGCell::Derivatives(int vtkNotUsed(subId), 
                            double vtkNotUsed(pcoords)[3], 
                            double *vtkNotUsed(values), 
                            int vtkNotUsed(dim), 
                            double *vtkNotUsed(derivs))
{
}

