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



// Project point on line. If it lies between 0<=t<=1 and distance off line
// is less than tolerance, intersection detected.
int vtkCSGCell::IntersectWithLine(const double vtkNotUsed(p1)[3],
                                  const double vtkNotUsed(p2)[3],
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

