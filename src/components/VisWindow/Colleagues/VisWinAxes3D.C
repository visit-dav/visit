// ************************************************************************* //
//                                VisWinAxes3D.C                             //
// ************************************************************************* //

#include <vtkVisItCubeAxesActor.h>
#include <vtkRenderer.h>
#include <vtkActorCollection.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkOutlineSource.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>
#include <VisWinAxes3D.h>


// ****************************************************************************
//  Method: VisWinAxes3D constructor
//
//  Arguments:
//      p      A proxy that allows more access to the VisWindow for this 
//             colleague.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 28, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug  3 14:55:59 PDT 2001
//    Changed from using a 2d cube axes actor to using a 3d version.
//
//    Kathleen Bonnell, Tue Oct 30 10:30:10 PST 2001 
//    Removed last*Pow, last*AxisDigits. 
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//    Make the axes un-pickable.
//
//    Kathleen Bonnell, Thu Oct  3 14:41:19 PDT 2002  
//    Disable lighting on the axesBox by setting its ambient/diffuse
//    coefficients. 
//
//    Brad Whitlock, Thu Jul 28 10:07:18 PDT 2005
//    Added new members for storing user-specified axis titles and units.
//
// ****************************************************************************

VisWinAxes3D::VisWinAxes3D(VisWindowColleagueProxy &p) : VisWinColleague(p),
    userXTitle(), userYTitle(), userZTitle(), 
    userXUnits(), userYUnits(), userZUnits()
{
    axes = vtkVisItCubeAxesActor::New();
    axes->SetFlyModeToClosestTriad();
    axes->GetProperty()->SetColor(0, 0, 0);
    axes->SetCornerOffset(0.);
    axes->PickableOff();

    axesBoxSource = vtkOutlineSource::New();
    axesBoxMapper = vtkPolyDataMapper::New();
    axesBoxMapper->SetInput(axesBoxSource->GetOutput());
    axesBox = vtkActor::New();
    axesBox->SetMapper(axesBoxMapper);
    axesBox->PickableOff();
    axesBox->GetProperty()->SetAmbient(1.);
    axesBox->GetProperty()->SetDiffuse(0.);
    visibility = true;

    for (int i = 0; i < 6; i++)
    {
        currentBounds[i] = -1;
    }
    addedAxes3D = false;

    userXUnitsFlag = false;
    userYUnitsFlag = false;
    userZUnitsFlag = false;
    userXTitleFlag = false;
    userYTitleFlag = false;
    userZTitleFlag = false;
}


// ****************************************************************************
//  Method: VisWinAxes3D destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

