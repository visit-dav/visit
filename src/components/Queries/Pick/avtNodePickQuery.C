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
//                           avtNodePickQuery.C                              //
// ************************************************************************* //

#include <avtNodePickQuery.h>

#include <float.h>

#include <vtkCellData.h>
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
//    Hank Childs, Thu Mar 10 10:35:37 PST 2005
//    Fix memory leak.
//
//    Kathleen Bonnell, Tue Jun 28 10:45:28 PDT 2005 
//    Calculate 'real' ids under the proper circumstances (Created ghosts,
//    the ghost array is available, we don't have the picked node right
//    away and the ds is structured). 
//
//    Kathleen Bonnell, Fri Jul  8 14:15:21 PDT 2005 
//    Changed test for determining if the 'real' id needs to be calculated. 
//    Also, transform the point to be used as 'pick letter' when this
//    pick did not use the locate query to determine that point.
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
    int type = ds->GetDataObjectType();
    //
    // We may need the real id when picking on a Contour plot of
    // an AMR dataset. 
    //
    bool needRealId = false;

    if (pickedNode == -1 && ghostType == AVT_CREATED_GHOSTS &&
        (type == VTK_STRUCTURED_GRID || type == VTK_RECTILINEAR_GRID))
    {
        needRealId = vtkVisItUtility::ContainsMixedGhostZoneTypes(ds);
    }

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
                   << "Could not find zone corresponding to pick point" 
                   << endl;
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

        double dist = vtkMath::Distance2BetweenPoints(pickAtts.GetPickPoint(), 
                          ds->GetPoint(pickedNode)); 
        if (dist < minDist)
        {
            minDist = dist;
            
        }
        else 
        {
            return;
        }
    }

    pickAtts.SetCellPoint(ds->GetPoint(pickedNode)); 

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
            // the zones could not be found, no further processing required.
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

    if (needRealId)
    {
        SetRealIds(ds);
        //
        // Put the real ids in the correct spot for output.
        //
        pickAtts.SetElementNumber(pickAtts.GetRealElementNumber());
        pickAtts.SetIncidentElements(pickAtts.GetRealIncidentElements());
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
        if (!skippedLocate)
        {
            pickAtts.SetPickPoint(pickAtts.GetNodePoint());
        }
        else 
        {
           avtVector v1(pickAtts.GetCellPoint());
           v1 = (*transform) *v1;
           double pp[3] = {v1.x, v1.y, v1.z};
            pickAtts.SetPickPoint(pp);
        }
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
        if (!needRealId)
            foundNode = pickedNode;
        else 
            foundNode = pickAtts.GetRealElementNumber();
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
//    Kathleen Bonnell, Tue Jun 28 10:47:35 PDT 2005 
//    Find intersected cell and test if it is a ghost before proceeding
//    further.  
//
// ****************************************************************************

int
avtNodePickQuery::DeterminePickedNode(vtkDataSet *ds)
{
    double *bnds = ds->GetBounds();
    double *pp = pickAtts.GetPickPoint();
#if 0

    if (pp[0] < bnds[0] || pp[0] > bnds[1] ||
        pp[1] < bnds[2] || pp[1] > bnds[3] ||
        pp[2] < bnds[4] || pp[2] > bnds[5])
    {
        return -1;
    }

#endif
    int zone = vtkVisItUtility::FindCell(ds, pp);

    if (zone == -1)
        return -1;

    vtkDataArray *ghostZones = ds->GetCellData()->GetArray("avtGhostZones");
    if (ghostZones && ghostZones->GetTuple1(zone) > 0)
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
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDatAttributes arg.
//
// ****************************************************************************

void
avtNodePickQuery::Preparation(const avtDataAttributes &)
{
    //
    // Transform the point that will be used in locating the node. 
    //
    double *pickPoint  = pickAtts.GetPickPoint();
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
