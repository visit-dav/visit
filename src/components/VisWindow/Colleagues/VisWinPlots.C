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
//                              VisWinPlots.C                                //
// ************************************************************************* //

#include <VisWinPlots.h>

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkDataObjectCollection.h>
#include <vtkFollower.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkVectorText.h>

#include <VisWindow.h>
#include <VisWinPathTracker.h>

#include <avtActor.h>
#include <avtExternallyRenderedImagesActor.h>
#include <avtLegend.h>
#include <avtTransparencyActor.h>
#include <avtTerminatingSource.h>

#include <BadPlotException.h>
#include <DebugStream.h>
#include <PlotDimensionalityException.h>

using std::string;
using std::vector;


// ****************************************************************************
//  Method: VisWinPlots constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Sep  4 14:07:15 PDT 2001
//    Made bboxGrid a vtkOutlineSource instead of a vtkRectilinearGrid,
//    use vtkPolyDataMapper instead of vtkDataSetMapper.
//
//    Eric Brugger, Mon Oct 22 09:38:54 PDT 2001
//    Initialize bboxMode.
//
//    Kathleen Bonnell, Wed Nov 28 08:25:55 PST 2001 
//    Make the bounding box un-pickable. Initialize pickScaleFactor. 
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002  
//    Remove pickScaleFactor.
//
//    Hank Childs, Sun Jul  7 12:50:43 PDT 2002
//    Initialized new data member, transparencyActor.
//
//    Kathleen Bonnell, Thu Oct  3 14:41:19 PDT 2002  
//    Disable lighting on the bounding box, by setting its ambient/diffuse
//    coefficients.
//
//    Mark C. MIller, Thu Dec 19 11:38:05 PST 2002 
//    Initialized new data member, extRenderedImagesActor
//
// ****************************************************************************

VisWinPlots::VisWinPlots(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    bboxGrid = vtkOutlineSource::New();
    bboxMapper = vtkPolyDataMapper::New();
    bboxMapper->SetInput(bboxGrid->GetOutput());

    bbox = vtkActor::New();
    bbox->SetMapper(bboxMapper);
    bbox->VisibilityOff();
    bbox->PickableOff();
    bbox->GetProperty()->SetAmbient(1.);
    bbox->GetProperty()->SetDiffuse(0.);
    bboxMode = false;

    spatialExtentType = AVT_ORIGINAL_EXTENTS;
    userSetBounds = false;
    for (int i = 0 ; i < 6 ; i++)
    {
        currentBounds[i] = 0.;
        setBounds[i]     = 0.;
    }

    sceneHasChanged = false;

    transparencyActor = new avtTransparencyActor;
    extRenderedImagesActor = new avtExternallyRenderedImagesActor;
}


// ****************************************************************************
//  Method: VisWinPlots destructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Delete the transparency actor.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Delete the externally rendered images actor
//
//    Jeremy Meredith, Thu Jun 26 13:57:09 PDT 2003
//    Delete the plot actors (since they contain pointers to the
//    transparency actors) before delete the transparency actor itself.
//
// ****************************************************************************

