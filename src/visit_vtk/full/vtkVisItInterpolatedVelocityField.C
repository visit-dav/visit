// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkVisItInterpolatedVelocityField.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkGenericCell.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItCellLocator.h>

#include <vtkVisItUtility.h>
#include <DebugStream.h>

static void
InterpVector(vtkGenericCell *cell, int numPts, vtkDataArray *vectors, double *weights, double *vel);

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

    bool nodeCenteredVector = true;
    vtkDataArray *vectors =  ds->GetPointData()->GetVectors();
    if (vectors == NULL)
    {
        vectors = ds->GetCellData()->GetVectors();
        if (vectors == NULL)
        {
            debug1 <<" vtkVisItInterpolatedVelocityField::Can't locate vectors to interpolate" << endl;
            return false;
        }
        nodeCenteredVector = false;
    }

    vtkDataArray *vectors2 =  NULL;
    if (doPathlines)
    {
        if (nodeCenteredVector)
        {
            vectors2 = ds->GetPointData()->GetArray(nextTimeName.c_str());
        }
        else
        {
            vectors2 = ds->GetCellData()->GetArray(nextTimeName.c_str());
        }

        if (vectors2 == NULL)
        {
            debug1 <<" vtkVisItInterpolatedVelocityField::Can't locate vectors to interpolate" << endl;
            return false;
        }
        if (vectors == vectors2)
        {
            debug1 << "vtkVisItInterpolatedVelocityField::Evaluate - Problem: The two vector fields are the same." << endl;
        }
    }
    
    vtkIdType cell = -1;

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
        locator->IgnoreGhostsOff();
        locator->FindClosestPointWithinRadius(pt, rad, resPt,
                                              cell, subId, dist);
    }
   
    if (cell < 0)
        return false;

    lastCell = cell;

    //For zone centered vector fields:
    if (!nodeCenteredVector)
    {
        vectors->GetTuple(cell, vel);
        
        if (doPathlines)
        {
            double vel2[3];
            vectors2->GetTuple(cell, vel2);
            double prop1 = 1. - (t - curTime) / (nextTime - curTime);
            vel[0] = prop1*vel[0] + (1-prop1)*vel2[0];
            vel[1] = prop1*vel[1] + (1-prop1)*vel2[1];
            vel[2] = prop1*vel[2] + (1-prop1)*vel2[2];
        }
    }
    else
    {
        vtkGenericCell *GenCell = vtkGenericCell::New();
        ds->GetCell(cell, GenCell);
        
        int numPts = GenCell->GetNumberOfPoints();

        double closestPoint[3], dist2;
        int subId;
        int val = GenCell->EvaluatePosition(pt, closestPoint, subId, pcoords, dist2, weights);

        if (val <= 0)
        {
            GenCell->Delete();
            return false;
        }
        // interpolate the vectors
        InterpVector(GenCell, numPts, vectors, weights, vel);

        if (doPathlines)
        {
            double vel2[3];
            InterpVector(GenCell, numPts, vectors2, weights, vel2);
            
            double prop1 = 1. - (t - curTime) / (nextTime - curTime);
            vel[0] = prop1*vel[0] + (1-prop1)*vel2[0];
            vel[1] = prop1*vel[1] + (1-prop1)*vel2[1];
            vel[2] = prop1*vel[2] + (1-prop1)*vel2[2];
        }
        GenCell->Delete();
    }

    return true;
}


static void
InterpVector(vtkGenericCell *cell, int numPts, vtkDataArray *vectors, double *weights, double *vel)
{
    vel[0] = vel[1] = vel[2] = 0;
    double vec[3];
    for (int j=0; j < numPts; j++)
    {
        int id = cell->PointIds->GetId(j);
        vectors->GetTuple(id, vec);
        for (int i=0; i < 3; i++)
            vel[i] += vec[i] * weights[j];
    }
}
