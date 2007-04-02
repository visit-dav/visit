/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtZonePickQuery.C                           //
// ************************************************************************* //

#include <avtZonePickQuery.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
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
//    Kathleen Bonnell, Tue Aug 10 09:12:54 PDT 2004
//    When material selection has been applied, ensure that RetrieveVarInfo
//    will be using the correct zone id for this dataset.
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
//    Kathleen Bonnell, Tue Jun 28 10:57:39 PDT 2005 
//    Readded 'needRealId' test, modified so that Real ids are calculated only
//    under specific conditions:  we don't already know the picked Node, ghosts
//    were Created, we have the Ghosts array and the data is structured.  
//    E.g. when we are picking on the Contour plot of AMR data.
//
//    Kathleen Bonnell, Fri Jul  8 14:15:21 PDT 2005 
//    Modified test for determining if 'real' id needs to be calculated. 
//
// ****************************************************************************

void
avtZonePickQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL || pickAtts.GetFulfilled())
    {
            return;
    }
    if (dom != pickAtts.GetDomain() && !skippedLocate)
    {
        return;
    }

    int pickedZone = pickAtts.GetElementNumber();
    int type = ds->GetDataObjectType();

    //
    // We may need the real Id when we are picking on a Contour of an
    // AMR mesh. 
    //
    bool needRealId = false;
    if (pickedZone == -1 && ghostType == AVT_CREATED_GHOSTS &&
        (type == VTK_STRUCTURED_GRID || type == VTK_RECTILINEAR_GRID))
    {
        if (pickAtts.GetHasMixedGhostTypes() == -1)
        {
            needRealId = vtkVisItUtility::ContainsMixedGhostZoneTypes(ds);
        }
        else 
        {
            needRealId = (pickAtts.GetHasMixedGhostTypes() == 1);
        }
    }

    if (pickedZone == -1)
    {
        //
        //  If a valid zone hasn't already been determined, locate it now.
        //  
        double *cellPoint  = pickAtts.GetCellPoint();
        pickedZone = vtkVisItUtility::FindCell(ds, cellPoint);

        if (pickedZone == -1)
        {
            if (pickAtts.GetDomain() != -1)
            {
                //
                //  If a valid zone still hasn't been found, and we were
                //  expecting one for this domain, there's some kind of 
                //  problem. 
                //  
                pickAtts.SetDomain(-1);
                pickAtts.SetElementNumber(-1);
                debug5 << "PICK BIG PROBLEM!  Could not find zone"
                       << "corresponding to pick point" << endl;
                pickAtts.SetErrorMessage("Pick encountered an internal error"
                    " (could not find zone corresponding to pick point).\n"
                    "Please contact a VisIt developer"); 
                pickAtts.SetError(true);
            }
            return;
        }

        vtkDataArray *ghost = ds->GetCellData()->GetArray("avtGhostZones");
        if (ghost && ghost->GetTuple1(pickedZone) > 0)
            return;

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
    //  The database needs a valid domain
    // 
    if (pickAtts.GetDomain() == -1)
        pickAtts.SetDomain(dom);
    
    if (needRealId && pickAtts.GetMatSelected())
    {
        SetRealIds(ds);
        pickAtts.SetElementNumber(pickAtts.GetRealElementNumber());
    }

    //
    //  Allow the database to add any missing information.
    // 
    src->Query(&pickAtts);

    if (pickAtts.GetMatSelected())
    {
        //
        // The zone id stored in ElementNumber will not be correct relative
        // to this dataset.  Retrieve the correct one for use with 
        // RetrieveVarInfo, then reset it.
        //
        int currentZone = GetCurrentZoneForOriginal(ds, 
                            pickAtts.GetElementNumber());
        RetrieveVarInfo(ds, currentZone, pickAtts.GetIncidentElements());
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

    if (needRealId && !pickAtts.GetMatSelected())
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
        double ppt[3] = { v1.x, v1.y, v1.z };
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
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDatAttributes arg.
//
// ****************************************************************************

void
avtZonePickQuery::Preparation(const avtDataAttributes &)
{
    //
    // Transform the point that will be used in locating the cell. 
    //
    double *cellPoint  = pickAtts.GetCellPoint();
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