VisWinPlots::~VisWinPlots()
{
    //
    // The plots reference the transparency actor, so we need to free those up
    // before the transparency actor is deleted.  If there are additional
    // references to these plots elsewhere, it may cause problems -- 
    // especially if we ever allow plots to be added to multiple windows.
    //
    plots.clear();

    if (bbox != NULL)
    {
        bbox->Delete();
        bbox = NULL;
    }
    if (bboxGrid != NULL)
    {
        bboxGrid->Delete();
        bboxGrid = NULL;
    }
    if (bboxMapper != NULL)
    {
        bboxMapper->Delete();
        bboxMapper = NULL;
    }
    if (transparencyActor != NULL)
    {
        delete transparencyActor;
        transparencyActor = NULL;
    }
    if (extRenderedImagesActor != NULL)
    {
        delete extRenderedImagesActor;
        extRenderedImagesActor = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinPlots::AddPlot
//
//  Purpose:
//      Adds an avtActor_p to the vis window.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 13:19:15 PDT 2000
//    Notified the VisWindow that it now has plots.
//
//    Hank Childs, Sun Aug  6 15:09:37 PDT 2000
//    Made sure scalar bar had up-to-date range by forcing update.
//
//    Kathleen Bonnell, Tue Aug  3 15:18:29 PDT 2001 
//    Added call to OrderPlots method. 
//
//    Eric Brugger, Mon Oct 22 09:38:54 PDT 2001
//    Added code to resize the bounding box and turn off the visibility of
//    the plot if in bounding box mode.
//
//    Kathleen Bonnell, Wed Nov 28 08:25:55 PST 2001 
//    Added call to SetPickScaleFactor. 
//
//    Hank Childs, Tue Mar 12 17:04:02 PST 2002
//    Add call to update the plot list.
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002  
//    Removed call to SetPickScaleFactor. 
//
//    Hank Childs, Sun Jul  7 12:50:43 PDT 2002
//    Add support for transparency.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Add lighting support.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Add support for externally rendered images
//
//    Kathleen Bonnell, Wed May 28 11:56:39 PDT 2003
//    Scale the actor if the window is in full frame mode.
//
//    Kathleen Bonnell, Wed May 28 12:48:30 PDT 2003  
//    When in antialiasing mode, allow colleagues to re-add themselves to the
//    window, so that they can be drawn after plots, and reduce artefacts. 
//
//    Kathleen Bonnell, Fri Jun  6 15:23:05 PDT 2003 
//    GetFullFrameMode from mediator before retrieving scale factor and type. 
//    
//    Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003 
//    Send the antialiasing flag to OrderPlots method.
//    
//    Jeremy Meredith, Fri Nov 14 17:56:24 PST 2003
//    Added specular properties.
//
//    Hank Childs, Mon May 10 08:42:32 PDT 2004
//    Get the immediate morde rendering info from a different source.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Elminated SetExternallyRenderedImagesActor
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Moved getting of anti-aliasing and call to ReAddColleaguesToRenderWindow
//    into OrderPlots along with
//
//    Brad Whitlock, Mon Sep 18 11:17:39 PDT 2006
//    Added color texturing flag.
//
//    Cyrus Harrison, Sun Jun 17 21:07:09 PDT 2007
//    Added tracking of active paths
//
//    Jeremy Meredith, Wed Jun 20 17:00:33 EDT 2007
//    Only use the window mode to set whether specular is enabled, not
//    to clear all the other specular properties that won't be used.
//    This turned out not to be the cause of a specular bug, but I
//    believe this behavior is more correct anyway.
//
//    Mark C. Miller, Thu Jun 21 00:12:28 PDT 2007
//    Added support to overlay curve plots on 2D plots.
//    Reset scale vec to all 1's if not in full frame mode.
// ****************************************************************************

void
VisWinPlots::AddPlot(avtActor_p &p)
{
    //
    // Check to make sure this is a valid plot to add.
    //
    CheckPlot(p);

    //
    // Tell the vis window that it now has plots.
    //
    mediator.HasPlots(true);

    //
    // We are going to need the bounds to know if we should reset the camera
    // when the new actor is added.
    //
    double oldbounds[6];
    GetBounds(oldbounds);

    p->Add(mediator.GetCanvas(), mediator.GetForeground());
    p->SetTransparencyActor(transparencyActor);

    double vec[3] = { 1. , 1., 1.};
    if (mediator.GetFullFrameMode())
    {
        double scale; 
        int type;
        mediator.GetScaleFactorAndType(scale, type);
        if (type == 0) // x-axis
            vec[0] = scale;
        else
            vec[1] = scale;
        p->ScaleByVector(vec);
    }
    else
    {
        p->ScaleByVector(vec);
    }

    // get db path and add to active paths
    avtDataAttributes &atts  = (*p)->GetBehavior()->GetInfo().GetAttributes();
    VisWinPathTracker::Instance()->AddPath(atts.GetFullDBName());

    plots.push_back(p);
    OrderPlots();

    double newbounds[6];
    GetBounds(newbounds);
    AdjustCamera(oldbounds, newbounds);

    avtLegend_p l = p->GetLegend();
    if (*l != NULL)
    {
        double  color[3];
        mediator.GetForegroundColor(color);
        l->SetForegroundColor(color);
    }

    //
    // If in bounding box mode, resize the bounding box and turn off the
    // visibility of the plot.
    //
    if (bboxMode)
    {
        double  bounds[6];
        GetRealBounds(bounds);
        SetBoundingBox(bounds);
        p->VisibilityOff();
    }

    //
    // Ensure the new plot has the correct lighting coefficients. 
    //
    if (mediator.GetLighting())
        p->TurnLightingOn();
    else
        p->TurnLightingOff();
    if (mediator.GetAmbientOn())
    {
        p->SetAmbientCoefficient(mediator.GetAmbientCoefficient());
    }

    //
    // Ensure that the new plot has the correct surface representation.
    //
    p->SetSurfaceRepresentation(mediator.GetSurfaceRepresentation());

    //
    // Ensure that the new plot has the right immediate mode rendering flag.
    //
    VisWindow *vw = mediator;
    p->SetImmediateModeRendering(vw->GetImmediateModeRendering());

    mediator.UpdatePlotList(plots);

    //
    // Set the right specular values
    //
    p->SetSpecularProperties((p->GetWindowMode() == WINMODE_3D) ?
                             mediator.GetSpecularFlag() : false,
                             mediator.GetSpecularCoeff(),
                             mediator.GetSpecularPower(),
                             mediator.GetSpecularColor());

    // Ensure that the new plot has the right color texturing flag.
    p->SetColorTexturingFlag(vw->GetColorTexturingFlag());

    //
    // Must do explicit render if we want the changes to show up.
    //
    mediator.Render();
}

// ****************************************************************************
// Method: VisWinPlots::TriggerPlotListUpdate
//
// Purpose: 
//   Triggers a plot list update among the colleagues.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 11 14:16:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinPlots::TriggerPlotListUpdate(void)
{
    mediator.UpdatePlotList(plots);
}

// ****************************************************************************
//  Method: VisWinPlots::CheckPlot
//
//  Purpose:
//      Checks to make sure the plot can be added safely.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Add support for curve mode.
//
//    Eric Brugger, Wed Aug 20 09:57:48 PDT 2003
//    Modify the routine to set the mode based on the plot type if no plots
//    were present and to check that the modes match otherwise.
//
//    Mark C. Miller, Thu Jun 21 00:12:28 PDT 2007
//    Added support to overlay curve plots on 2D plots.
//
//    Hank Childs, Fri Aug 31 10:20:04 PDT 2007
//    Add support for plots that adapt to any window mode.
//
// ****************************************************************************

void
VisWinPlots::CheckPlot(avtActor_p &p)
{
    if (plots.size() == 0)
    {
        //
        // The mode isn't set, so set it.
        //
        mediator.ChangeMode(p->GetWindowMode());
    }
    else
    {
        //
        // If the modes don't match then it is an error.
        //
        if (mediator.GetMode() == p->GetWindowMode())
	    return;

        if (p->AdaptsToAnyWindowMode())
            return;

        if (mediator.GetMode() == WINMODE_2D &&
	    p->GetWindowMode() == WINMODE_CURVE)
	    return;
       
        if (mediator.GetMode() == WINMODE_CURVE &&
	    p->GetWindowMode() == WINMODE_2D)
        {
            mediator.ChangeMode(WINMODE_2D);
	    return;
        }

        EXCEPTION3(PlotDimensionalityException, mediator.GetMode(), 
            p->GetWindowMode(), plots.size());
    }
}


// ****************************************************************************
//  Method: VisWinPlots::RemovePlot
//
//  Purpose:
//      Removes a plot from the window.
//
//  Arguments:
//      p       The plot to remove.
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2000
//
//  Modifications:
//    Eric Brugger, Mon Oct 22 09:38:54 PDT 2001
//    Added code to turn visibility on for the plot being removed if in
//    bounding box mode.
//
//    Hank Childs, Tue Mar 12 17:04:02 PST 2002
//    Add call to update the plot list.
//
//    Cyrus Harrison, Sun Jun 17 21:07:09 PDT 2007
//    Added tracking of active paths
//
// ****************************************************************************

void
VisWinPlots::RemovePlot(avtActor_p &p)
{
    //
    // Find the PlotActors to remove.
    //
    avtActor_p removeme;
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        avtActor_p p2 = *it;
        if (*p2 == *p)
        {
            removeme = *it;
            break;
        }
    }

    //
    // Make sure a valid plot was specified.
    //
    if (*removeme == NULL)
    {
        EXCEPTION0(BadPlotException);
    }

    //
    // If in bounding box mode turn on the visibility.
    //
    if (bboxMode)
    {
        (*it)->VisibilityOn();
    }

    // remove ref to db path and from active paths
    avtDataAttributes &atts = (*it)->GetBehavior()->GetInfo().GetAttributes();
    VisWinPathTracker::Instance()->RemovePath(atts.GetFullDBName());

    //
    // Remove the plot actors from the vector.
    //
    plots.erase(it);

    //
    // Remove the plot.
    //
    removeme->Remove(mediator.GetCanvas(), mediator.GetForeground());

    //
    // Check to see if the VisWindow still has plots and tell it so if it
    // doesn't.
    //
    if (plots.begin() == plots.end())
    {
        mediator.HasPlots(false);
    }

    mediator.UpdatePlotList(plots);

    //
    // Must do explicit render if we want the changes to show up.
    //
    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinPlots::ClearPlots
//
//  Purpose:
//      Removes all of the plots from the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//    Eric Brugger, Mon Oct 22 09:38:54 PDT 2001
//    Added code to turn visibility on for all the plots if in bounding
//    box mode.
//
// ****************************************************************************

void
VisWinPlots::ClearPlots(void)
{
    std::vector< avtActor_p >::iterator it;

    //
    // We need to suspend updates if they are not suspended already, so we
    // don't see each individual actor get removed.
    //
    bool  suspendedUpdates = false;
    if (mediator.UpdatesEnabled())
    {
        VisWindow *vw = mediator;
        vw->DisableUpdates();
        suspendedUpdates = true;
    }
    //
    // The vector we want to remove them from will be changing, so make a copy
    // of it.
    //
    std::vector< avtActor_p >  localPlots;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        localPlots.push_back(*it);
    }

    //
    // Iterate through our copy of the plot list, removing each one and
    // turning off the visibility if in bounding box mode.
    //
    for (it = localPlots.begin() ; it != localPlots.end() ; it++)
    {
        RemovePlot(*it);
        if (bboxMode)
        {
            (*it)->VisibilityOn();
        }
    }

    //
    // Enable updates if we suspended them earlier.
    //
    if (suspendedUpdates)
    {
        VisWindow *vw = mediator;
        vw->EnableUpdates();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetPlotListIndex
//
//  Purpose: If the vis window has a particular plot identified by name, return
//           that plot's index in the window's list of active plots.  If not,
//           return -1.
//
//  Arguments:
//      plotName : Name of the plot being queried
//
//  Programmer: Mark Blair
//  Creation:   Wed Aug 30 14:09:00 PDT 2006
//
// ****************************************************************************

int
VisWinPlots::GetPlotListIndex(const char *plotName)
{
    int plotIndex;

    for (plotIndex = 0; plotIndex < plots.size(); plotIndex++)
    {
        if (strcmp(plots[plotIndex]->GetTypeName(), plotName) == 0) break;
    }
    
    if (plotIndex >= plots.size()) plotIndex = -1;
    
    return plotIndex;
}


// ****************************************************************************
//  Method: VisWinPlots::GetPlotInfoAtts
//
//  Purpose: If plot identified by name is among the plots for this VisWindow,
//           returns pointer to any attributes that were pushed by that plot.
//           Returns NULL if no such plot exists or if no attributes were
//           pushed by the plot.
//
//  Arguments:
//      plotName : Name (type name) of the plot being queried
//
//  Programmer: Mark Blair
//  Creation:   Wed Oct 25 15:12:55 PDT 2006
//
// ****************************************************************************

const PlotInfoAttributes *
VisWinPlots::GetPlotInfoAtts(const char *plotName)
{
    int plotIndex;

    for (plotIndex = 0; plotIndex < plots.size(); plotIndex++)
    {
        if (strcmp(plots[plotIndex]->GetTypeName(), plotName) == 0) break;
    }
    
    if (plotIndex >= plots.size()) return NULL;
    
    return plots[plotIndex]->GetBehavior()->GetPlotInfoAtts();
}


// ****************************************************************************
//  Method: VisWinPlots::SetViewExtentsType
//
//  Purpose:
//      Set the flavor of spatial extents we should use to get the bounds
//      (and therefore set the view).
//
//  Arguments:
//      vt      The new view type.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2002
//
// ****************************************************************************

void
VisWinPlots::SetViewExtentsType(avtExtentType vt)
{
    spatialExtentType = vt;
}


// ****************************************************************************
//  Method: VisWinPlots::StartBoundingBox
//
//  Purpose:
//      Removes all of the actors from the canvas and adds a bounding box
//      in its place.
//
//  Programmer: Hank Childs
//  Creation:   May 18, 2000
//
//  Modifications:
//
//    Eric Brugger, Mon Oct 22 09:38:54 PDT 2001
//    Added code to set the bounding box flag.
//
//    Hank Childs, Fri Jul 19 17:47:05 PDT 2002
//    Make the transparency actor not render when in bounding box mode.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Make the externally rendered images actor not render when in bounding
//    box mode.
//
//    Mark C. Miller, Wed Jun  8 10:53:46 PDT 2005
//    Used new SaveVisibility interface for externally rendered images actor
//
// ****************************************************************************

void
VisWinPlots::StartBoundingBox(void)
{
    //
    // Set the bounding box flag.
    //
    bboxMode = true;

    //
    // If we have mesh lines shifted towards the camera, it is going to make
    // the bounding box look weird for very thin plots.
    //
    double zero[3];
    zero[0] = zero[1] = zero[2] = 0.;
    ShiftPlots(zero);

    //
    // Remove all of the plots from the renderer.
    //
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->VisibilityOff();
    }
    transparencyActor->VisibilityOff();
    extRenderedImagesActor->SaveVisibility(this, false);
    extRenderedImagesActor->UseBlankImage();
    //
    // Create the bounding box and add it to the renderer.
    //
    double  bounds[6];
    GetRealBounds(bounds);
    SetBoundingBox(bounds);
    bbox->VisibilityOn();

    // remove me ???
    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinPlots::EndBoundingBox
//
//  Purpose:
//      Adds all of the actors back to the renderer and removes the bounding
//      box.
//
//  Programmer: Hank Childs
//  Creation:   May 18, 2000
//
//  Modifications:
//
//    Eric Brugger, Mon Oct 22 09:38:54 PDT 2001
//    Added code to unset the bounding box flag.
//
//    Hank Childs, Fri Jul 19 17:47:05 PDT 2002
//    Make the transparency actor render again now that we are ending bounding
//    box mode.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Make the externally rendered images actor render again now that we are
//    ending bounding box mode.
//
//    Mark C. Miller, Wed Jun  8 10:53:46 PDT 2005
//    Used new RestoreVisibility interface for externally rendered images actor
//
// ****************************************************************************

void
VisWinPlots::EndBoundingBox(void)
{
    //
    // Unset the bounding box flag.
    //
    bboxMode = false;

    //
    // Remove the bounding box from the renderer.
    //
    bbox->VisibilityOff();

    //
    // Add all of the actors back to the renderer.
    //
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->VisibilityOn();
    }
    transparencyActor->VisibilityOn();
    extRenderedImagesActor->RestoreVisibility(this);

    //
    // We will not do an explicit render here -- only remember that we need
    // one.  This is to minimize the total number of draws.  This only works if
    // MotionEnd is called later (and it is by the VisitInteractor).
    //
    sceneHasChanged = true;
}


// ****************************************************************************
//  Method: VisWinPlots::SetBoundingBox
//
//  Purpose:
//      Creates the bounding box actor (provided it does not already exist)
//      and sets its input to be the bounding box specified.
//
//  Arguments:
//      bounds   A float containing the min and max x, y, and z values.
//
//  Programmer:  Hank Childs
//  Creation:    May 18, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Sep 22 09:59:27 PDT 2000
//    Added a modified call since a rectilinear grid does not keep track of
//    whether its parts are modified.
//
//    Kathleen Bonnell, Tue Sep  4 14:07:15 PDT 2001
//    Call SetBounds for the bboxGrid as it is now a vtkOutlineSource, not
//    a vtkRectilinearGrid.
//
// ****************************************************************************

void
VisWinPlots::SetBoundingBox(double *bounds)
{
    bboxGrid->SetBounds(bounds);
}


// ****************************************************************************
//  Method: VisWinPlots::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color for the bounding box and scalar bar text.
//
//  Arguments:
//      fr      The red component (rgb) of the foreground.
//      fg      The green component (rgb) of the foreground.
//      fb      The blue component (rgb) of the foreground.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Aug 30 23:42:32 PST 2001
//    Added a loop to set the plot legends' foreground color.
//
// ****************************************************************************

void
VisWinPlots::SetForegroundColor(double fr, double fg, double fb)
{
    bbox->GetProperty()->SetColor(fr, fg, fb);

    //
    // Set the foreground color for the plot legends.
    //
    double fgColor[3];
    fgColor[0] = fr;
    fgColor[1] = fg;
    fgColor[2] = fb;
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        avtLegend_p legend = (*it)->GetLegend();
        if(*legend != NULL)
        {
            legend->SetForegroundColor(fgColor);
        }
    }
}


// ****************************************************************************
//  Method: VisWinPlots::Start2DMode
//
//  Purpose:
//      Enters 2D mode for this colleague.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//   Modifications:
//
//     Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//     Added support for externally rendered images actor
//
// ****************************************************************************

void
VisWinPlots::Start2DMode(void)
{
    transparencyActor->AddToRenderer(mediator.GetCanvas());
    extRenderedImagesActor->AddToRenderer(mediator.GetCanvas());
}


// ****************************************************************************
//  Method: VisWinPlots::Stop2DMode
//
//  Purpose:
//      Stops 2D mode for this colleague.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
//   Modifications:
//
//     Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//     Added support for externally rendered images actor
//
// ****************************************************************************

void
VisWinPlots::Stop2DMode(void)
{
    transparencyActor->RemoveFromRenderer(mediator.GetCanvas());
    extRenderedImagesActor->RemoveFromRenderer(mediator.GetCanvas());
}


// ****************************************************************************
//  Method: VisWinPlots::Start3DMode
//
//  Purpose:
//      Enters 3D mode for this colleague.  This means adding the bounding
//      box actor to the canvas.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Added support for externally rendered image actor
//
// ****************************************************************************

void
VisWinPlots::Start3DMode(void)
{
    mediator.GetCanvas()->AddActor(bbox);
    transparencyActor->AddToRenderer(mediator.GetCanvas());
    extRenderedImagesActor->AddToRenderer(mediator.GetCanvas());
}


// ****************************************************************************
//  Method: VisWinPlots::Stop3DMode
//
//  Purpose:
//      Stops 3D mode for this colleague.  This means removing the bounding
//      box actor from the canvas.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Added support for externally rendered image actor
//
// ****************************************************************************

void
VisWinPlots::Stop3DMode(void)
{
    mediator.GetCanvas()->RemoveActor(bbox);
    transparencyActor->RemoveFromRenderer(mediator.GetCanvas());
    extRenderedImagesActor->RemoveFromRenderer(mediator.GetCanvas());
}


// ****************************************************************************
//  Method: VisWinPlots::UpdateView
//
//  Purpose:
//      Shifts the wireframe actors towards the camera to avoid z-buffer 
//      errors.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Feb 26 10:36:51 PST 2002
//    Shift the mesh lines by a little extra.
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Kathleen Bonnell, Tue Jul 23 15:01:55 PDT 2002   
//    Added call to UpdateScaleFactor.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Added support for externally rendered image actor
//
//    Eric Brugger, Wed Dec 24 10:07:33 PST 2003
//    Added code to adaptively reduce the amount the actor is moved toward
//    the camera as the image is zoomed by dividing the offset by the image
//    zoom.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Passed cam to extRenderedImagesActor to support stereo SR mode
//
// ****************************************************************************

void
VisWinPlots::UpdateView()
{
    vtkCamera *cam = mediator.GetCanvas()->GetActiveCamera();

    //
    // Tell the transparency actor what the current view is -- this will help
    // with its sorting algorithms.
    //
    transparencyActor->PrepareForRender(cam);
    extRenderedImagesActor->PrepareForRender(cam);

    //
    // Pull the wireframe actors a little closer to the camera to make sure
    // there are no z-buffer errors.  Note that canvas issues are hidden
    // by GetCanvas routine.
    //
    double distance = 0.003;
    double pos[3], foc[3];
    double imageZoom;
    cam->GetPosition(pos);
    cam->GetFocalPoint(foc);
    imageZoom = cam->GetFocalDisk();
    double projection[3];
    projection[0] = distance * (pos[0] - foc[0]) / imageZoom;
    projection[1] = distance * (pos[1] - foc[1]) / imageZoom;
    projection[2] = distance * (pos[2] - foc[2]) / imageZoom;

    ShiftPlots(projection);
    UpdateScaleFactor();
}


// ****************************************************************************
//  Method: VisWinPlots::ShiftPlots
//
//  Purpose:
//      Shifts all wireframe plots by the specified vector.  This can be used
//      to avoid z-buffer errors or to remove a previous shift when going to
//      bounding box mode.
//
//  Arguments:
//      vec       The vector to shift the plots by.  This vector is relative
//                to their true position in world space.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 16:39:52 PST 2001
//    Made this work adaptively for different plot types.
//
// ****************************************************************************

void
VisWinPlots::ShiftPlots(double vec[3])
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->ShiftByVector(vec);
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetBounds
//
//  Purpose:
//      Determine the bounds of all of the plots in the vis window.
//
//  Arguments:
//      bounds      An array that the bounds are copied into as min-x, max-x,
//                  min-y, max-y, min-z, and max-z.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
// ****************************************************************************

void
VisWinPlots::GetBounds(double bounds[6])
{
    if (userSetBounds)
    {
        for (int i = 0 ; i < 6 ; i++)
        {
            bounds[i] = setBounds[i];
        }
    }
    else
    {
        GetRealBounds(bounds);
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetRealBounds
//
//  Purpose:
//      Determine the bounds of all of the plots in the vis window.
//
//  Arguments:
//      bounds      An array that the bounds are copied into as min-x, max-x,
//                  min-y, max-y, min-z, and max-z.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov  8 15:14:54 PST 2000
//    Renamed GetRealBounds.
//
//    Hank Childs, Mon Jul 15 09:18:00 PDT 2002
//    Add capability to get actual extents rather than original extents.
//
// ****************************************************************************

void
VisWinPlots::GetRealBounds(double bounds[6])
{
    bool   setBounds = false;
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        if (! setBounds)
        {
            switch (spatialExtentType)
            {
              case AVT_ORIGINAL_EXTENTS:
                (*it)->GetOriginalBounds(bounds);
                break;
              case AVT_ACTUAL_EXTENTS:
                (*it)->GetActualBounds(bounds);
                break;
              default:
                debug1 << "Encountered bad extent type." << endl;
            }
            setBounds = true;
        }
        else
        {
            double tmpBounds[6];
            switch (spatialExtentType)
            {
              case AVT_ORIGINAL_EXTENTS:
                (*it)->GetOriginalBounds(tmpBounds);
                break;
              case AVT_ACTUAL_EXTENTS:
                (*it)->GetActualBounds(tmpBounds);
                break;
              default:
                debug1 << "Encountered bad extent type." << endl;
            }
            bounds[0] = (tmpBounds[0] < bounds[0] ? tmpBounds[0] : bounds[0]);
            bounds[1] = (tmpBounds[1] > bounds[1] ? tmpBounds[1] : bounds[1]);
            bounds[2] = (tmpBounds[2] < bounds[2] ? tmpBounds[2] : bounds[2]);
            bounds[3] = (tmpBounds[3] > bounds[3] ? tmpBounds[3] : bounds[3]);
            bounds[4] = (tmpBounds[4] < bounds[4] ? tmpBounds[4] : bounds[4]);
            bounds[5] = (tmpBounds[5] > bounds[5] ? tmpBounds[5] : bounds[5]);
        }
    }
    if (! setBounds)
    {
        bounds[0] = bounds[2] = bounds[4] = 0.;
        bounds[1] = bounds[3] = bounds[5] = 1.;
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetDataRange
//
//  Purpose:
//      Determine the min & max data values of all of the plots in the vis window.
//
//  Arguments:
//      dmin    Storage for the minimum value. 
//      dmax    Storage for the maximum value. 
//                  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002
//
// ****************************************************************************

void
VisWinPlots::GetDataRange(double &dmin, double &dmax)
{
    bool   setRange = false;
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        if (! setRange)
        {
            (*it)->GetDataExtents(dmin, dmax);
            setRange = true;
        }
        else
        {
            double tmin, tmax;
            (*it)->GetDataExtents(tmin, tmax);
            dmin = (tmin < dmin ? tmin : dmin);
            dmax = (tmax > dmax ? tmax : dmax);
        }
    }
    if (! setRange)
    {
        dmin = dmax = 0.;
    }
}

// ****************************************************************************
//  Method: VisWinPlots::AdjustCamera
//
//  Purpose:
//      Determines if the camera needs to be adjusted for the new actor.
//
//  Arguments:
//      oldbounds   The old bounds of the actors as 
//                  <xmin, xmax, ymin, ymax, zmin, zmax>.
//      newbounds   The new bounds of the actors as 
//                  <xmin, xmax, ymin, ymax, zmin, zmax>.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Added calls to have light positions updated if the camera is updated.
//    vtkRenderers handle this fine for rotations and zooms, but if the bounds 
//    have increased, the base position for the lights must be updated 
//    manually.
//
//    Kathleen Bonnell, Thu Aug 29 09:49:36 PDT 2002  
//    Removed call to  UpateLightPositions as vw->ResetView currently has
//    no effect.  It should be handled by VisWindow anyway. 
//    
// ****************************************************************************

void
VisWinPlots::AdjustCamera(const double oldbounds[6], const double newbounds[6])
{
    VisWindow *vw = mediator;
    if (userSetBounds)
    {
        if (currentBounds[0] != setBounds[0] || 
            currentBounds[1] != setBounds[1] ||
            currentBounds[2] != setBounds[2] ||
            currentBounds[3] != setBounds[3] ||
            currentBounds[4] != setBounds[4] ||
            currentBounds[5] != setBounds[5])
        {
            vw->ResetView();
        }
    }
    else
    {
        //
        // If the new actor is outside the original bounds, reset the camera.
        //
        if (newbounds[0] < oldbounds[0] || newbounds[1] > oldbounds[1] ||
            newbounds[2] < oldbounds[2] || newbounds[3] > oldbounds[3] ||
            newbounds[4] < oldbounds[4] || newbounds[5] > oldbounds[5])
        {
            vw->ResetView();
        }
    }

    for (int i = 0 ; i < 6 ; i++)
    {
        currentBounds[i] = newbounds[i];
    }
}


// ****************************************************************************
//  Method: VisWinPlots::SetBounds
//
//  Purpose:
//      Allows the user to specify the bounds when a new actor is added or
//      deleted.  This is primarily for animation.
//
//  Arguments:
//      bounds    The desired bounds as <xmin,xmax,ymin,ymax,zmin,zmax>.
//
//  Programmer: Hank Childs
//  Creation:   August 6, 2000
//
// ****************************************************************************

void
VisWinPlots::SetBounds(const double bounds[6])
{
    userSetBounds = true;
    for (int i = 0 ; i < 6 ; i++)
    {
        setBounds[i] = bounds[i];
    }
}


// ****************************************************************************
//  Method: VisWinPlots::UnsetBounds
//
//  Purpose:
//      Allows the use to specify that bounds should no longer be imposed on
//      the new actors.
//
//  Programmer: Hank Childs
//  Creation:   August 6, 2000
//
// ****************************************************************************

void
VisWinPlots::UnsetBounds(void)
{
    userSetBounds = false;
}


// ****************************************************************************
//  Method: VisWinPlots::OrderPlots
//
//  Purpose:
//      Removes all plots from the window and re-adds them in rendering order
//      based on the the RenderOrder set in the behavior. 
//
//  Arguments:
//    aa     Indicates if antialiasing mode is on. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 3, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Jul  9 09:09:08 PDT 2002
//    Add support for transparency.
//
//    Mark C. Miller, Thu Dec 19 11:38:05 PST 2002
//    Added support for externally rendered images actor
//
//    Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003 
//    Added bool argument, which is passed to GetRenderOrder.
//    Plots with RenderOrder 'ABSOLUTELY_LAST' will be rendered after
//    the transparent actors. (Generally only in Antialiased mode).
//
//    Kathleen Bonnell, Wed Dec  3 16:42:38 PST 2003 
//    Added call to ReAddToolsToRenderWindow. 
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added code to get 'aa' and calls to ReAddColleaguesToRenderWindow
//    Note that old call to ReAddToolsToRenderWindow happens as part of
//    ReAddColleaguesToRenderWindow
//
//    Mark C. MIller, Thu Jan 20 22:27:39 PST 2005
//    Changed to use TransparenciesMightExist
//
// ****************************************************************************

void
VisWinPlots::OrderPlots()
{
    bool aa = mediator.GetAntialiasing();
    std::vector< avtActor_p >::iterator it;
    std::vector< avtActor_p > orderPlots;
    std::vector< avtActor_p >::iterator oit;
  
    avtActor_p p;
    //
    // first remove all the plots from the window
    // and insert them into the ordered list as we go
    //
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        p = *it;
        p->Remove(mediator.GetCanvas(), mediator.GetForeground());
        oit = orderPlots.begin();
        while (oit != orderPlots.end() && 
               (*oit)->GetRenderOrder(aa) <= p->GetRenderOrder(aa)) 
        {
            oit++;
        }
        orderPlots.insert(oit, p);
    }
    transparencyActor->RemoveFromRenderer(mediator.GetCanvas());
    extRenderedImagesActor->RemoveFromRenderer(mediator.GetCanvas());

    //
    // now add the plots back the window using the ordered plots list.
    //
    for (oit = orderPlots.begin(); oit != orderPlots.end(); oit++)
    {
        if  ((*oit)->GetRenderOrder(aa) != ABSOLUTELY_LAST)
            (*oit)->Add(mediator.GetCanvas(), mediator.GetForeground());
    }

    //
    // The tools don't show up if added after the transparency actor
    // (when all other colleagues are re-added, so re-add them here.
    //
    bool addedColleaguesBeforeTransparencyActor =
             transparencyActor->TransparenciesMightExist();
    if (addedColleaguesBeforeTransparencyActor)
        mediator.ReAddColleaguesToRenderWindow();
    transparencyActor->AddToRenderer(mediator.GetCanvas());
    extRenderedImagesActor->AddToRenderer(mediator.GetCanvas());
    for (oit = orderPlots.begin(); oit != orderPlots.end(); oit++)
    {
        if  ((*oit)->GetRenderOrder(aa) == ABSOLUTELY_LAST)
            (*oit)->Add(mediator.GetCanvas(), mediator.GetForeground());
    }

    //
    //  If we are in anti-aliasing mode and we do not have any transparency,
    //  Allow other colleagues to place themselves last in the renderer, to
    //  reduce artefacts. 
    //
    if (!addedColleaguesBeforeTransparencyActor && aa)
    {
        mediator.ReAddColleaguesToRenderWindow();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetAllDatasets
//
//  Purpose:
//      Gets all of the datasets attached to this module.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Sun May 26 19:21:31 PDT 2002
//    Overhauled routine to meet new interface for actors.
//
//    Jeremy Meredith, Fri Jul 26 13:56:05 PDT 2002
//    Set the source for the clone since it is sometimes needed when saving
//    windows.
//
// ****************************************************************************

avtDataset_p
VisWinPlots::GetAllDatasets(void)
{
    //
    // We are assuming that all of the inputs are avtDatasets.  Otherwise,
    // we are in trouble, because it is not clear how to merge a dataset and
    // an image, for example.
    //
    std::vector< avtActor_p >::iterator it;
    avtDataset_p rv = NULL;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        avtDataObject_p dob = (*it)->GetDataObject();
        if (strcmp(dob->GetType(), "avtDataset") != 0)
        {
            continue;
        }

        //
        // If this is the first dataset we have encountered, make it the one
        // we will return.  Otherwise, merge the new one into the
        // conglomeration we have been building.
        //
        if (*rv == NULL)
        {
            avtDataObject_p clone = dob->Clone();
            clone->SetSource(dob->GetTerminatingSource());
            CopyTo(rv, clone);
        }
        else
        {
            //
            // The 'true' means to do a merge, even if the data attributes
            // don't match.
            //
            rv->Merge(*dob, true);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: VisWinPlots::MotionBegin
//
//  Purpose:
//      Called when motion has just begun.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 26, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Aug 22 10:54:35 PDT 2007
//    Set individual plots into reduced detail mode.
//
// **************************************************************************** 

void
VisWinPlots::MotionBegin(void)
{
    transparencyActor->UsePerfectSort(false);
    //
    // We don't need to re-render even if the actor has geometry since
    // we are switching to a lower quality mode of operation.
    //

    // Switch the plots that can accept it into reduced detail mode.
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->ReducedDetailModeOn();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::MotionEnd
//
//  Purpose:
//      Called when motion has just ended.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jul 26 14:32:24 PDT 2002
//    Made perfect sorting a permanent mode instead of active for only
//    a single frame.  It is now disabled by the MotionBegin method.
//
//    Brad Whitlock, Wed Aug 22 12:08:02 PDT 2007
//    Return plots from reduced detail mode.
//
// **************************************************************************** 

void
VisWinPlots::MotionEnd(void)
{
    if (transparencyActor->UsePerfectSort(true))
    {
        //
        // The transparency actor has geometry -- force a re-render to pick
        // up its changes.
        //
        sceneHasChanged = true;
    }

    // Switch the plots that can accept it out of reduced detail mode.
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        sceneHasChanged |= (*it)->ReducedDetailModeOff();
    }

    if (sceneHasChanged)
    {
        mediator.Render();
        sceneHasChanged = false;
    }
}


// ****************************************************************************
//  Method: VisWinPlots::ScalePlots
//
//  Purpose:
//      Scales all plots by the specified vector.  
//
//  Arguments:
//      vec       The vector to scale the plots by.  This vector is relative
//                to their true position in world space.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 11, 2002 
//
// ****************************************************************************

void
VisWinPlots::ScalePlots(const double vec[3])
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->ScaleByVector(vec);
    }
}


// ****************************************************************************
//  Method: VisWinPlots::UpdateScaleFactor
//
//  Purpose:
//      Allows decoration actors to update their scale factor. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 19, 2002 
//
// ****************************************************************************

void
VisWinPlots::UpdateScaleFactor()
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->UpdateScaleFactor();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::TurnLightingOn
//
//  Purpose:
//      Allows actors to update their lighting coefficients. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
VisWinPlots::TurnLightingOn()
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->TurnLightingOn();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::TurnLightingOff
//
//  Purpose:
//      Allows actors to update their lighting coefficients. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
VisWinPlots::TurnLightingOff()
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->TurnLightingOff();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::SetAmbientCoefficient
//
//  Purpose:
//      Allows actors to update their lighting coefficients. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
VisWinPlots::SetAmbientCoefficient(const double amb)
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->SetAmbientCoefficient(amb);
    }
}

// ****************************************************************************
//  Method: VisWinPlots::SetSurfaceRepresentation
//
//  Purpose:
//      Allows actors to update their surface representation. 
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 23 15:48:39 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep  2 08:52:16 PDT 2004
//    Set specular properties when surface rep is Surface.
//
// ****************************************************************************

void
VisWinPlots::SetSurfaceRepresentation(int rep)
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->SetSurfaceRepresentation(rep);
    }

    //
    //  If we are changing to surface rep, and are in 3D mode, make
    //  sure the specular properties get reset. (They were not 
    //  applied if old rep was wireframe).
    //
    if (rep == 0 && mediator.GetMode() == WINMODE_3D)
    {
        SetSpecularProperties(mediator.GetSpecularFlag(),
                              mediator.GetSpecularCoeff(),
                              mediator.GetSpecularPower(),
                              mediator.GetSpecularColor());
    }
}

