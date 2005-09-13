#include "vtkCSGCell.h"
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkObjectFactory.h>

vtkCxxRevisionMacro(vtkCSGCell, "$Revision: 1.17 $");
vtkStandardNewMacro(vtkCSGCell);

int vtkCSGCell::EvaluatePosition(float  vtkNotUsed(x)[3], 
                                  float  vtkNotUsed(closestPoint)[3],
                                  int&   vtkNotUsed(subId), 
                                  float  vtkNotUsed(pcoords)[3], 
                                  float& vtkNotUsed(dist2), 
                                  float  *vtkNotUsed(weights))
{
  return 0;
}

void vtkCSGCell::EvaluateLocation(int&  vtkNotUsed(subId), 
                                   float vtkNotUsed(pcoords)[3],
                                   float vtkNotUsed(x)[3],
                                   float *vtkNotUsed(weights))
{
}

int vtkCSGCell::CellBoundary(int vtkNotUsed(subId), 
                            float vtkNotUsed(pcoords)[3], 
                            vtkIdList *vtkNotUsed(pts))
{
  return 0;
}

void vtkCSGCell::Contour(float vtkNotUsed(value), 
                           vtkDataArray *vtkNotUsed(cellScalars), 
                           vtkPointLocator *vtkNotUsed(locator),
                           vtkCellArray *vtkNotUsed(verts), 
                           vtkCellArray *vtkNotUsed(lines), 
                           vtkCellArray *vtkNotUsed(polys), 
                           vtkPointData *vtkNotUsed(inPd),
                           vtkPointData *vtkNotUsed(outPd),
                           vtkCellData *vtkNotUsed(inCd),
                           vtkIdType vtkNotUsed(cellId), 
                           vtkCellData *vtkNotUsed(outCd))
{
}

// Project point on line. If it lies between 0<=t<=1 and distance off line
// is less than tolerance, intersection detected.
int vtkCSGCell::IntersectWithLine(float vtkNotUsed(p1)[3], 
                                   float vtkNotUsed(p2)[3], 
                                   float vtkNotUsed(tol), 
                                   float& vtkNotUsed(t),
                                   float vtkNotUsed(x)[3], 
                                   float pcoords[3], 
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
                            float vtkNotUsed(pcoords)[3], 
                            float *vtkNotUsed(values), 
                            int vtkNotUsed(dim), 
                            float *vtkNotUsed(derivs))
{
}

void vtkCSGCell::Clip(float vtkNotUsed(value), 
                        vtkDataArray *vtkNotUsed(cellScalars), 
                        vtkPointLocator *vtkNotUsed(locator), 
                        vtkCellArray *vtkNotUsed(verts),    
                        vtkPointData *vtkNotUsed(inPD),
                        vtkPointData *vtkNotUsed(outPD),
                        vtkCellData *vtkNotUsed(inCD), 
                        vtkIdType vtkNotUsed(cellId),
                        vtkCellData *vtkNotUsed(outCD),
                        int vtkNotUsed(insideOut))
{
}
