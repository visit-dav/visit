// ************************************************************************* //
//                              avtZonePickQuery.C                           //
// ************************************************************************* //

#include <avtZonePickQuery.h>

#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkVisItUtility.h>

#include <avtMatrix.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtZonePickQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 20, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtZonePickQuery::avtZonePickQuery()
{
}


// ****************************************************************************
//  Method: avtZonePickQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 20, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtZonePickQuery::~avtZonePickQuery()
{
}


// ****************************************************************************
//  Method: avtZonePickQuery::SetInvTransform
//
//  Purpose:
//      Sets the inverseTransform, used to return correct pick points. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 20, 2004 
//
// ****************************************************************************

void
avtZonePickQuery::SetInvTransform(const avtMatrix *m)
{
    transform =  m;
}



// ****************************************************************************
//  Method: avtZonePickQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 20, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtZonePickQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != pickAtts.GetDomain() || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    int pickedZone = pickAtts.GetElementNumber();
    int type = ds->GetDataObjectType();
    bool needRealId = (ghostType == AVT_HAS_GHOSTS || pickedZone == -1) &&
            (type == VTK_STRUCTURED_GRID || type == VTK_RECTILINEAR_GRID || 
             ds->GetFieldData()->GetArray("vtkOriginalDimensions") != NULL );

    if (pickedZone == -1)
    {
        //
        //  If a valid zone hasn't already been determined, locate it now.
        //  
        float *cellPoint  = pickAtts.GetCellPoint();
        pickedZone = vtkVisItUtility::FindCell(ds, cellPoint);

        //
        //  If a valid zone still hasn't been found, there's some kind of 
        //  problem, it should have been found for this domain. 
        //  
        if (pickedZone == -1)
        {
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            debug5 << "PICK BIG PROBLEM!  "
                   << "Could not find zone corresponding to pick point" << endl;
            pickAtts.SetErrorMessage("Pick encountered an internal "
                "error (could not find zone corresponding to pick point).\n"
                "Please contact a VisIt developer"); 
            pickAtts.SetError(true);
            return;
        }
        pickAtts.SetElementNumber(pickedZone);
    }


    if (!pickAtts.GetMatSelected())
    {
        bool success = false;
        GetZoneCoords(ds, pickedZone);
        success = RetrieveNodes(ds, pickedZone);
        if (success)
        {
            pickAtts.SetElementNumber(pickedZone);
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
                    "error (could not find incident elements).\n"
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
        SetRealIds(ds);

    //
    //  Allow the database to add any missing information.
    // 
    src->Query(&pickAtts);

    if (pickAtts.GetMatSelected())
    {
        RetrieveVarInfo(ds); 
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
        // Put the real ids in the correct spot for output.
        //
        pickAtts.SetElementNumber(pickAtts.GetRealElementNumber());
        pickAtts.SetIncidentElements(pickAtts.GetRealIncidentElements());
    }
    pickAtts.SetElementNumber(pickAtts.GetElementNumber() + cellOrigin);
    //
    // If the points of this dataset have been transformed, and we know 
    // the inverse transform matrix, transform the pick point that will 
    // be displayed in the pick info window.
    //
    if (transform != NULL)
    {
        avtVector v1(pickAtts.GetPickPoint());
        v1 = (*transform) * v1;
        // 
        // PickPoint is used for placing the pick letter, so set
        // this tranformed point in CellPoint instead.
        // 
        float ppt[3] = { v1.x, v1.y, v1.z };
        pickAtts.SetCellPoint(ppt);
    }
    else 
    {
        // 
        // CellPoint may have been 'fudged' towards the cell center by
        // avtLocateCellQuery.  Make sure the point that will be displayed
        // is not that 'fudged' point.
        // 
        pickAtts.SetCellPoint(pickAtts.GetPickPoint());
    }
}



// ****************************************************************************
//  Method: avtZonePickQuery::Preparation
//
//  Purpose:
//    Allows this pick to modify pickAtts before filters are applied.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

void
avtZonePickQuery::Preparation()
{
    //
    // Transform the point that will be used in locating the cell. 
    //
    float *cellPoint  = pickAtts.GetCellPoint();
    if (transform != NULL)
    {
        //
        // Transform the intersection point back to original space.
        //
        avtVector v1(cellPoint);
        v1 = (*transform) * v1;
        cellPoint[0] = v1.x;
        cellPoint[1] = v1.y;
        cellPoint[2] = v1.z;
        //
        // Reset the cell point to the transformed point.
        //
        pickAtts.SetCellPoint(cellPoint);
    }
}
