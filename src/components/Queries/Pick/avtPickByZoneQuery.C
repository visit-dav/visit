// ************************************************************************* //
//                           avtPickByZoneQuery.C                            //
// ************************************************************************* //

#include <avtPickByZoneQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkIntArray.h>
#include <vtkVisItUtility.h>

#include <avtMatrix.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <BadCellException.h>



// ****************************************************************************
//  Method: avtPickByZoneQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtPickByZoneQuery::avtPickByZoneQuery()
{
}


// ****************************************************************************
//  Method: avtPickByZoneQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2004 
//
//  Modifications:
// ****************************************************************************

avtPickByZoneQuery::~avtPickByZoneQuery()
{
}


// ****************************************************************************
//  Method: avtPickByZoneQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul  8 16:42:05 PDT 2004
//    Changed the way that zoneid is modified when accounting for ghost zones.
//
//    Kathleen Bonnell, Tue Aug 10 13:14:38 PDT 2004 
//    Account for material selection.
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004 
//    Removed 'needRealId' test, no longer needed (we are reporting ghost
//    zones when ghostType == AVT_HAS_GHOSTS). 
//
//    Kathleen Bonnell, Wed Oct 20 17:10:21 PDT 2004 
//    Use vtkVisItUtility method to compute cell center. 
//
//    Kathleen Bonnell, Wed Dec 15 17:24:27 PST 2004 
//    Added logic to handle case when chosen zone is to be considered global.
//
//    Kathleen Bonnell, Tue Feb  8 08:29:44 PST 2005 
//    Added test for presence of avtGlobalZoneNumbers array, create error
//    message when not available but should be.
//
// ****************************************************************************

void
avtPickByZoneQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }
    int userZoneId = pickAtts.GetElementNumber();
    if (!pickAtts.GetElementIsGlobal())
    {
        if (dom != pickAtts.GetDomain()) 
            return;
    }
    else if (ds->GetCellData()->GetArray("avtGlobalZoneNumbers") == NULL)
    {
        pickAtts.SetDomain(-1);
        pickAtts.SetElementNumber(-1);
        pickAtts.SetErrorMessage("Pick could not be performed because a global "
                                 "zone id was specified for Pick but the mesh "
                                 "does not contain global zone information.");
        pickAtts.SetError(true);
        return; 
    }

    int zoneid = userZoneId;
    int maxEls = ds->GetNumberOfCells();
    if (pickAtts.GetMatSelected() &&  !pickAtts.GetElementIsGlobal())
    {
        //
        // The zone id stored in ElementNumber will not be correct relative
        // to this dataset.  Retrieve the correct one for use with 
        // RetrieveVarInfo, then reset it.
        //
        zoneid = GetCurrentZoneForOriginal(ds, userZoneId);
    }

    if (!pickAtts.GetElementIsGlobal() && (zoneid < 0 || zoneid >= maxEls))
    {
        EXCEPTION2(BadCellException, userZoneId+cellOrigin, maxEls+cellOrigin);
    }

    bool DBsuppliedZoneId = true;
    if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
    {
        if (pickAtts.GetElementIsGlobal())
        {
            userZoneId = vtkVisItUtility::GetLocalElementForGlobal(
                         ds, userZoneId, true);
            if (userZoneId == -1)
                return;
            zoneid = userZoneId;
            DBsuppliedZoneId = false;
        }
        GetZoneCoords(ds, zoneid);
        if (RetrieveNodes(ds, zoneid))
        {
            RetrieveVarInfo(ds);
            pickAtts.SetFulfilled(true);
        }
        else
        {
            // the incident nodes could not be found, no further processing 
            // required.  SetDomain and ElementNumber to -1 to indicate failure. 
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            pickAtts.SetErrorMessage("Pick encountered an internal "
                "error (could not find incident nodes).\n"
                "Please contact a VisIt developer"); 
            pickAtts.SetError(true);
            return; 
        }
    }
    if (pickAtts.GetElementIsGlobal())
        pickAtts.SetDomain(dom); 

    src->Query(&pickAtts);

    if (!pickAtts.GetFulfilled())
        return;

    if (pickAtts.GetElementIsGlobal() && DBsuppliedZoneId)
    {
       zoneid =  GetCurrentZoneForOriginal(ds, pickAtts.GetElementNumber());
       userZoneId = zoneid;
    }

    pickAtts.SetElementNumber(userZoneId+cellOrigin);

    if (pickAtts.GetMatSelected())
    {
        RetrieveVarInfo(ds, zoneid, pickAtts.GetIncidentElements());
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
    // Use the cell center as the place to position the pick letter.
    //
    float center[3];
    vtkVisItUtility::GetCellCenter(ds->GetCell(zoneid), center);
    pickAtts.SetCellPoint(center); 
    //
    // If the points of this dataset have been transformed, and we know the
    // transform matrix, transform the center point that will be used to place 
    // the Pick letter. 
    //
    if (transform != NULL)
    {
        avtVector v1(center);
        v1 = (*transform) * v1;
        center[0] = v1.x; 
        center[1] = v1.y; 
        center[2] = v1.z; 
        pickAtts.SetPickPoint(center);
    }
    else if (pickAtts.GetNeedTransformMessage())
    {
        //
        // Points were transformed but we don't need the message because we 
        // are displaying the real zone center coords in pick output. 
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
            // determining zone location.
            //
            pickAtts.SetRealElementNumber(zoneid);
        }
    }
    else 
    {
        pickAtts.SetPickPoint(center);
    }
}


// ****************************************************************************
//  Method: avtPickByZoneQuery::Preparation
//
//  Purpose:
//    Allows this pick to modify pickAtts before filters are applied.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 17:24:27 PST 2004
//    Set domain to -1 (so that no domain-restriction is performed) if
//    the zone is global.
//
// ****************************************************************************

void
avtPickByZoneQuery::Preparation()
{
    if (!pickAtts.GetElementIsGlobal())
    {
        int dom = pickAtts.GetDomain() - blockOrigin;
        pickAtts.SetDomain(dom < 0 ? 0 : dom);
        int  zone = pickAtts.GetElementNumber() - cellOrigin;
        pickAtts.SetElementNumber(zone < 0 ? 0 : zone);
    }
    else 
    {
        pickAtts.SetDomain(-1);
    }
}


// ****************************************************************************
//  Method: avtPickByZoneQuery::SetTransform
//
//  Purpose:
//    Sets the transform needed by this pick.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPickByZoneQuery::SetTransform(const avtMatrix *m)
{
    transform = m;
}