// ****************************************************************************
//  Method: VisWinPlots::SetImmediateModeRendering
//
//  Purpose:
//      Allows actors to create or free their display lists.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 23 15:48:39 PST 2002
//
// ****************************************************************************

void
VisWinPlots::SetImmediateModeRendering(bool immediateMode)
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->SetImmediateModeRendering(immediateMode);
    }
}


// ****************************************************************************
//  Method: VisWinPlots::SetSpecularProperties
//
//  Purpose:
//      Set specular properties on actors.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 14, 2003
//
// ****************************************************************************

void
VisWinPlots::SetSpecularProperties(bool flag, double coeff, double power, 
                                   const ColorAttribute &color)
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->SetSpecularProperties(flag, coeff,power,color);
    }
}

// ****************************************************************************
//  Method: VisWinPlots::SetColorTexturingFlag
//
//  Purpose:
//      Set color texturing flag on actors.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 18 11:15:16 PDT 2006
//
// ****************************************************************************

void
VisWinPlots::SetColorTexturingFlag(bool val)
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->SetColorTexturingFlag(val);
    }
}

// ****************************************************************************
//  Method: VisWinPlots::SetExternalRenderCallback
//
//  Purpose:
//      Forward request to register an external rendering callback to the
//      externally rendered images actor.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 13, 2003 
//
// ****************************************************************************

