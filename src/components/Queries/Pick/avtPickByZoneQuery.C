// ************************************************************************* //
//                           avtPickByZoneQuery.C                            //
// ************************************************************************* //

#include <avtPickByZoneQuery.h>

#include <vtkCell.h>
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
// ****************************************************************************

void
avtPickByZoneQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != pickAtts.GetDomain() || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    int userZoneId = pickAtts.GetElementNumber();
    int zoneid = userZoneId;
    int maxEls = ds->GetNumberOfCells();
    if (pickAtts.GetMatSelected())
    {
        //
        // The zone id stored in ElementNumber will not be correct relative
        // to this dataset.  Retrieve the correct one for use with 
        // RetrieveVarInfo, then reset it.
        //
        zoneid = GetCurrentZoneForOriginal(ds, userZoneId);
    }

    if (zoneid < 0 || zoneid >= maxEls)
    {
        EXCEPTION2(BadCellException, userZoneId+cellOrigin, maxEls+cellOrigin);
    }

    int type = ds->GetDataObjectType();
    bool needRealId = ghostType == AVT_HAS_GHOSTS  &&
            (type == VTK_STRUCTURED_GRID || type == VTK_RECTILINEAR_GRID || 
             ds->GetFieldData()->GetArray("vtkOriginalDimensions") != NULL );

    if (needRealId)
    {
        // Need to convert a zoneid that is Non-Ghost relative
        // to a zoneid that is ghost-relative.
        zoneid = vtkVisItUtility::ZoneGhostIdFromNonGhost(ds, zoneid);
        pickAtts.SetElementNumber(zoneid);
    }

    if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
    {
        GetZoneCoords(ds, zoneid);
        if (RetrieveNodes(ds, zoneid))
        {
            RetrieveVarInfo(ds);
            pickAtts.SetFulfilled(true);
        }
        else
        {
            // the indident nodes could not be found, no further processing 
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
    //
    // If a material-var is requested it may need the real ids -- if
    // ghost zones were not present when 
    // a material-var is requested.
    //
    if (needRealId && ghostType == AVT_CREATED_GHOSTS) 
    {
        SetRealIds(ds);
    }


    src->Query(&pickAtts);

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
    // The queryable source may have added more info, so call this again. 
    //
    if (needRealId)
    {
        SetRealIds(ds);
        //
        // Put the real node ids in the correct spot for output.
        pickAtts.SetIncidentElements(pickAtts.GetRealIncidentElements());
    }

    //
    // Use the cell center as the place to position the pick letter.
    //
    vtkCell *cell = ds->GetCell(zoneid);
    float parametricCenter[3], center[3];
    float weights[28];
    int subId = cell->GetParametricCenter(parametricCenter);
    cell->EvaluateLocation(subId, parametricCenter, center, weights);
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
//
// ****************************************************************************

void
avtPickByZoneQuery::Preparation()
{
    int dom = pickAtts.GetDomain() - blockOrigin;
    pickAtts.SetDomain(dom < 0 ? 0 : dom);
    int  zone = pickAtts.GetElementNumber() - cellOrigin;
    pickAtts.SetElementNumber(zone < 0 ? 0 : zone);
}


// ****************************************************************************
//  Method: avtPickByZoneQuery::Preparation
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

