// ************************************************************************* //
//                           avtNodePickQuery.C                              //
// ************************************************************************* //

#include <avtNodePickQuery.h>

#include <float.h>

#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItUtility.h>

#include <avtMatrix.h>
#include <avtParallel.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <DebugStream.h>


using     std::string;


// ****************************************************************************
//  Method: avtNodePickQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 13, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNodePickQuery::avtNodePickQuery()
{
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtNodePickQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 13, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNodePickQuery::~avtNodePickQuery()
{
}


// ****************************************************************************
//  Method: avtNodePickQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 13, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 10 09:06:54 PDT 2004
//    When material-selection has been applied, ensure that RetriveVarInfo
//    will be using the correct zone ids for this dataset.
//
//    Kathleen Bonnell, Thu Aug 26 09:50:31 PDT 2004 
//    Handle case when pickatts.domain has not yet been set. (e.g. when
//    picking 2d contour or boundary plots.)
//
//    Kathleen Bonnell, Mon Aug 30 17:53:58 PDT 2004
//    Modified early-return test -- split into two, and use new flag 
//    skippedLocate. 
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004 
//    Removed 'needRealId' test, no longer needed (we are reporting ghost
//    zones when ghostType == AVT_HAS_GHOSTS). 
//
//    Kathleen Bonnell, Mon Dec  6 14:30:39 PST 2004 
//    Added special logic for when locate was skipped. 
//
// ****************************************************************************

void
avtNodePickQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL || pickAtts.GetFulfilled()) 
    {
        return;
    }
    if (dom != pickAtts.GetDomain() && !skippedLocate)
    {
        return;
    }

    int pickedNode = pickAtts.GetElementNumber();

    if (pickedNode == -1)
    {
        pickedNode = DeterminePickedNode(ds);
        if (pickedNode == -1 && pickAtts.GetDomain() != -1)
        {
            // the node could not be found, no further processing required.
            // SetDomain and ElementNumber to -1 to indicate failure. 
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            debug4 << "PICK BIG PROBLEM!  "
                   << "Could not find zone corresponding to pick point" << endl;
 
            pickAtts.SetErrorMessage("Pick encountered an internal "
                        "error (could not find closest node).\n"
                        "Please contact a VisIt developer"); 
            pickAtts.SetError(true);
            return; 
        }
    }
    if (skippedLocate)
    {
        if (pickedNode == -1)
        {
            return;
        }

        float dist = vtkMath::Distance2BetweenPoints(pickAtts.GetPickPoint(), 
                          vtkVisItUtility::GetPoints(ds)->GetPoint(pickedNode)); 
        if (dist < minDist)
        {
            minDist = dist;
            
        }
        else 
        {
            return;
        }
    }
    pickAtts.SetCellPoint(vtkVisItUtility::GetPoints(ds)->GetPoint(pickedNode)); 


    if (!pickAtts.GetMatSelected())
    {
        GetNodeCoords(ds, pickedNode);
        if (RetrieveZones(ds, pickedNode))
        {
            pickAtts.SetElementNumber(pickedNode);
            RetrieveVarInfo(ds);
            pickAtts.SetFulfilled(true);
        }
        else
        {
            // the zone/node could not be found, no further processing required.
            // SetDomain and ElementNumber to -1 to indicate failure. 
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            pickAtts.SetErrorMessage("Pick encountered an internal "
                    "error (could not determine incident zones).\n"
                    "Please contact a VisIt developer"); 
            pickAtts.SetError(true);
            return; 
        }
    }

    //
    //  The database needs a valid domain
    // 
    if (pickAtts.GetDomain() == -1)
        pickAtts.SetDomain(dom);
    
 
    //
    //  Allow the database to add any missing information.
    // 
    src->Query(&pickAtts);

    if (pickAtts.GetMatSelected())
    {
        //
        // The zone numbers stored in IncidentElements are not the correct
        // ones to use with this dataset ... get the correct ones to use
        // with RetrieveVarInfo, then reset them.
        //
        intVector pickedZones = pickAtts.GetIncidentElements();
        intVector currentZones = GetCurrentZoneForOriginal(ds, pickedZones);
        RetrieveVarInfo(ds, pickAtts.GetElementNumber(), currentZones); 
    }

    //
    // Set the domain and zone of pickAtts in relation to the
    // blockOrigin and cellOrigin of the problem.  
    // 
    if (singleDomain)
    {
        //
        // Indicate that there was only one domain.
        // We don't report the domain number for single-domain problems.
        //
        pickAtts.SetDomain(-1);
    }
    else
    {
        pickAtts.SetDomain(dom+blockOrigin);
    }

    //
    // CellPoint should now contain the actual node coordinates.
    // This is the value displayed to the user in the PickAtts output.
    // PickPoint determines where on the screen the pick letter will
    // be placed.  This should be the actual node coordinates (CellPoint)
    // if the plot was NOT transformed.
    //
    // If the plot was transformed && the inverseTransform is available,
    // use the NodePoint as determined by avtLocateCellQuery.
    //
    // If the plot was transformed && inverseTransform is NOT available,
    // there is no way to determine the location of the picked node in
    // transformed space, so leave PickPoint set to the intersection point
    // with the ray as determined by avtLocateCellQuery.
    //
    if (transform != NULL)
    {
        pickAtts.SetPickPoint(pickAtts.GetNodePoint());
    }
    else if (pickAtts.GetNeedTransformMessage())
    {
        //
        // Points were transformed, but we don't need the message because
        // we are displaying the node coords to the user in pick output.
        //
        pickAtts.SetNeedTransformMessage(false);
        pickAtts.SetPickPoint(pickAtts.GetNodePoint());
    }
    else 
    {
        pickAtts.SetPickPoint(pickAtts.GetCellPoint());
    }

    if (skippedLocate)
    {
        // reset so that future domains will also check.
        pickAtts.SetFulfilled(false);
        pickAtts.SetDomain(-1);
        pickAtts.SetElementNumber(-1);
        foundNode = pickedNode;
        foundDomain = dom;
    }
}


