/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                           avtPickByZoneQuery.C                            //
// ************************************************************************* //

#include <avtPickByZoneQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkIntArray.h>
#include <vtkVisItUtility.h>
#include <PickVarInfo.h>

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
//    Kathleen Bonnell, Wed May 11 17:14:03 PDT 200 
//    Convert varnmames to Global names when the DB supplied the ids. 
//
//    Kathleen Bonnell, Tue Jan 30 16:25:23 PST 2007 
//    Send zoneid to RetrieveVarInfo.
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
            RetrieveVarInfo(ds, zoneid);
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
       ConvertElNamesToGlobal();
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
    double center[3];
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
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDatAttributes arg.
//
// ****************************************************************************

void
avtPickByZoneQuery::Preparation(const avtDataAttributes &)
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

