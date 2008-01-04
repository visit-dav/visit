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
//                               avtBehavior.C                               //
// ************************************************************************* //

#include <avtBehavior.h>

#include <vtkRenderer.h>

#include <avtLegend.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtBehavior constructor
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Apr  3 14:58:59 PDT 2001
//    Added initialization of renderOrder.
//
//    Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//    Added initialization of antialiasedRenderOrder.
// 
// ****************************************************************************

avtBehavior::avtBehavior()
{
    legend      = NULL;
    renderer    = NULL;
    shiftFactor = 0;
    renderOrder = DOES_NOT_MATTER;
    antialiasedRenderOrder = DOES_NOT_MATTER;
}


// ****************************************************************************
//  Method: avtBehavior destructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//      
// ****************************************************************************

avtBehavior::~avtBehavior()
{
    if (renderer != NULL)
    {
        Remove(renderer);
        renderer = NULL;
    }
}
    

// ****************************************************************************
//  Method: avtBehavior::Add
//
//  Purpose:
//      Adds the behavior part of the actor (ie the legend) to a renderer.
//
//  Arguments:
//      ren        A renderer to add to.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
// ****************************************************************************

void
avtBehavior::Add(vtkRenderer *ren)
{
    if (*legend != NULL)
    {
        legend->Add(ren);
        renderer = ren;
    }
}


// ****************************************************************************
//  Method: avtBehavior::Remove
//
//  Purpose:
//      Removes the behavior part of the actor (ie the legend) from a renderer.
//
//  Arguments:
//      ren        A renderer to remove from.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
// ****************************************************************************

void
avtBehavior::Remove(vtkRenderer *ren)
{
    if (*legend != NULL)
    {
        if (renderer != ren)
        {
            EXCEPTION0(ImproperUseException);
        }

        legend->Remove();
        renderer = NULL;
    }
}


// ****************************************************************************
//  Method: avtBehavior::GetOriginalBounds
//
//  Purpose:
//      Gets the original bounds of the actor.
//
//  Arguments:
//      b       A location to put the bounds.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jan  5 11:10:17 PST 2001
//    Using current bounds instead of original bounds.
//
//    Hank Childs, Sun Mar 25 12:00:37 PST 2001
//    Correct error in setting bounds.
//
//    Hank Childs, Thu Aug 23 08:11:11 PDT 2001
//    Add a little more error checking.
//
//    Hank Childs, Tue Sep  4 15:34:56 PDT 2001
//    Reflect changes in avtDataAttributes.
//
//    Hank Childs, Mon Jul 15 09:07:25 PDT 2002
//    Renamed to GetOriginalBounds.
//
// ****************************************************************************

void
avtBehavior::GetOriginalBounds(double b[6])
{
    int  dim = info.GetAttributes().GetSpatialDimension();

    double extents[6];
    bool gotExtents = info.GetAttributes().GetSpatialExtents(extents);

    if (!gotExtents)
    {
        extents[0] = extents[2] = extents[4] = 0.;
        extents[1] = extents[3] = extents[5] = 1.;
    }

    for (int i = 0 ; i < 3 ; i++)
    {
        if (i < dim)
        {
            b[2*i  ] = extents[2*i];
            b[2*i+1] = extents[2*i+1];
        }
        else
        {
            b[2*i  ] = 0.;
            b[2*i+1] = 0.;
        }
    }
}


// ****************************************************************************
//  Method: avtBehavior::GetActualBounds
//
//  Purpose:
//      Gets the actual bounds of the actor.
//
//  Arguments:
//      b       A location to put the bounds.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2002
//
// ****************************************************************************

void
avtBehavior::GetActualBounds(double b[6])
{
    int  dim = info.GetAttributes().GetSpatialDimension();

    double extents[6];
    bool gotExtents = info.GetAttributes().GetCurrentSpatialExtents(extents);

    if (!gotExtents)
    {
        extents[0] = extents[2] = extents[4] = 0.;
        extents[1] = extents[3] = extents[5] = 1.;
    }

    for (int i = 0 ; i < 3 ; i++)
    {
        if (i < dim)
        {
            b[2*i  ] = extents[2*i];
            b[2*i+1] = extents[2*i+1];
        }
        else
        {
            b[2*i  ] = 0.;
            b[2*i+1] = 0.;
        }
    }
}