// ****************************************************************************
//  Method: avtNodePickQuery::DeterminePickedNode
//
//  Purpose:
//    Finds the closest node-point to the picked point.  
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//
//  Returns:
//    The node id closest to PickPoint. (-1 if there is some kind of failure).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 13, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec  6 14:30:39 PST 2004
//    Add logic to return -1 when pickpoint does not lie in dataset bounds,
//    and if 'FindPoint' returns a ghost node.
//    
// ****************************************************************************

int
avtNodePickQuery::DeterminePickedNode(vtkDataSet *ds)
{
    float *bnds = ds->GetBounds();
    float *pp = pickAtts.GetPickPoint();

    if (pp[0] < bnds[0] || pp[0] > bnds[1] ||
        pp[1] < bnds[2] || pp[1] > bnds[3] ||
        pp[2] < bnds[4] || pp[2] > bnds[5])
    {
        return -1;
    }
   
    int node = ds->FindPoint(pickAtts.GetPickPoint());
    vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::SafeDownCast(
        ds->GetPointData()->GetArray("avtGhostNodes"));
    if (ghostNodes && ghostNodes->GetValue(node) > 0)
    {
        return -1;
    }
    else
    {
        return node;
    }
}


// ****************************************************************************
//  Method: avtNodePickQuery::Preparation
//
//  Purpose:
//    Allows this pick to modify pickAtts before filters are applied.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

void
avtNodePickQuery::Preparation()
{
    //
    // Transform the point that will be used in locating the node. 
    //
    float *pickPoint  = pickAtts.GetPickPoint();
    if (transform != NULL)
    {
        //
        // Transform the intersection point back to original space.
        //
        avtVector v1(pickPoint);
        v1 = (*transform) * v1;
        pickPoint[0] = v1.x;
        pickPoint[1] = v1.y;
        pickPoint[2] = v1.z;
        //
        // Reset the pick point to the transformed point.
        //
        pickAtts.SetPickPoint(pickPoint);
    }
}


// ****************************************************************************
//  Method: avtNodePickQuery::SetInvTransform
//
//  Purpose:
//      Sets the inverseTransform, used to return correct pick points. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

void
avtNodePickQuery::SetInvTransform(const avtMatrix *m)
{
    transform = m;
}


// ****************************************************************************
//  Method: avtNodePickQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 6, 2004 
//
// ****************************************************************************

void
avtNodePickQuery::PreExecute(void)
{
    minDist = +FLT_MAX;
    foundNode = -1;
    foundDomain = -1;
}


// ****************************************************************************
//  Method: avtNodePickQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//      Sets pickAtts elementNumber, Domain, and fulfilled flag.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 6, 2004
//
// ****************************************************************************

void
avtNodePickQuery::PostExecute(void)
{
    if (skippedLocate)
    {
        if (ThisProcessorHasMinimumValue(minDist) && minDist != +FLT_MAX)
        {
            pickAtts.SetFulfilled(true);
            pickAtts.SetElementNumber(foundNode);
            if (singleDomain)
            {
                //
                // Indicate that there was only one domain.
                // We don't report the domain number for single-domain problems.
                //
                pickAtts.SetDomain(-1);
            }
            else
            {
                pickAtts.SetDomain(foundDomain+blockOrigin);
            }
        }
    }
    avtPickQuery::PostExecute();
}
