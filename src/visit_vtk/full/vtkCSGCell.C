/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "vtkCSGCell.h"
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkObjectFactory.h>

vtkCxxRevisionMacro(vtkCSGCell, "$Revision: 1.17 $");
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

void vtkCSGCell::Contour(double vtkNotUsed(value), 
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

void vtkCSGCell::Clip(double vtkNotUsed(value), 
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
