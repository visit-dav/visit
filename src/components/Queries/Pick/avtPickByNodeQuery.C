// ************************************************************************* //
//                           avtPickByNodeQuery.C                            //
// ************************************************************************* //

#include <avtPickByNodeQuery.h>

#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkPoints.h>
#include <vtkIntArray.h>
#include <vtkVisItUtility.h>

#include <avtMatrix.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <BadNodeException.h>



// ****************************************************************************
//  Method: avtPickByNodeQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:    May 10, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPickByNodeQuery::avtPickByNodeQuery()
{
}


// ****************************************************************************
//  Method: avtPickByNodeQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtPickByNodeQuery::~avtPickByNodeQuery()
{
}


// ****************************************************************************
//  Method: avtPickByNodeQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul  8 16:42:05 PDT 2004
//    Changed the way that nodeid is modified when accounting for ghost zones.
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004 
//    Removed 'needRealId' test, no longer needed (we are reporting ghost
//    zones when ghostType == AVT_HAS_GHOSTS). 
//
// ****************************************************************************

void
avtPickByNodeQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != pickAtts.GetDomain() || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    int nodeid = pickAtts.GetElementNumber();
    int maxEls = ds->GetNumberOfPoints(); 

    // Verify the node number is in range.
    if (nodeid < 0 || nodeid >= maxEls)
    {
        EXCEPTION2(BadNodeException, nodeid, maxEls);
    } 

    if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
    {
        GetNodeCoords(ds, nodeid);    
        if (RetrieveZones(ds, nodeid))
        {
            RetrieveVarInfo(ds);
            pickAtts.SetFulfilled(true);
        }
        else
        {
            // the incidient zones could not be found, no further processing required.
            // SetDomain and ElementNumber to -1 to indicate failure. 
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            pickAtts.SetErrorMessage("Pick encountered an internal "
                "error (could not find incident elements).\n"
                "Please contact a VisIt developer"); 
            pickAtts.SetError(true);
            return; 
        }
    }

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
    // PickPoint determines where on the screen the pick letter will be
    // placed.  This should be the actual node coordinates if the plot was 
    // NOT transformed.
    //
    // If the plot was transformed && the transform is available, then transform
    // the node coordinates.
    //
    // If the plot was transformed && inverseTransform is NOT available,
    // there is no way to determine the location of the picked node in
    // transformed space, so  ????????
    //
    float coord[3];
    vtkVisItUtility::GetPoints(ds)->GetPoint(nodeid, coord);
    pickAtts.SetCellPoint(coord);
    if (transform != NULL)
    {
        avtVector v1(coord);
        v1 = (*transform) *v1;
        coord[0] = v1.x;
        coord[1] = v1.y;
        coord[2] = v1.z;
        pickAtts.SetPickPoint(coord);
    }
    else if (pickAtts.GetNeedTransformMessage())
    {
        //
        // Points were transformed, but we don't need the message because
        // we are displaying the real node coords to the user in pick output.
        //
        pickAtts.SetNeedTransformMessage(false);
        //
        // We need to query the actual data to find a location for the
        // pick letter.  
        //
        pickAtts.SetNeedActualCoords(true);
        if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
        {
            //
            // We have used a 'ghosted' id to query here, but have set the
            // element number back to what the user requested.  Tell the
            // locator code that it should use the RealElementNumber in
            // determining the node location.
            //
            pickAtts.SetRealElementNumber(nodeid);
        }
    }
    else
    {
        pickAtts.SetPickPoint(coord);
    }
}


// ****************************************************************************
//  Method: avtPickByNodeQuery::Preparation
//
//  Purpose:
//    Allows this pick query to modify pickAtts before filters are applied.    
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPickByNodeQuery::Preparation()
{
    int dom = pickAtts.GetDomain() - blockOrigin;
    pickAtts.SetDomain(dom < 0 ? 0 : dom);
}


// ****************************************************************************
//  Method: avtPickByNodeQuery::SetTransform
//
//  Purpose:
//    Set the transform needed by this pick. 
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPickByNodeQuery::SetTransform(const avtMatrix *m)
{
    transform = m;
}

