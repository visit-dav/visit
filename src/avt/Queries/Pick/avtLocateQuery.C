// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtLocateQuery.C                              //
// ************************************************************************* //

#include <avtLocateQuery.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkIdList.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>

#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <DebugStream.h>
#include <NonQueryableInputException.h>
#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>



// ****************************************************************************
//  Method: avtLocateQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//
// ****************************************************************************

avtLocateQuery::avtLocateQuery()
{
    foundElement = foundDomain = -1;
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtLocateQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
// ****************************************************************************

avtLocateQuery::~avtLocateQuery()
{
    ;
}

// ****************************************************************************
//  Method: avtLocateQuery::VerifyInput
//
//  Purpose:
//    Rejects non-queryable input && input that has topological dimension == 0
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov 17 13:39:42 PST 2005
//    Don't test Input's topodim, may be unreliable.  Use value stored in
//    pickatts instead, it came directly from the plot, not the pipeline.
//
// ****************************************************************************

void
avtLocateQuery::VerifyInput()
{
    avtDataObjectQuery::VerifyInput();
    if (pickAtts.GetInputTopoDim() == 0)
    {
        EXCEPTION1(NonQueryableInputException,
            "Requires plot with topological dimension > 0.");
    }
}

// ****************************************************************************
//  Method: avtLocateQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
//    Kathleen Bonnell, Thu Feb 24 16:04:45 PST 2011
//    Use rectilinearGridTransform matrix to transform cell/pick points to
//    correct location if necessary.
//
// ****************************************************************************

void
avtLocateQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

    foundElement = foundDomain = -1;
    minDist = +FLT_MAX;

    avtDataAttributes &dAtts = GetInput()->GetInfo().GetAttributes();

    if (dAtts.GetRectilinearGridHasTransform())
    {
        if (!(dAtts.HasInvTransform() && dAtts.GetCanUseInvTransform()) &&
            !(dAtts.HasInvTransform() && dAtts.GetCanUseInvTransform()))
        {
            // mesh transformed in mapper, so need to apply the inverse
            // of that transform to the ray points so that Pick will be
            // working in the correct space.
            debug3 << "Pick's ray points transformed by "
                   << "rectilinearGridTransform." << endl;
            avtMatrix m(dAtts.GetRectilinearGridTransform());
            m.Inverse();
            avtVector r1(pickAtts.GetRayPoint1());
            avtVector r2(pickAtts.GetRayPoint2());
            r1 = m * r1;
            r2 = m * r2;
            double rp1[3];
            double rp2[3];
            for (int i = 0; i < 3; ++i)
            {
                rp1[i] = r1[i];
                rp2[i] = r2[i];
            }
            pickAtts.SetRayPoint1(rp1);
            pickAtts.SetRayPoint2(rp2);
        }
    }
}


// ****************************************************************************
//  Method: avtLocateQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov  4 15:18:15 PST 2004
//    Set a flag in pick atts specifiying whether or not this code was
//    successful in finding an intersection.
//
// ****************************************************************************

void
avtLocateQuery::PostExecute(void)
{
    int foundIsect = 0;
    if (ThisProcessorHasMinimumValue(minDist) && minDist != +FLT_MAX)
    {
        pickAtts.SetDomain(foundDomain);
        pickAtts.SetElementNumber(foundElement);
        foundIsect = 1;
    }
    //
    // Make sure all processors are on the same page.
    //
    foundIsect = UnifyMaximumValue(foundIsect);
    pickAtts.SetLocationSuccessful(foundIsect);
}


// ****************************************************************************
//  Method: avtLocateQuery::SetPickAtts
//
//  Purpose:
//      Sets the pickAtts to the passed values.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
// ****************************************************************************

void
avtLocateQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts =  *pa;
}


// ****************************************************************************
//  Method: avtLocateQuery::GetPickAtts
//
//  Purpose:
//    Retrieve the PickAttributes being used in this query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
// ****************************************************************************

const PickAttributes *
avtLocateQuery::GetPickAtts()
{
    return &pickAtts;
}



// ****************************************************************************
//  Method: avtLocateQuery::RGRidIsect
//
//  Purpose:
//    Determine the intersection of a point or ray with a rectilinear grid.
//
//  Arguments:
//    ds        The dataset to search.
//    dist      The distance from the isect to the ray-origin.
//    isect     The intersection point.
//
//  Returns:
//    Cell id of intersected cell. (-1 if ds not intersected, or cell
//    was a ghost).
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 27 15:46:29 PDT 2005
//    Made method return int, removed ijk from arguments.  Test for ghosts.
//
//    Hank Childs, Tue Sep 11 13:11:17 PDT 2007
//    Code around VTK bug with ComputeCellId.
//
//    Kathleen Biagas, The Dec 7, 2023
//    Swap vtkBox::IntersectBox with vtkVisItUtility::IntersectBox. The VTK 9
//    version of the former utilizes a tolerance that adjusts the bounds and
//    interferes with results (especially for 2D).
//
// ****************************************************************************