void
VisWinPlots::SetExternalRenderCallback(VisCallbackWithDob *cb,
    void *data)
{
    extRenderedImagesActor->RegisterExternalRenderCallback(cb, data);
}

// ****************************************************************************
//  Method: VisWinPlots::EnableExternalRenderRequests
//
//  Purpose:
//     Forward request to enable external rendering requests to the ERI actor 
//
//  Programmer: Mark C. Miller 
//  Creation:   February 5, 2003 
//
// ****************************************************************************

bool
VisWinPlots::EnableExternalRenderRequests(void)
{
    return extRenderedImagesActor->EnableExternalRenderRequests();
}

// ****************************************************************************
//  Method: VisWinPlots::DisableExternalRenderRequests
//
//  Purpose:
//     Forward request to disable external rendering requests to the ERI actor 
//
//  Programmer: Mark C. Miller 
//  Creation:   February 5, 2003 
//
//  Modified:
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to clear the external renderer's cached image if disabling it.
// ****************************************************************************

bool
VisWinPlots::DisableExternalRenderRequests(bool bClearImage)
{
    return extRenderedImagesActor->DisableExternalRenderRequests(bClearImage);
}

// ****************************************************************************
//  Method: VisWinPlots::IsMakingExternalRenderRequests
//
//  Purpose: Query externally rendered images actor to see if it is making
//  requests
//
//  Programmer: Mark C. Miller 
//  Creation:   March 27, 2007 
//
// ****************************************************************************

