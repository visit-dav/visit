/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <vtkVisItCellLocator.h>

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
   doPathlines = false;
   nextTimeName = "";
   curTime = 0.;
   nextTime = 1.;
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

void
vtkVisItInterpolatedVelocityField::SetLocator(vtkVisItCellLocator *loc)
{
   if (locator != NULL)
   {
       locator->Delete();
       locator = NULL;
   }
   locator = loc;
   if (locator != NULL)
       locator->Register(NULL);
}

bool
vtkVisItInterpolatedVelocityField::Evaluate(double *pt, double *vel, double t)
{
    if (ds == NULL)
    {
        debug1 <<" vtkVisItInterpolatedVelocityField::No data set to evaluate!" << endl;
        return false;
    }
    vtkDataArray *vectors =  ds->GetPointData()->GetVectors();
    vtkDataArray *vectors2 =  NULL;
    if (doPathlines)
        vectors2 = ds->GetPointData()->GetArray(nextTimeName.c_str());
    if (vectors == NULL)
    {
        debug1 <<" vtkVisItInterpolatedVelocityField::Can't locate vectors to interpolate" << endl;
        return false;
    }
    if (doPathlines && vectors2 == NULL)
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
        if (locator == NULL)
        {
            locator = vtkVisItCellLocator::New();
            locator->SetDataSet(ds);
            locator->IgnoreGhostsOn();
            locator->BuildLocator();
        }

        double rad = 1e-6, dist=0.0;
        double resPt[3]={0.0,0.0,0.0};
        int subId = 0;
        int success = locator->FindClosestPointWithinRadius(pt, rad, resPt,
                                                                cell, subId, dist);
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
    vel[0] = vel[1] = vel[2] = 0;
    double vec[3];
    for (int j=0; j < numPts; j++)
    {
      int id = GenCell->PointIds->GetId(j);
      vectors->GetTuple(id, vec);
      for (int i=0; i < 3; i++)
      {
        vel[i] += vec[i] * weights[j];
      }
    }
    if (doPathlines)
    {
      double vel2[3] = { 0, 0, 0 };
      double vel1[3] = { vel[0], vel[1], vel[2] };
      for (int j=0; j < numPts; j++)
      {
        int id = GenCell->PointIds->GetId(j);
        vectors2->GetTuple(id, vec);
        for (int i=0; i < 3; i++)
        {
          vel2[i] += vec[i] * weights[j];
        }
      }
      double prop1 = 1. - (t - curTime) / (nextTime - curTime);
      vel[0] = prop1*vel1[0] + (1-prop1)*vel2[0];
      vel[1] = prop1*vel1[1] + (1-prop1)*vel2[1];
      vel[2] = prop1*vel1[2] + (1-prop1)*vel2[2];
    }
    GenCell->Delete();

    return true;
}