int
avtLocateQuery::RGridIsect(vtkRectilinearGrid *rgrid, double &dist,
                           double isect[3])
{
    int i, ijk[3];
    double t, dsBounds[6], rayDir[3];
    double *rayPt1 = pickAtts.GetRayPoint1();
    double *rayPt2 = pickAtts.GetRayPoint2();
    int success = 0;
    int cellId = -1;


    rgrid->GetBounds(dsBounds);

    if (rayPt1[0] == rayPt2[0] &&
        rayPt1[1] == rayPt2[1] &&
        rayPt1[2] == rayPt2[2])
    { /* WORLD COORDINATE LOCATE */
        success = vtkVisItUtility::ComputeStructuredCoordinates(rgrid, rayPt1,
                                                                ijk);
        if (success)
        {
            isect[0] = rayPt1[0];
            isect[1] = rayPt1[1];
            isect[2] = rayPt1[2];
            dist = 0;
        }
    }
    else
    { /* RAY-INTERSECTION LOCATE */
        for (i = 0; i < 3; i++)
        {
           rayDir[i] = rayPt2[i] - rayPt1[i];
        }
        if (vtkVisItUtility::IntersectBox(dsBounds, rayPt1, rayDir, isect, t))
        {
            success = vtkVisItUtility::ComputeStructuredCoordinates(rgrid,
                          isect, ijk);
            if (success)
            {
                dist = vtkMath::Distance2BetweenPoints(rayPt1, isect);
            }
        }
    }
    if (success)
    {
        // vtkRectilinearGrid::ComputeCellId is buggy for dimensions
        // of type 1xYxZ or Xx1xZ.
        int dims[3];
        rgrid->GetDimensions(dims);
        if (dims[0] == 1 && dims[1] == 1)
            cellId = ijk[2];
        else if (dims[0] == 1)
            cellId = ijk[2]*(dims[1]-1) + ijk[1];
        else if (dims[1] == 1)
            cellId = ijk[2]*(dims[0]-1) + ijk[0];
        else
            cellId = rgrid->ComputeCellId(ijk);
        vtkDataArray *ghosts = rgrid->GetCellData()->GetArray("avtGhostZones");
        if (ghosts && ghosts->GetTuple1(cellId) > 0)
        {
            cellId = -1;
        }
    }

    return cellId;
}


// ****************************************************************************
//  Method: avtLocateQuery::LocatorFindCell
//
//  Purpose:
//    Uses a locator to find the cell intersected by the pick ray.
//    Ignores ghost zones.
//
//  Arguments:
//    ds      The dataset to query.
//    dist    A place to store the distance along the ray of the
//            intersection point.
//    isect   A place to store the intersection point of the ray with the
//            dataset.
//
//  Returns:
//    The id of the cell that was intersected (-1 if none found).
//
//  Notes:  Moved vrom avtLocateCellQuery.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb  3 10:25:15 PST 2011
//    Don't send 'UserBounds' to cell locator if they are all zero, as this
//    means the actual extents could not be retrieved from the viewer plot.
//
//    Kathleen Bonnell, Thu Feb  3 11:34:30 PST 2011
//    PickAtts plotBounds now stored as a vector.
//
// ****************************************************************************

int
avtLocateQuery::LocatorFindCell(vtkDataSet *ds, double &dist, double *isect)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    double *rayPt1 = pickAtts.GetRayPoint1();
    double *rayPt2 = pickAtts.GetRayPoint2();
    dist = -1;

    vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New();
    cellLocator->SetIgnoreGhosts(true);
    cellLocator->SetDataSet(ds);
    //
    // Cells may have been removed, and unused points may still exist,
    // giving the dataset larger bounds than just the cell bounds, so
    // tell the locator to use the actual bounds retrieved from the
    // plot that originated this query.  The locator will use these
    // bounds only if they are smaller than the dataset bounds.
    //
    if (!pickAtts.GetPlotBounds().empty())
    {
        cellLocator->SetUserBounds(&pickAtts.GetPlotBounds()[0]);
    }
    cellLocator->BuildLocator();

    double pcoords[3] = {0., 0., 0.}, ptLine[3] = {0., 0., 0.};
    int subId = 0, success = 0;

    vtkIdType foundCell;
    if (rayPt1[0] == rayPt2[0] &&
        rayPt1[1] == rayPt2[1] &&
        rayPt1[2] == rayPt2[2])
    { /* WORLD COORD LOCATE */
        cellLocator->FindClosestPoint(rayPt1, ptLine, foundCell,
                                     subId, dist);
        if (foundCell >= 0 && dist >= 0)
        {
            success = 1;
            isect[0] = rayPt1[0];
            isect[1] = rayPt1[1];
            isect[2] = rayPt1[2];
        }
    }
    else
    { /* RAY-INTERSECT LOCATE */
        success = cellLocator->IntersectWithLine(rayPt1, rayPt2, dist,
                                     isect, pcoords, subId, foundCell);
    }

    cellLocator->Delete();
    if (success)
        return foundCell;
    else
        return -1;
}