bool
VisWinPlots::IsMakingExternalRenderRequests(void) const
{
    return extRenderedImagesActor->IsMakingExternalRenderRequests();
}

// ****************************************************************************
//  Method: VisWinPlots::GetAverageExternalRenderingTime
//
//  Purpose: Return average rendering time for last 5 most recent external
//  renders 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 27, 2007 
//
// ****************************************************************************

double
VisWinPlots::GetAverageExternalRenderingTime(void) const
{
    return extRenderedImagesActor->GetAverageRenderingTime();
}

// ****************************************************************************
//  Method: VisWinPlots::DoNextExternalRenderAsVisualQueue
//
//  Purpose: For next render only, render the 'in-progress' visual que 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 27, 2007 
//
// ****************************************************************************

void
VisWinPlots::DoNextExternalRenderAsVisualQueue(int w, int h, const double *c)
{
    extRenderedImagesActor->DoNextExternalRenderAsVisualQueue(w, h, c);
}

// ****************************************************************************
//  Method: VisWinPlots::FullFrameOn
//
//  Purpose:
//    Scales the plots to fit into the full-frame.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 6, 2003
//
// ****************************************************************************

void
VisWinPlots::FullFrameOn(const double scale, const int type)
{
    double vec[3] = {1., 1., 1.};
    if (type == 0) // x-axis is being scaled
        vec[0] = scale; 
    else // if (type == 1) // y-axis is being scaled
        vec[1] = scale; 

    ScalePlots(vec);
}


