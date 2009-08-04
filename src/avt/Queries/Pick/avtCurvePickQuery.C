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

// ************************************************************************* //
//                           avtCurvePickQuery.C                            //
// ************************************************************************* //

#include <avtCurvePickQuery.h>

#include <vtkCellArray.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <avtOriginatingSource.h>
#include <avtCurveConstructorFilter.h>


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
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtCurvePickQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

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
//    Hank Childs, Thu Mar 10 10:30:35 PST 2005
//    Fix memory leak.
//    
//    Kathleen Bonnell, Mon Jul 31 11:39:05 PDT 2006 
//    Turn 1D RectilinarGrid into PolyData to perform the pick. 
//    
// ****************************************************************************

void
avtCurvePickQuery::Execute(vtkDataSet *inDS, const int dom)
{
    if (inDS == NULL)
    {
        return;
    }

    vtkDataSet *ds = inDS;
    if (inDS->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkDataArray *xc = vtkRectilinearGrid::SafeDownCast(ds)->
                           GetXCoordinates();
        vtkDataArray *sc = ds->GetPointData()->GetScalars();

        int nPts = xc->GetNumberOfTuples();

        vtkPoints *pts = vtkPoints::New();
        pts->SetDataType(xc->GetDataType());
        pts->SetNumberOfPoints(nPts);

        vtkCellArray *verts = vtkCellArray::New();
        vtkCellArray *lines = vtkCellArray::New();
        verts->Allocate(nPts);
        lines->Allocate(nPts-1);
        vtkIdType ptIds[2];
        for (int i = 0; i < nPts; i++)
        {
             pts->SetPoint(i, xc->GetTuple1(i), sc->GetTuple1(i), 0.); 
             ptIds[0] = i; 
             verts->InsertNextCell(1, ptIds);
             if (i < nPts-1)
             {
                 ptIds[1] = i+1; 
                 lines->InsertNextCell(2, ptIds);
             }
        }

        ds = vtkPolyData::New();
        ((vtkPolyData*)ds)->SetPoints(pts);
        ((vtkPolyData*)ds)->SetVerts(verts);
        ((vtkPolyData*)ds)->SetLines(lines);

        pts->Delete();
        verts->Delete();
        lines->Delete();
    }

    avtDataRequest_p dataRequest = 
        GetInput()->GetOriginatingSource()->GetFullDataRequest();
    pickAtts.SetTimeStep(dataRequest->GetTimestep());
 
    double pt1[3] = { 0., 0., 0.};
    double pt2[3] = { 0., 0., 0.};

    int pointId = pickAtts.GetElementNumber();

    if (pointId == -1)
    {
        pointId = FindClosestPoint(ds);
    }

    if (pointId != -1)
    {
        if (pickAtts.GetPickType() == PickAttributes::CurveNode)
        {
            ds->GetPoint(pointId, pt1);
        }
        else
        {
            vtkIdList *ptIds = vtkIdList::New();
            ds->GetCellPoints(pointId, ptIds);
            ds->GetPoint(ptIds->GetId(0), pt1);
            ds->GetPoint(ptIds->GetId(1), pt2);
            ptIds->Delete();
        }
        pickAtts.SetNodePoint(pt1);
        pickAtts.SetCellPoint(pt2);
        pickAtts.SetPickPoint(pt1);
        pickAtts.SetDimension(2);
        pickAtts.SetFulfilled(true);
        foundDomain = dom; 
    }
    if (inDS->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        ds->Delete();
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
//    Hank Childs, Fri Feb 15 15:55:06 PST 2008
//    Initialize variable to make Klocwork happy.
//
// ****************************************************************************

int
avtCurvePickQuery::FindClosestPoint(vtkDataSet *ds)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    double *rayPt1 = pickAtts.GetRayPoint1();

    vtkPointLocator *pointLocator = vtkPointLocator::New(); 
    pointLocator->SetDataSet(ds);
    pointLocator->BuildLocator();

    vtkIdType foundPoint = -1; 
    double pt[3] = {rayPt1[0], rayPt1[1], 0.};
    double dist = minDist, rad = minDist;
    foundPoint = pointLocator->FindClosestPointWithinRadius(rad, pt, dist);

    if (foundPoint >= 0 && dist < minDist)
    {
        minDist = dist;
    }

    pointLocator->Delete();
    return foundPoint;
}