// ****************************************************************************
//  Method: avtBehavior::SetInfo
//
//  Purpose:
//      Sets the data object information for the behavior.
//
//  Arguments:
//      i         The information.
//
//  Programmer:   Hank Childs
//  Creation:     December 22, 2000
//
// ****************************************************************************

void
avtBehavior::SetInfo(const avtDataObjectInformation &i)
{
    info.Copy(i);
}


// ****************************************************************************
//  Method: avtBehavior::GetDimension
//
//  Purpose:
//      Gets the dimension of the actor.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 15:34:56 PDT 2001
//    Reflect changes in avtDataAttributes.
//  
// ****************************************************************************

int
avtBehavior::GetDimension(void)
{
    return info.GetAttributes().GetSpatialDimension();
}


// ****************************************************************************
//  Method: avtBehavior::GetWindowMode
//
//  Purpose:
//      Gets the window mode of the actor.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2003
//
// ****************************************************************************

WINDOW_MODE
avtBehavior::GetWindowMode(void)
{
    return info.GetAttributes().GetWindowMode();
}


// ****************************************************************************
//  Method: avtBehavior::AdaptsToAnyWindowMode
//
//  Purpose:
//      Determines if this actor can adapt to any window mode.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2007
//
// ****************************************************************************

bool
avtBehavior::AdaptsToAnyWindowMode(void)
{
    return info.GetAttributes().GetAdaptsToAnyWindowMode();
}


// ****************************************************************************
//  Method: avtBehavior::SetLegend
//
//  Purpose:
//      Sets the legend for the behavior.
//
//  Arguments:
//      l        The legend for the behavior.
//
//  Programmer:  Hank Childs
//  Creation:    December 22, 2000
//
// ****************************************************************************

void
avtBehavior::SetLegend(avtLegend_p l)
{
    legend = l;
}


// ****************************************************************************
//  Method: avtBehavior::SetShiftFactor
//
//  Purpose:
//      Sets the shift factor so that plots can be shifted towards the screen.
//
//  Arguments:
//      sf      The new shift factor.  This should be between 0 and 1.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
// ****************************************************************************

void
avtBehavior::SetShiftFactor(double sf)
{
    if (sf < 0.)
    {
        debug1 << "Bad shift factor given (" << sf << "), correcting to 0."
               << endl;
        sf = 0.;
    }

    if (sf > 1.)
    {
        debug1 << "Bad shift factor given (" << sf << "), correcting to 1."
               << endl;
        sf = 1.;
    }

    shiftFactor = sf;
}


// ****************************************************************************
//  Method: avtBehavior::GetShiftFactor
//
//  Purpose:
//      Returns the shift factor for a plot.  The shift factor is whatever
//      is appropriate for the plot type, provided that its topological
//      dimension is 1.  Otherwise the shift factor is 0.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 15:34:56 PDT 2001
//    Reflect changes in avtDataAttributes.
//  
//    Kathleen Bonnell, Mon Feb  2 13:15:54 PST 2004 
//    In 2D, allow plots with topological dimension of 0 to utilize their
//    own  shift factor.  (So vector plots won't be obscured by mesh plots).
//  
// ****************************************************************************