// ****************************************************************************
//  Method: VisWinPlots::FullFrameOff
//
//  Purpose:
//    Undoes the scale that made plots fit into the full-frame.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 6, 2003
//
// ****************************************************************************

void
VisWinPlots::FullFrameOff()
{
    double vec[3] = {1., 1., 1.};
    ScalePlots(vec);
}


// ****************************************************************************
//  Method: VisWinPlots::TransparenciesExist
//
//  Purpose:
//    Returns true if there are transparent actors. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 3, 2003 
//
// ****************************************************************************

bool
VisWinPlots::TransparenciesExist()
{
    return transparencyActor->TransparenciesExist();
}



// ****************************************************************************
//  Method: VisWinPlots::SuspendOpaqueGeometry
//
//  Purpose:
//    Make opaque geometry invisible
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:32:06 PDT 2004
//
// ***************************************************************************

void
VisWinPlots::SuspendOpaqueGeometry()
{
    for(int i=0; i<plots.size(); i++)
    {
        if( plots[i]->IsVisible() )
        {
            plots[i]->OpaqueVisibilityOff();
        }
    }
}

// ****************************************************************************
//  Method: VisWinPlots::SuspendTranslucentGeometry
//
//  Purpose:
//    Make translucent geometry invisible
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:32:06 PDT 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 13:39:48 PDT 2004
//    Had it change on the transparency actor as a whole because it is
//    less error prone (and easier and more efficient).
//
// ***************************************************************************

