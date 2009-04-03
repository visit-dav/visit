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

#include <vtkVisItInterpolatedVelocityField.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkGenericCell.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItPointLocator.h>

#include <vtkVisItUtility.h>
#include <DebugStream.h>


vtkVisItInterpolatedVelocityField* vtkVisItInterpolatedVelocityField::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVisItInterpolatedVelocityField");
  if(ret)
    {
    return (vtkVisItInterpolatedVelocityField*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVisItInterpolatedVelocityField;
}

vtkVisItInterpolatedVelocityField::vtkVisItInterpolatedVelocityField()
{
   ds = NULL;
   lastCell = 0;
   pcoords[0] = pcoords[1] = pcoords[2] = 0.;
   for (int i = 0 ; i < 1024 ; i++)
      weights[i] = 0.;
   locator = NULL;
}

vtkVisItInterpolatedVelocityField::~vtkVisItInterpolatedVelocityField()
{
   if (ds != NULL)
       ds->Delete();
   if (locator != NULL)
       locator->Delete();
}

void
vtkVisItInterpolatedVelocityField::SetDataSet(vtkDataSet *ds_)
{
   if (ds != NULL)
       ds->Delete();
   if (locator != NULL)
   {
       locator->Delete();
       locator = NULL;
   }
   ds = ds_;
   ds->Register(NULL);
}

bool
vtkVisItInterpolatedVelocityField::Evaluate(double *pt, double *vel)
{
    if (ds == NULL)
    {
        debug1 <<" vtkVisItInterpolatedVelocityField::No data set to evaluate!" << endl;
        return false;
    }
    vtkDataArray *vectors =  ds->GetPointData()->GetVectors();
    if (ds == NULL)
    {
        debug1 <<" vtkVisItInterpolatedVelocityField::Can't locate vectors to interpolate" << endl;
        return false;
    }
    
    int cell = -1;

    // This is vtkVisItUtility::FindCell, except we cache the locator.
    // I should probably refactor that method, but I'm short on time.
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int ijk[3];
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid*)ds;
        if (vtkVisItUtility::ComputeStructuredCoordinates(rgrid, pt, ijk) == 0)
            return false;
        cell = rgrid->ComputeCellId(ijk);
        if (cell < 0)
            return false;
    }
    else
    {
        // Pulled this from vtkPointSet::FindCell, because for
        // some of our data, their default 'MAXWALK' is too small.
        int nCells = ds->GetNumberOfCells();
        if (nCells == 0)
        {
            return false;
        }

        vtkIdType ptId, cellId;
        vtkCell *cellVTK;
        int walk, found = -1, subId;
        double pcoords[3], diagLen, tol;
        double closestPoint[3], dist2;
        vtkIdList *cellIds, *ptIds;

        if (locator == NULL)
        {
            locator = vtkVisItPointLocator::New();
            locator->SetDataSet(ds);
            locator->SetIgnoreDisconnectedPoints(1);
            locator->BuildLocator();
        }

        diagLen = ds->GetLength();
        if (nCells != 0)
            tol = diagLen / (double) nCells;
        else
            tol = 1e-6;

        //
        // Finding ONLY the single closest point won't work for this use-case
        // if any adjacent cells share identical point coordinates with  
        // different point Ids.  (E.g. physically adjacent cells which are not
        // logically connected.) So find 8 closest-points and work from there.
        //
        vtkIdList *closestPoints = vtkIdList::New();
        locator->FindClosestNPoints(8, pt, closestPoints);
        if (closestPoints->GetNumberOfIds() == 0)
        {
            closestPoints->Delete();
            return false;
        }

        double minDist2 = FLT_MAX;
        cellIds = vtkIdList::New(); 
        cellIds->Allocate(8, 100);
        ptIds = vtkIdList::New();
        ptIds->Allocate(8, 100);
    
        for (int z = 0; z < closestPoints->GetNumberOfIds() && found == -1; z++)
        {
            ptId = closestPoints->GetId(z);
            ds->GetPointCells(ptId, cellIds);
            if (cellIds->GetNumberOfIds() > 0)
            {
                cellId = cellIds->GetId(0);
                cellVTK = ds->GetCell(cellId);
                int evaluate = cellVTK->EvaluatePosition
                    (pt, closestPoint, subId, pcoords, dist2, weights);

                if (evaluate == 1 && dist2 <= tol && dist2 < minDist2)
                { 
                    found = cellId; 
                    minDist2 = dist2;
                } 
            }
            int MAXWALK = 50;
            if (found == -1 && cellIds->GetNumberOfIds() > 0)
            {
                for (walk = 0; walk < MAXWALK && minDist2 != 0. ; walk++)
                {
                    cellVTK->CellBoundary(subId, pcoords, ptIds);
                    ds->GetCellNeighbors(cellId, ptIds, cellIds);
                    if (cellIds->GetNumberOfIds() > 0)
                    {
                        cellId = cellIds->GetId(0);
                        cellVTK = ds->GetCell(cellId);
                    }
                    else
                    {
                        break; // outside of data
                    }
                    if (cellVTK)
                    {
                        int eval = cellVTK->EvaluatePosition
                            (pt, closestPoint, subId, pcoords, dist2, weights);
                        if (eval == 1 && dist2 <= tol && dist2 < minDist2)
                        {
                            minDist2 = dist2;
                            found = cellId;
                        }
                    }

                }
            }
        }

        ptIds->Delete();
        cellIds->Delete();
        closestPoints->Delete();
        cell = found;
    }
   
    if (cell < 0)
        return false;

    vtkGenericCell *GenCell = vtkGenericCell::New();
    ds->GetCell(cell, GenCell);
    lastCell = cell;

    int numPts = GenCell->GetNumberOfPoints();

    double closestPoint[3];
    int subId;
    double dist2;
    int val = GenCell->EvaluatePosition(pt, closestPoint, subId, pcoords, dist2, weights);
    if (val <= 0)
        return false;
    // interpolate the vectors
    vel[0] = vel[1] = vel[2];
    double vec[3];
    for (int j=0; j < numPts; j++)
    {
      int id = GenCell->PointIds->GetId(j);
      vectors->GetTuple(id, vec);
      for (int i=0; i < 3; i++)
      {
        vel[i] +=  vec[i] * weights[j];
      }
    }

    return true;
}