double
avtBehavior::GetShiftFactor(void)
{
    double rv = 0.;
    if (info.GetAttributes().GetTopologicalDimension() == 1)
    {
        rv = shiftFactor;
    }
    else if (info.GetAttributes().GetTopologicalDimension() == 0 &&
             info.GetAttributes().GetSpatialDimension() == 2)
    {
        rv = shiftFactor;
    }
    else
    {
        rv = 0.;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtBehavior::SetRenderOrder
//
//  Purpose:
//      Sets the render order so that plots can be rendered in proper order.
//
//  Arguments:
//      ro      The new render order.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 3, 2001
//
// ****************************************************************************

void
avtBehavior::SetRenderOrder(RenderOrder ro)
{
    renderOrder = ro;
}


// ****************************************************************************
//  Method: avtBehavior::SetAntialiasedRenderOrder
//
//  Purpose:
//      Sets the render order so that antialiased plots can be rendered in 
//      proper order.
//
//  Arguments:
//      ro      The new render order for antialiased mode.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 29, 2003 
//
// ****************************************************************************

void
avtBehavior::SetAntialiasedRenderOrder(RenderOrder ro)
{
    antialiasedRenderOrder = ro;
}


// ****************************************************************************
//  Method: avtBehavior::SetRenderOrder
//
//  Purpose:
//      Sets the render order so that plots can be rendered in proper order.
//
//  Arguments:
//      ro      The new render order.  This should be between 0 and 3.
//              (MUST_GO_FIRST, ABSOLUTELY_LAST)
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 3, 2001
//
// ****************************************************************************

void
avtBehavior::SetRenderOrder(int ro)
{
    if (ro < 0)
    {
        debug1 << "Bad render order given (" << ro << "), correcting to 0"
               << " (" << RenderOrderName(0) << ")." << endl;
        ro = 0;
    }

    if (ro >= MAX_ORDER)
    {
        debug1 << "Bad render order given (" << ro << "), correcting to "
               << MAX_ORDER-1 << " (" << RenderOrderName(MAX_ORDER-1)
               << ")."  << endl;
        ro = MAX_ORDER-1;
    }

    renderOrder = Int2RenderOrder(ro);
}


// ****************************************************************************
//  Method: avtBehavior::SetAntiAliasedRenderOrder
//
//  Purpose:
//      Sets the render order so that antialiased plots can be rendered in 
//      proper order.
//
//  Arguments:
//      ro      The new render order.  This should be between 0 and 3.
//              (MUST_GO_FIRST, ABSOLTELY_LAST)
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 29, 2003 
//
// ****************************************************************************
void
avtBehavior::SetAntialiasedRenderOrder(int ro)
{
    if (ro < 0)
    {
        debug1 << "Bad render order given (" << ro << "), correcting to 0"
               << " (" << RenderOrderName(0) << ")." << endl;
        ro = 0;
    }

    if (ro >= MAX_ORDER)
    {
        debug1 << "Bad render order given (" << ro << "), correcting to "
               << MAX_ORDER-1 << " (" << RenderOrderName(MAX_ORDER-1)
               << ")."  << endl;
        ro = MAX_ORDER-1;
    }

    antialiasedRenderOrder = Int2RenderOrder(ro);
}


// ****************************************************************************
//  Method: avtBehavior::GetRenderOrder
//
//  Purpose:
//      Returns the render order for a plot.  The render order is whatever
//      is appropriate for the plot type.
//
//  Arguments:
//     antialiased  Indicates whether to return antialisedRenderOrder or
//                  regular renderOrder. 
//
//  Returns:
//    renderOrder if passed arg is false, antialiasedRenderOrder otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 3, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//    Added bool arg indicating which renderorder to return. Changed return
//    based on passed arg.
// 
// ****************************************************************************

int
avtBehavior::GetRenderOrder(bool antialiased)
{
    if (!antialiased)
        return RenderOrder2Int(renderOrder);
    else 
        return RenderOrder2Int(antialiasedRenderOrder);
}


// ****************************************************************************
//  Method: avtBehavior::RequiresReExecuteForQuery
//
//  Purpose:
//    Returns whether or not Re-execution of the pipeline is required
//    in order to perform a query. 
//
//  Returns:
//    True if points were transformed && the original cells array has not
//    been created, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 26, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 25 11:18:43 PST 2003
//    Renamed from "GetTransformedPoints" to more accurately reflect the 
//    purpose.  
//
//    Kathleen Bonnell, Thu Apr 10 11:00:52 PDT 2003   
//    Added test for presence of Transform.  
//    
//    Kathleen Bonnell, Tue Jun  1 17:12:34 PDT 2004 
//    The return value is now a function of whether or not points were 
//    transformed, and the presence of either the needed transform or
//    Original Nodes/Zones array. 
//    
//    Kathleen Bonnell, Tue Oct 12 15:58:56 PDT 2004 
//    Handle Vector Plots and Point meshes differently -- if the Node or Zone 
//    arrays weren't kept around, even if the data atts say we have them 
//    Condense filter will have removed them. 
//    
//    Hank Childs, Thu Mar  3 08:59:29 PST 2005
//    If we are doing DLB, then we need to re-execute.
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added more conditions -- CanUseOrigZones and OrigNodesRequiredForPick.
//
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005 
//    Removed 'needNodes' from topodim == 0 portion of test. 
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed OrigNodesRequiredForPick to OrigElementsRequiredForPick, and
//    check for presence of original nodes if !needZones.
//
//    Hank Childs, Sat Aug  4 13:05:51 PDT 2007
//    Beef up test when we need original elements for pick and the points
//    have been transformed.
//
// ****************************************************************************

bool
avtBehavior::RequiresReExecuteForQuery(const bool needInvT, 
                                       const bool needZones)
{
    bool retval = false;
    if (GetInfo().GetValidity().GetIsThisDynamic())
        retval = true;
    else if (info.GetAttributes().GetTopologicalDimension() == 0) 
    {
        // 
        // Handle things differently for Vector plots and point meshes.
        // 
        bool zonesAvailable = info.GetAttributes().GetContainsOriginalCells();
        bool nodesAvailable = info.GetAttributes().GetContainsOriginalNodes();
        bool keptNodeZone = info.GetAttributes().GetKeepNodeZoneArrays();
        bool canUseZones = info.GetAttributes().CanUseOrigZones();

        if (needZones)
        {
            retval = (!keptNodeZone || (canUseZones && !zonesAvailable));
        }
        else
        {
            retval = (!keptNodeZone || !nodesAvailable);
        }
    }
    else if (info.GetValidity().GetPointsWereTransformed())
    {
        bool invXformAvailable  = info.GetAttributes().HasInvTransform() &&
                              info.GetAttributes().GetCanUseInvTransform();

        bool xformAvailable  = info.GetAttributes().HasTransform() &&
                           info.GetAttributes().GetCanUseTransform();

        bool zonesAvailable = info.GetAttributes().GetContainsOriginalCells();
        bool nodesAvailable = info.GetAttributes().GetContainsOriginalNodes();
        bool canUseZones = info.GetAttributes().CanUseOrigZones();

        if (needInvT && needZones)
        {
            retval = !invXformAvailable && canUseZones && !zonesAvailable;
        }
        else if (needInvT && !needZones) 
        {
            retval = !invXformAvailable && !nodesAvailable;
        }
        else if (!needInvT && needZones)
        {
            retval = !xformAvailable && canUseZones && !zonesAvailable;
        }
        else  //  if (!needInvT && !needZones) 
        {
            retval = !xformAvailable && !nodesAvailable;
        }

        if (info.GetAttributes().OrigElementsRequiredForPick())
        {
            if (needZones)
                retval = retval || (info.GetAttributes().CanUseOrigZones() &&
                         !info.GetAttributes().GetContainsOriginalCells());
            else 
                retval = retval ||
                         (!info.GetAttributes().GetContainsOriginalNodes());
        }
    }
    else if (info.GetAttributes().OrigElementsRequiredForPick())
    {
        if (needZones)
            retval = info.GetAttributes().CanUseOrigZones() &&
                     !info.GetAttributes().GetContainsOriginalCells();
        else 
            retval = !info.GetAttributes().GetContainsOriginalNodes();
    }
    return retval;
}


// ****************************************************************************
//  Method: avtBehavior::GetDataExtents
//
//  Purpose:
//      Gets the data extents of the actor.
//
//  Arguments:
//    dmin      A location to store the minimum value. 
//    dmax      A location to store the maximum value. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 22 16:36:29 PST 2004
//    Removed exception for variable dimension != 1, as all var extents now
//    contain two elements, regardless of dimension.
//
// ****************************************************************************

void
avtBehavior::GetDataExtents(double &dmin, double &dmax)
{
    double extents[2];
    bool gotExtents = info.GetAttributes().GetDataExtents(extents);

    if (!gotExtents)
    {
        extents[0] = extents[1] = 0.;
    }

    dmin = extents[0];
    dmax = extents[1];
}


// ****************************************************************************
//  Method: avtBehavior::GetPlotInfoAtts
//
//  Purpose:
//
//  Returns:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 20, 2006 
//
//  Modifications:
//
// ****************************************************************************

const PlotInfoAttributes *
avtBehavior::GetPlotInfoAtts() 
{
    return info.GetAttributes().GetPlotInfoAtts();
}