void
VisWinPlots::SuspendTranslucentGeometry()
{
    transparencyActor->SuspendRendering();
}

// ****************************************************************************
//  Method: VisWinPlots::ResumeOpaqueGeometry
//
//  Purpose:
//    Make opaque geometry visible again
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:32:06 PDT 2004
//
// ***************************************************************************

void
VisWinPlots::ResumeOpaqueGeometry()
{
    for(int i=0; i<plots.size(); i++)
    {
        if( plots[i]->IsVisible() )
        {
            plots[i]->OpaqueVisibilityOn();
        }
    }
}

// ****************************************************************************
//  Method: VisWinPlots::ResumeTranslucentGeometry
//
//  Purpose:
//    Make translucent geometry visible again
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:32:06 PDT 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 13:39:48 PDT 2004
//    Had it change on the transparency actor as a whole because it is
//    less error prone (and easier and more efficient).
//
// ***************************************************************************

void
VisWinPlots::ResumeTranslucentGeometry()
{
    transparencyActor->ResumeRendering();
}


// ****************************************************************************
//  Method: VisWinPlots::MakeAllPickable
//
//  Purpose:
//      Make all the actors pickable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 4, 2004
//
// ****************************************************************************

void
VisWinPlots::MakeAllPickable()
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->MakePickable();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::MakeAllUnPickable
//
//  Purpose:
//      Make all the actors unpickable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 4, 2004
//
// ****************************************************************************