VisWinAxes3D::~VisWinAxes3D()
{
    if (axes != NULL)
    {
        axes->Delete();
        axes = NULL;
    }
    if (axesBoxSource != NULL)
    {
        axesBoxSource->Delete();
        axesBoxSource = NULL;
    }
    if (axesBoxMapper != NULL)
    {
        axesBoxMapper->Delete();
        axesBoxMapper = NULL;
    }
    if (axesBox != NULL)
    {
        axesBox->Delete();
        axesBox = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinAxes3D::SetForegroundColor
//
//  Purpose:
//      Sets the color of the axes.
//
//  Arguments:
//      fr        The red component of the foreground color.
//      fg        The green component of the foreground color.
//      fb        The blue component of the foreground color.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

void
VisWinAxes3D::SetForegroundColor(double fr, double fg, double fb)
{
    axes->GetProperty()->SetColor(fr, fg, fb);
    axesBox->GetProperty()->SetColor(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinAxes3D::Start3DMode
//
//  Purpose:
//      We are about to enter 3D mode, so the axes should be added to the
//      background, as long as there are plots.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

void
VisWinAxes3D::Start3DMode(void)
{
    if (ShouldAddAxes3D())
    {
        AddAxes3DToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxes3D::Stop3DMode
//
//  Purpose:
//      We are about to leave 3D mode, so the axes should be removed from the
//      background.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

void
VisWinAxes3D::Stop3DMode(void)
{
    RemoveAxes3DFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxes3D::HasPlots
//
//  Purpose:
//      This routine is how the vis window tells this colleague that it now
//      has plots.  We can now add the axes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

void
VisWinAxes3D::HasPlots(void)
{
    if (ShouldAddAxes3D())
    {
        AddAxes3DToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinAxes3D::NoPlots
//
//  Purpose:
//      This routine is how the vis window tells this colleague that it no
//      longer has plots.  Remove the axes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

void
VisWinAxes3D::NoPlots(void)
{
    RemoveAxes3DFromWindow();
}


// ****************************************************************************
//  Method: VisWinAxes3D::ShouldAddAxes3D
//
//  Purpose:
//      There are two constraints on whether the axes should be added to the
//      VisWindow - whether we are in 3D mode and whether we have plots.  This
//      buffers the logic for that so the individual operations don't need to
//      know about each other.
//
//  Returns:    true if the axes should be added to the window, false 
//              otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
// ****************************************************************************

bool
VisWinAxes3D::ShouldAddAxes3D()
{
    return (mediator.GetMode() == WINMODE_3D && mediator.HasPlots());
}


// ****************************************************************************
//  Method: VisWinAxes3D::AddAxes3DToWindow
//
//  Purpose:
//      Adds the axes to the window.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 15:31:46 PDT 2006
//    AddProp has been deprecated, use AddViewProp instead.
//
// ****************************************************************************

void
VisWinAxes3D::AddAxes3DToWindow(void)
{
    if (addedAxes3D)
    {
        return;
    }

    //
    // Get the camera of the canvas and register it with the axes.
    //
    axes->SetCamera(mediator.GetCanvas()->GetActiveCamera());

    //
    // Add the axes to the background (note that we are using a different
    // renderer's camera -- the canvas').
    //
    mediator.GetCanvas()->AddViewProp(axes);
    mediator.GetCanvas()->AddActor(axesBox);

    addedAxes3D = true;
}


// ****************************************************************************
//  Method: VisWinAxes3D::RemoveAxes3DFromWindow
//
//  Purpose:
//      Removes the axes from the window.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 15:31:46 PDT 2006
//    RemoveProp has been deprecated, use RemoveViewProp instead.
//
// ****************************************************************************

void
VisWinAxes3D::RemoveAxes3DFromWindow(void)
{
    if (!addedAxes3D)
    {
        return;
    }

    mediator.GetCanvas()->RemoveViewProp(axes);
    mediator.GetCanvas()->RemoveActor(axesBox);

    addedAxes3D = false;
}


// ****************************************************************************
//  Method: VisWinAxes3D::SetBounds
//
//  Purpose:
//    Sets the bounds for this axes to use.
//
//  Arguments:
//    bounds    The bounds as min-x, max-x, min-y, max-y, min-z, and max-z.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 20, 2l01
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct 30 10:30:10 PST 2001 
//    Removed calls to AdjustValues, AdjustRange. Functionality moved to
//    more appropriate location of vtkVisItCubeAxesActor.
//
// ****************************************************************************

void
VisWinAxes3D::SetBounds(double bounds[6])
{
    bool boundsChanged = false; 

    for (int i = 0; i < 6 ; i++)
    {
      if (currentBounds[i] == bounds[i])
      {
          continue;
      }

      boundsChanged = true; 
      break;
    }

    if (boundsChanged)
    {
        //
        // Add a fudge-factor to prevent axes from being obscured by plots
        // that fill their full extents. 
        //
        double fudgeX = (bounds[1] - bounds[0]) * 0.001;
        double fudgeY = (bounds[3] - bounds[2]) * 0.001;
        double fudgeZ = (bounds[5] - bounds[4]) * 0.001;
        currentBounds[0] = bounds[0] - fudgeX;
        currentBounds[1] = bounds[1] + fudgeX;
        currentBounds[2] = bounds[2] - fudgeY;
        currentBounds[3] = bounds[3] + fudgeY;
        currentBounds[4] = bounds[4] - fudgeZ;
        currentBounds[5] = bounds[5] + fudgeZ;

        axes->SetBounds(currentBounds);
        axesBoxSource->SetBounds(currentBounds);
    }
}


// ****************************************************************************
//  Method: VisWinAxes3D::UpdateView
//
//  Purpose:
//      Updates the axes' camera with the active camera. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 20, 2001
//
// ****************************************************************************

void
VisWinAxes3D::UpdateView()
{
    axes->SetCamera(mediator.GetCanvas()->GetActiveCamera());
}


// ****************************************************************************
//  Method: VisWinAxes3D::UpdatePlotList
//
//  Purpose:
//      Decides what the units are for the X, Y, and Z directions.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Jul 23 17:59:43 PST 2004
//    I added support for setting the axis titles based on the x,y,z labels.
//
//    Brad Whitlock, Thu Jul 28 10:06:20 PDT 2005
//    I added support for overriding the title and units.
//
// ****************************************************************************

void
VisWinAxes3D::UpdatePlotList(vector<avtActor_p> &list)
{
    int nActors = list.size();
    string x, y, z;
    string xt, yt, zt;

    for (int i = 0 ; i < nActors ; i++)
    {
        avtDataAttributes &atts = 
                             list[i]->GetBehavior()->GetInfo().GetAttributes();

        // Last one in is the winner.
        if (atts.GetXUnits() != "")
            x = atts.GetXUnits();
        if (atts.GetYUnits() != "")
            y = atts.GetYUnits();
        if (atts.GetZUnits() != "")
            z = atts.GetZUnits();

        // Last one in is the winner.
        if (atts.GetXLabel() != "")
            xt = atts.GetXLabel();
        if (atts.GetYLabel() != "")
            yt = atts.GetYLabel();
        if (atts.GetZLabel() != "")
            zt = atts.GetZLabel();
    }

    if(userXUnitsFlag)
        axes->SetXUnits(userXUnits.c_str());
    else
        axes->SetXUnits(x.c_str());

    if(userYUnitsFlag)
        axes->SetYUnits(userYUnits.c_str());
    else
        axes->SetYUnits(y.c_str());

    if(userZUnitsFlag)
        axes->SetZUnits(userZUnits.c_str());
    else
        axes->SetZUnits(z.c_str());

    if(userXTitleFlag)
        axes->SetXTitle(userXTitle.c_str());
    else
        axes->SetXTitle(xt.c_str());

    if(userYTitleFlag)
        axes->SetYTitle(userYTitle.c_str());
    else
        axes->SetYTitle(yt.c_str());

    if(userZTitleFlag)
        axes->SetZTitle(userZTitle.c_str());
    else
        axes->SetZTitle(zt.c_str());
}


// ****************************************************************************
//  Function: SetXTickVisibility
//
//  Purpose:
//    Sets the visibility of x-axis ticks.
//
//  Arguments:
//    xVis       The visibility of the x-axis ticks.
//    xLabelsVis The visibility of the x-axis labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetXTickVisibility(int xVis, int xLabelsVis)
{
    axes->SetXAxisMinorTickVisibility(xVis);
    // Major ticks dependent upon labels visibility, too
    axes->SetXAxisTickVisibility(xVis || xLabelsVis);
}


// ****************************************************************************
//  Function: SetYTickVisibility
//
//  Pupose:
//    Sets the visibility of y-axis ticks.
//
//  Arguments:
//    yVis       The visibility of the y-axis ticks.
//    yLabelsVis The visibility of the y-axis labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetYTickVisibility(int yVis, int yLabelsVis)
{
    axes->SetYAxisMinorTickVisibility(yVis);
    // Major ticks dependent upon labels visibility, too
    axes->SetYAxisTickVisibility(yVis || yLabelsVis);
}

// ****************************************************************************
//  Function: SetZTickVisibility
//
//  Purpose:
//    Sets the visibility of z-axis ticks.
//
//  Arguments:
//    zVis       The visibility of the z-axis ticks.
//    zLabelsVis The visibility of the z-axis labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetZTickVisibility(int zVis, int zLabelsVis)
{
    axes->SetZAxisMinorTickVisibility(zVis);
    // Major ticks dependent upon labels visibility, too
    axes->SetZAxisTickVisibility(zVis || zLabelsVis);
}


// ****************************************************************************
//  Function: SetXLabelVisibility
//
//  Purpose:
//      Sets the visibility of x-axis labels.
//
//  Arguments:
//      x-vis     The visibility of the x-axis labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetXLabelVisibility(int xVis)
{
    axes->SetXAxisLabelVisibility(xVis);
    axes->SetXAxisTickVisibility(xVis);
}

   
// ****************************************************************************
//  Function: SetYLabelVisibility
//
//  Purpose:
//      Sets the visibility of y-axis labels.
//
//  Arguments:
//      y-vis     The visibility of the y-axis labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetYLabelVisibility(int yVis)
{
    axes->SetYAxisLabelVisibility(yVis);
}

   
// ****************************************************************************
//  Function: SetZLabelVisibility
//
//  Purpose:
//      Sets the visibility of z-axis labels.
//
//  Arguments:
//      z-vis     The visibility of the z-axis labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetZLabelVisibility(int zVis)
{
    axes->SetZAxisLabelVisibility(zVis);
}


// ****************************************************************************
//  Function: SetXGridVisibility
//
//  Purpose:
//      Sets the visibility of x-axis gridlines.
//
//  Arguments:
//      x-vis     The visibility of the x-axis gridlines.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 3, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetXGridVisibility(int xVis)
{
    axes->SetDrawXGridlines(xVis);
}


// ****************************************************************************
//  Function: SetYGridVisibility
//
//  Purpose:
//      Sets the visibility of y-axis gridlines.
//
//  Arguments:
//      y-vis     The visibility of the y-axis gridlines.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 3, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetYGridVisibility(int yVis)
{
    axes->SetDrawYGridlines(yVis);
}


// ****************************************************************************
//  Function: SetZGridVisibility
//
//  Purpose:
//      Sets the visibility of z-axis gridlines.
//
//  Arguments:
//      z-vis     The visibility of the z-axis gridlines.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 3, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetZGridVisibility(int zVis)
{
    axes->SetDrawZGridlines(zVis);
}

   
// ****************************************************************************
//  Function: SetVisibility
//
//  Purpose:
//      Sets the visibility of this colleague.
//
//  Arguments:
//      vis     The visibility of this colleague.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetVisibility(int vis)
{
    visibility = vis;
    axes->SetVisibility(vis);
    axesBox->SetVisibility(vis);
}
   

// ****************************************************************************
//  Function: SetBBoxVisibility
//
//  Purpose:
//      Sets the visibility of the bounding box. 
//
//  Arguments:
//      vis     The visibility of this bounding box.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetBBoxVisibility(int vis)
{
    axesBox->SetVisibility(vis && visibility);
}


// ****************************************************************************
//  Function: SetFlyMode
//
//  Purpose:
//      Sets the fly-mode (closest-triad, furthest-triad, outer-edges).
//
//  Arguments:
//      mode     The fly-mode.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug  3 15:04:32 PDT 2001
//    Added Static mode, enabled furthest triad mode.
//
//    Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001 
//    Changed Static to StaticEdges, added StaticTriad  mode. 
//
// ****************************************************************************

void
VisWinAxes3D::SetFlyMode(int mode)
{
    switch (mode)
    {
        case 0 : axes->SetFlyModeToClosestTriad(); break;
        case 1 : axes->SetFlyModeToFurthestTriad(); break;
        case 2 : axes->SetFlyModeToOuterEdges(); break;
        case 3 : axes->SetFlyModeToStaticTriad(); break;
        case 4 : axes->SetFlyModeToStaticEdges(); break;
        default : axes->SetFlyModeToClosestTriad(); break;
    }
}

  
// ****************************************************************************
//  Function: SetTickLocation
//
//  Purpose:
//      Sets the location of the ticks.
//
//  Arguments:
//      loc     The location of the ticks.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 28, 2001.
//
// ****************************************************************************

void
VisWinAxes3D::SetTickLocation(int loc)
{
    axes->SetTickLocation(loc);
} 


// ****************************************************************************
//  Function: ReAddToWindow
//
//  Purpose:
//    Removes, then adds the axes back into the window.  For Anti-aliasing
//    purposes, so that the axes is rendered after the plots.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 28, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  8 20:08:23 PDT 2003
//    Only re-add if antialising is turned on.
//
// ****************************************************************************

void
VisWinAxes3D::ReAddToWindow()
{
    if (mediator.GetAntialiasing())
    {
        RemoveAxes3DFromWindow();
        if (ShouldAddAxes3D())
        {
            AddAxes3DToWindow();
        }
    }
}


// ****************************************************************************
//  Function: SetLabelScaling
//
//  Purpose:
//    Sets the auto label scaling and label exponents for the axes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 16, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
VisWinAxes3D::SetLabelScaling(bool autoscale, int upowX, int upowY, int upowZ)
{
    axes->SetLabelScaling(autoscale, upowX, upowY, upowZ);
} 

// ****************************************************************************
// Method: SetTitle
//
// Purpose: 
//   Sets the X,Y,Z titles for the axes. If the userSet flag is true then we
//   override the titles that come from the database.
//
// Arguments:
//   title   : The new title.
//   userSet : Whether to use the new title.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 10:50:38 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes3D::SetXTitle(const std::string &title, bool userSet)
{
    userXTitle = title;
    userXTitleFlag = userSet;
}

void
VisWinAxes3D::SetYTitle(const std::string &title, bool userSet)
{
    userYTitle = title;
    userYTitleFlag = userSet;
}

void
VisWinAxes3D::SetZTitle(const std::string &title, bool userSet)
{
    userZTitle = title;
    userZTitleFlag = userSet;
}

// ****************************************************************************
// Method: SetTitle
//
// Purpose: 
//   Sets the X,Y,Z titles for the axes. If the userSet flag is true then we
//   override the titles that come from the database.
//
// Arguments:
//   title   : The new title.
//   userSet : Whether to use the new title.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 10:50:38 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
VisWinAxes3D::SetXUnits(const std::string &units, bool userSet)
{
    userXUnits = units;
    userXUnitsFlag = userSet;    
}

void
VisWinAxes3D::SetYUnits(const std::string &units, bool userSet)
{
    userYUnits = units;
    userYUnitsFlag = userSet;    
}

void
VisWinAxes3D::SetZUnits(const std::string &units, bool userSet)
{
    userZUnits = units;
    userZUnitsFlag = userSet;    
}