// ****************************************************************************
//  Method: avtLocateQuery::RayIntersectsDataSet
//
//  Purpose:
//    Determines if the ray stored in PickAtts intersects the passed dataset.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 6, 2004
//
//  Modifications:
//    Kathleen Biagas, The Dec 7, 2023
//    Swap vtkBox::IntersectBox with vtkVisItUtility::IntersectBox. The VTK 9
//    version of the former utilizes a tolerance that adjusts the bounds and
//    interferes with results (especially for 2D).
//
// ****************************************************************************

bool
avtLocateQuery::RayIntersectsDataSet(vtkDataSet *ds)
{
    double *bnds = ds->GetBounds();
    double *pt1 = pickAtts.GetRayPoint1();
    double *pt2 = pickAtts.GetRayPoint2();
    double dir[3];
    for (int i = 0; i < 3; i++)
    {
        dir[i] = pt2[i] - pt1[i];
    }
    double dummy1[3], dummy2;
    return (vtkVisItUtility::IntersectBox(bnds, pt1, dir, dummy1, dummy2));
}


// ****************************************************************************
//  Method: avtLocateQuery::ApplyFilters
//
//  Purpose:
//      Retrieves the terminating source to use as input.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 14, 2006
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtLocateQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataRequest_p dataRequest =
        inData->GetOriginatingSource()->GetFullDataRequest();

    if (timeVarying || dataRequest->GetTimestep() == pickAtts.GetTimeStep())
        return avtDatasetQuery::ApplyFilters(inData);

    avtContract_p orig_contract = inData->GetOriginatingSource()->
        GetGeneralContract();

    avtDataRequest_p oldSpec = orig_contract->GetDataRequest();
    avtDataRequest_p newDS = new
        avtDataRequest(oldSpec, querySILR);

    newDS->SetTimestep(pickAtts.GetTimeStep());

    avtContract_p contract =
        new avtContract(newDS, queryAtts.GetPipeIndex());
    avtDataObject_p rv;
    CopyTo(rv, inData);
    rv->Update(contract);
    return rv;
}


// ****************************************************************************
//  Method: avtLocateQuery::ClosestLineToLine
//
//  Purpose:
//    Finds the line cell in the dataset closest to the line defined by pick
//    atts ray points, the distance to that line, and the intersection point.
//
//  Returns:
//    The element id (node or cell), or -1 if none found.
//
//  Arguments:
//    ds          The dataset to query.
//    returnNode  Whether or not to return the node coordinates in isect.
//    dist        A place to store the minimum distance.
//    isect       A place to store the intersection point:  The closest point
//                along the line, or the closest node coordinates if
//                returnNode is true.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 7, 2017
//
//  Modifications:
//
// ****************************************************************************

int
avtLocateQuery::ClosestLineToLine(vtkDataSet *ds, bool returnNode,
    double &dist, double isect[3])
{

    if (! ds->IsA("vtkPointSet"))
    {
        debug3 << "avtLocateQuery::ClosestLineToLine encountered dataset"
               << " that is not a vtkPointSet" << endl;
        return -1;
    }
    vtkIdType found= -1;

    double dist2, cp1[3], cp2[3], pt1[3], pt2[3], t1, t2;
    double *rayPt1 = pickAtts.GetRayPoint1();
    double *rayPt2 = pickAtts.GetRayPoint2();

    vtkPoints *pts =  vtkPointSet::SafeDownCast(ds)->GetPoints();
    vtkIdList *cellPtIds = vtkIdList::New();
    for (vtkIdType i = 0; i < ds->GetNumberOfCells(); ++i)
    {
        if (ds->GetCellType(i) != VTK_LINE)
        {
            debug3 << "avtLocateQuery::ClosestLineToLine encountered cell"
                   << " that is not a VTK_LINE" << endl;
            continue;
        }
        ds->GetCellPoints(i, cellPtIds);
        pts->GetPoint(cellPtIds->GetId(0), pt1);
        pts->GetPoint(cellPtIds->GetId(1), pt2);

        dist2 = vtkLine::DistanceBetweenLines(rayPt1, rayPt2, pt1, pt2,
            cp1, cp2, t1, t2);
        if (dist2 < dist && (t2 > 0 && t2 < 1))
        {
            if (returnNode)
            {
                if (t2 < 0.5)
                {
                    isect[0] = pt1[0];
                    isect[1] = pt1[1];
                    isect[2] = pt1[2];
                    found = cellPtIds->GetId(0);

                }
                else
                {
                    isect[0] = pt2[0];
                    isect[1] = pt2[1];
                    isect[2] = pt2[2];
                    found = cellPtIds->GetId(1);
                }
            }
            else
            {
                isect[0] = cp2[0];
                isect[1] = cp2[1];
                isect[2] = cp2[2];
                found = i;
            }
            dist = dist2;
        }
    }
    return found;
}
