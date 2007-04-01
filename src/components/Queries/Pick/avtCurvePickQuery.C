// ************************************************************************* //
//                           avtCurvePickQuery.C                            //
// ************************************************************************* //

#include <avtCurvePickQuery.h>

#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPointLocator.h>
#include <vtkVisItUtility.h>
#include <avtTerminatingSource.h>


#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtCurvePickQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 2, 2003
//
//  Modifications:
//
// ****************************************************************************

avtCurvePickQuery::avtCurvePickQuery()
{
    foundDomain = -1;
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtCurvePickQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCurvePickQuery::~avtCurvePickQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtCurvePickQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 2, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtCurvePickQuery::PreExecute(void)
{
    foundDomain = -1; 
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtCurvePickQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 2, 2003
//
//  Modifications:
//    
// ****************************************************************************

void
avtCurvePickQuery::PostExecute(void)
{
    if (ThisProcessorHasMinimumValue(minDist))
    {
        pickAtts.SetDomain(foundDomain);
    }
}


// ****************************************************************************
//  Method: avtCurvePickQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   December 2, 2003
//
//  Modifications:
//    
// ****************************************************************************

void
avtCurvePickQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }

    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();
    pickAtts.SetTimeStep(dspec->GetTimestep());
 
    float pt1[3] = { 0., 0., 0.};
    float pt2[3] = { 0., 0., 0.};

    int pointId = pickAtts.GetElementNumber();
    if (pointId == -1)
    {
        pointId = FindClosestPoint(ds);
    }

    if (pointId != -1)
    {
        vtkPoints *points = vtkVisItUtility::GetPoints(ds);
        float *p;
        if (pickAtts.GetPickType() == PickAttributes::CurveNode)
        {
            p = points->GetPoint(pointId);
            pt1[0] = p[0];
            pt1[1] = p[1];
            pt1[2] = p[2];
        }
        else
        {
            vtkIdList *ptIds = vtkIdList::New();
            ds->GetCellPoints(pointId, ptIds);
            p = points->GetPoint(ptIds->GetId(0));
            pt1[0] = p[0];
            pt1[1] = p[1];
            pt1[2] = p[2];
            p = points->GetPoint(ptIds->GetId(1));
            pt2[0] = p[0];
            pt2[1] = p[1];
            pt2[2] = p[2];
            ptIds->Delete();
        }
        pickAtts.SetNodePoint(pt1);
        pickAtts.SetCellPoint(pt2);
        pickAtts.SetPickPoint(pt1);
        pickAtts.SetDimension(2);
        pickAtts.SetFulfilled(true);
        foundDomain = dom; 
    }
}

// ****************************************************************************
//  Method: avtCurvePickQuery::SetPickAtts
//
//  Purpose:
//      Sets the pickAtts to the passed values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 2, 2003
//
// ****************************************************************************

void
avtCurvePickQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts =  *pa;
}


// ****************************************************************************
//  Method: avtCurvePickQuery::GetPickAtts
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 2, 2003
//
// ****************************************************************************

const PickAttributes *
avtCurvePickQuery::GetPickAtts() 
{
    return &pickAtts; 
}


// ****************************************************************************
//  Method: avtCurvePickQuery::FindClosestPoint
//
//  Purpose:
//    Uses a locator to find the closest point to the given point. 
//
//  Arguments:
//    ds      The dataset to query.
//    isect   A place to store the closest point coordinates.
//
//  Returns:
//    The id of the closest point (-1 if none found).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   December 2, 2003 
//
//  Modifications:
//    
// ****************************************************************************

int
avtCurvePickQuery::FindClosestPoint(vtkDataSet *ds)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    float *rayPt1 = pickAtts.GetRayPoint1();

    vtkPointLocator *pointLocator = vtkPointLocator::New(); 
    pointLocator->SetDataSet(ds);
    pointLocator->BuildLocator();

    vtkIdType foundPoint = -1; 
    float pt[3] = {rayPt1[0], rayPt1[1], 0.};
    float dist, rad = minDist;
    foundPoint = pointLocator->FindClosestPointWithinRadius(rad, pt, dist);

    if (foundPoint >= 0 && dist < minDist)
    {
        minDist = dist;
    }

    pointLocator->Delete();
    return foundPoint;
}


