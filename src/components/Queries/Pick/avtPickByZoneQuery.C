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
//
// ****************************************************************************

void
avtPickByZoneQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != pickAtts.GetDomain() || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    int zoneNumber = pickAtts.GetElementNumber();
    int type = ds->GetDataObjectType();
    bool needRealId = ghostType == AVT_HAS_GHOSTS  &&
            (type == VTK_STRUCTURED_GRID || type == VTK_RECTILINEAR_GRID || 
             ds->GetFieldData()->GetArray("vtkOriginalDimensions") != NULL );


    int maxEls = ds->GetNumberOfCells();
    if (zoneNumber < 0 || zoneNumber >= maxEls)
    {
        EXCEPTION2(BadCellException, zoneNumber+cellOrigin, maxEls+cellOrigin);
    }

    if (!pickAtts.GetMatSelected() && ghostType != AVT_CREATED_GHOSTS)
    {
        if (needRealId)
        {
            int dims[3], ijk[3] = {0, 0, 0};
            vtkVisItUtility::GetDimensions(ds, dims);
            vtkVisItUtility::GetLogicalIndices(ds, true, zoneNumber, ijk, 
                                               false, false);
            vtkIntArray *realDims =
                (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
            if (realDims != NULL)
            {
                ijk[0] += realDims->GetValue(0);
                ijk[1] += realDims->GetValue(2);
                ijk[2] += realDims->GetValue(4);
            }
            zoneNumber = ijk[0] +
                         ijk[1] * (dims[0]-1) +
                         ijk[2] * (dims[0]-1) * (dims[1]-1);
            pickAtts.SetElementNumber(zoneNumber);
        }
        GetZoneCoords(ds, zoneNumber);
        if (RetrieveNodes(ds, zoneNumber))
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

    if (pickAtts.GetMatSelected())
        RetrieveVarInfo(ds);

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
        // Put the real ids in the correct spot for output.

        pickAtts.SetElementNumber(pickAtts.GetRealElementNumber());
        pickAtts.SetIncidentElements(pickAtts.GetRealIncidentElements());
    }
    pickAtts.SetElementNumber(pickAtts.GetElementNumber() + cellOrigin);

    //
    // Use the cell center as the place to position the pick letter.
    //
    vtkCell *cell = ds->GetCell(zoneNumber);
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
            pickAtts.SetRealElementNumber(zoneNumber);
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