void
VisWinPlots::MakeAllUnPickable()
{
    std::vector< avtActor_p >::iterator it;
    for (it = plots.begin() ; it != plots.end() ; it++)
    {
        (*it)->MakeUnPickable();
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetMaxZShift
//
//  Purpose:
//    Return the maximum that any plot has been shifted in z. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 27, 2005 
//
// ****************************************************************************

double
VisWinPlots::GetMaxZShift()
{
    double maxZShift = 0;
    if (mediator.GetMode() ==  WINMODE_2D)
    {
        std::vector< avtActor_p >::iterator it;
        double actorZPos;
        for (it = plots.begin() ; it != plots.end() ; it++)
        {
            actorZPos = (*it)->GetZPosition();
            if (actorZPos > maxZShift)
                maxZShift = actorZPos;
        }
    }
    return maxZShift;
}

// ****************************************************************************
//  Method: VisWinPlots::GetMaxZShift
//
//  Programmer: Mark C. Miller
//  Creation:   April 5, 2006 
//
//  Notes: Moved from original implementation in ViewerPlotList
// ****************************************************************************
bool
VisWinPlots::DoAllPlotsAxesHaveSameUnits()
{
    int i;
    bool first = true;
    std::string theUnits = "";
    for (i = 0; i < plots.size(); i++)
    {
        if (plots[i]->IsVisible())
        {
            avtDataObject_p dob = plots[i]->GetDataObject();

            if (*dob != NULL)
            {
                avtDataAttributes &datts = dob->GetInfo().GetAttributes();
                if (first)
                {
                    if (datts.GetXUnits() != "")
                        theUnits = datts.GetXUnits();
                    else if (datts.GetYUnits() != "")
                        theUnits = datts.GetYUnits();
                    if (theUnits != "")
                        first = false;
                }
                if ((datts.GetXUnits() != "" && datts.GetXUnits() != theUnits) ||
                    (datts.GetYUnits() != "" && datts.GetYUnits() != theUnits))
                    return false;
            }
        }
    }
    return true;
}
