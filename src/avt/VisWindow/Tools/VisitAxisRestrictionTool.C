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

#include <math.h>
#include <VisitAxisRestrictionTool.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <avtExtents.h>
#include <avtVector.h>

using std::vector;
using std::string;

const float VisitAxisRestrictionTool::radius = 1/60.;

// ****************************************************************************
// Method: VisitAxisRestrictionTool::VisitAxisRestrictionTool
//
// Purpose: 
//   This is the constructor for the sphere tool.
//
// Arguments:
//   p : A reference to the tool proxy.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

VisitAxisRestrictionTool::VisitAxisRestrictionTool(VisWindowToolProxy &p) :
    VisitInteractiveTool(p), Interface(p)
{
    Interface.ResetNumberOfAxes(0);

    hotPoints = origHotPoints;

    addedBbox = false;
    textAdded = false;
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::~VisitAxisRestrictionTool
//
// Purpose: 
//   This is the destructor for the sphere tool class.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//   
// ****************************************************************************

VisitAxisRestrictionTool::~VisitAxisRestrictionTool()
{
    // Delete the text mappers and actors
    DeleteTextActors();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::Enable
//
// Purpose: 
//   This method enables the tool.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::Enable()
{
    bool val = IsEnabled();
    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if(!val)
    {
        UpdateTool();
    }

    AddText();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::Disable
//
// Purpose: 
//   This method disables the tool.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::Disable()
{
    bool val = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    RemoveText();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::IsAvailable
//
// Purpose: 
//   Returns whether or not the tool is available for use.
//
// Returns:    Whether or not the tool is available for use.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

bool
VisitAxisRestrictionTool::IsAvailable() const
{
    return (proxy.GetMode() == WINMODE_AXISARRAY) && proxy.HasPlots();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::StopAxisArrayMode
//
// Purpose: 
//   This method tells the tool that 3D mode is stopping. The tool is disabled.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisitAxisRestrictionTool::StopAxisArrayMode()
{
    Disable();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::SetForegroundColor
//
// Purpose: 
//   This method sets the tool's foreground color.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::SetForegroundColor(double r, double g, double b)
{
    // Set the colors for the text actors.
    color[0] = r;
    color[1] = g;
    color[2] = b;
    for(int i = 0; i < posTextActors.size(); ++i)
        posTextActors[i]->GetTextProperty()->SetColor(color);
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::UpdateTool
//
// Purpose: 
//   Repostions the tool using the attributes stored in the Interface.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisitAxisRestrictionTool::UpdateTool()
{
    DoClampAndTransformations();
    UpdateText();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::UpdateView
//
// Purpose: 
//   Updates the location of the text when the view changes.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisitAxisRestrictionTool::UpdateView()
{ 
    if (IsEnabled())
    {
        UpdateText();

        if (proxy.HasPlots())
            AddText();
        else
            RemoveText();
    }
    else
        RemoveText();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::CreateTextActors
//
// Purpose: 
//   Create the text actors and mappers used to draw the origin/radius info.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::CreateTextActors()
{
    posTextActors.resize(origHotPoints.size());
    for(int i = 0; i < posTextActors.size(); ++i)
    {
        posTextActors[i] = vtkTextActor::New();
        posTextActors[i]->ScaledTextOff();
        posTextActors[i]->GetTextProperty()->SetColor(color);
    }
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::DeleteTextActors
//
// Purpose: 
//   Deletes the text actors and mappers.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::DeleteTextActors()
{
    RemoveText();
    for(int i = 0; i < posTextActors.size(); ++i)
    {
        if (posTextActors[i] != NULL)
        {
            posTextActors[i]->Delete();
            posTextActors[i] = NULL;
        }
    }
    posTextActors.clear();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::AddText
//
// Purpose: 
//   Adds the text actors to the foreground canvas.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::AddText()
{
    if (textAdded)
        return;

#ifndef NO_ANNOTATIONS
    for(int i = 0; i < posTextActors.size(); ++i)
    {
        proxy.GetForeground()->AddActor2D(posTextActors[i]);
    }
#endif

    textAdded = true;
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::RemoveText
//
// Purpose: 
//   Removes the text actors from the foreground canvas.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::RemoveText()
{
    if (!textAdded)
        return;

#ifndef NO_ANNOTATIONS
    for(int i = 0; i < posTextActors.size(); ++i)
    {
        proxy.GetForeground()->RemoveActor2D(posTextActors[i]);
    }
#endif

    textAdded = false;
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::UpdateText
//
// Purpose: 
//   Updates the info that the text actors display.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::UpdateText()
{
    for (int i=0; i<posTextActors.size(); i++)
    {
        int axis = int(i/2);
        char str[100];
        sprintf(str, "<%1.5g>",
                origHotPoints[i].pt.y * (axesMax[axis]-axesMin[axis])
                + axesMin[axis]);
        posTextActors[i]->SetInput(str);

        avtVector posScreen = ComputeWorldToDisplay(hotPoints[i].pt);
        double pt[3] = {posScreen.x, posScreen.y, 0.};
        posTextActors[i]->GetPositionCoordinate()->SetValue(pt);
    }
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::CallCallback
//
// Purpose: 
//   Lets the outside world know that the tool has a new slice plane.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//    Jeremy Meredith, Fri Feb  8 16:22:03 EST 2008
//    Set the min/max to something that's obviously supposed to be a semantic
//    min/max limit if the hotpoints are all the way at the end of the axis.
//   
//    Jeremy Meredith, Fri Feb 15 13:21:20 EST 2008
//    Added axis names to the axis restriction tool.
//
// ****************************************************************************

void
VisitAxisRestrictionTool::CallCallback()
{
    Interface.ResetNumberOfAxes(axesMin.size());
    for (int ax=0; ax<axesMin.size(); ax++)
    {
        Interface.SetAxisName(ax, axesNames[ax]);

        if (origHotPoints[ax*2+1].pt.y <= 0)
        {
            Interface.SetAxisMin(ax, -1e+37);
        }
        else
        {
            float minval = origHotPoints[ax*2+1].pt.y *
                           (axesMax[ax]-axesMin[ax]) + axesMin[ax];
            Interface.SetAxisMin(ax, minval);
        }

        if (origHotPoints[ax*2+0].pt.y >= 1)
        {
            Interface.SetAxisMax(ax, +1e+37);
        }
        else
        {
            float maxval = origHotPoints[ax*2+0].pt.y *
                           (axesMax[ax]-axesMin[ax]) + axesMin[ax];
            Interface.SetAxisMax(ax, maxval);
        }
    }

    Interface.ExecuteCallback();
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::InitialActorSetup
//
// Purpose: 
//   Makes the text and outline actors active and starts bounding box mode.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::InitialActorSetup()
{
    // Enter bounding box mode if there are plots.
    if(proxy.HasPlots())
    {
        addedBbox = true;
        //proxy.StartBoundingBox();
    }
}

// ****************************************************************************
// Method: VisitAxisRestrictionTool::FinalActorSetup
//
// Purpose: 
//   Removes certain actors from the renderer and ends bounding box mode.
//
// Programmer: Jeremy Meredith
// Creation:   February  1, 2008
//
// Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::FinalActorSetup()
{
    // End bounding box mode.
    if(addedBbox)
    {
        //proxy.EndBoundingBox();
    }
    addedBbox = false;
    if (proxy.TransparenciesExist())
        proxy.RecalculateRenderOrder();
}

// ****************************************************************************
//  Method:  VisitAxisRestrictionTool::DoClampAndTransformations
//
//  Purpose:
//    Applies the current transformation to the hotpoints.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  1, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::DoClampAndTransformations()
{
    for (int i=0; i<hotPoints.size(); i++)
    {
        if (origHotPoints[i].pt.y < 0)
            origHotPoints[i].pt.y = 0;
        if (origHotPoints[i].pt.y > 1)
            origHotPoints[i].pt.y = 1;

        hotPoints[i] = origHotPoints[i];
        if (proxy.GetFullFrameMode())
        {
            // 
            // Translate the hotPoints so they appear in the correct position
            // in full-frame mode. 
            // 
            double scale;
            int type;
            proxy.GetScaleFactorAndType(scale, type);
            if (type == 0 ) // x_axis
            {
                hotPoints[i].pt.x *= scale;
            }
            else            // x_axis
            {
                hotPoints[i].pt.y *= scale;
            }
        }
    }
}

// ****************************************************************************
//  Method:  VisitAxisRestrictionTool::Translate
//
//  Purpose:
//    This is the handler method that is called when the translate hotpoint
//    is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//    index : the index of the hotpoint that was triggered
//
//  Programmer: Jeremy Meredith
//  Creation:   February  1, 2008
//
// ****************************************************************************

void
VisitAxisRestrictionTool::Move(CB_ENUM e, int, int, int x, int y, int index)
{
    if(e == CB_START)
    {
        vtkRenderer *ren = proxy.GetCanvas();
        vtkCamera *camera = ren->GetActiveCamera();
        double ViewFocus[3];
        camera->GetFocalPoint(ViewFocus);
        ComputeWorldToDisplay(ViewFocus[0], ViewFocus[1],
                              ViewFocus[2], ViewFocus);
        // Store the focal depth.
        focalDepth = ViewFocus[2];

        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        avtVector newPoint = ComputeDisplayToWorld(avtVector(x,y,focalDepth));
        //
        // Have to recalculate the old mouse point since the viewport has
        // moved, so we can't move it outside the loop
        //
        avtVector oldPoint = ComputeDisplayToWorld(avtVector(lastX,lastY,focalDepth));

        avtVector motion = newPoint - oldPoint;

        if (proxy.GetFullFrameMode())
        {
            double scale;
            int type;
            proxy.GetScaleFactorAndType(scale, type);
            if (type == 1)
            {
                motion.y /= scale;
            }
        }

        origHotPoints[index].pt.y += motion.y;
        DoClampAndTransformations();

        // Update the text and outline actors.
        UpdateText();

        // Render the window
        proxy.Render();
    }
    else
    {
        RemoveText();

        // Call the tool's callback.
        CallCallback();

        // Remove the right actors.
        FinalActorSetup();

        AddText();
        proxy.Render();
    }
}

// ****************************************************************************
//  Method:  VisitAxisRestrictionTool::ReAddToWindow
//
//  Purpose:
//    Allows the tool to re-add any actors affected by anti-aliasing to remove
//    and re-add themselves back to the renderer, so that they will be rendered
//    after plots.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  1, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::ReAddToWindow()
{
    if (IsEnabled())
    {
        // nothing to do....
    }
}


//
// Static callback functions.
//

void
VisitAxisRestrictionTool::MoveCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y, int index)
{
    VisitAxisRestrictionTool *pt = (VisitAxisRestrictionTool *)it;
    pt->Move(e, ctrl, shift, x, y, index);
}


// ****************************************************************************
//  Method:  VisitAxisRestrictionTool::UpdatePlotList
//
//  Purpose:
//    This is where we figure out the extents of each of our axes,
//    create hotpoints and text for them.
//
//  Arguments:
//    list       the plot list
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  1, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:59:55 EST 2008
//    Added support for array variables and bin-defined x positions.
//
//    Jeremy Meredith, Fri Feb 15 13:21:20 EST 2008
//    Added axis names to the axis restriction tool.
//
// ****************************************************************************
void
VisitAxisRestrictionTool::UpdatePlotList(std::vector<avtActor_p> &list)
{
    int nActors = list.size();

    int arrayActor = -1;
    int arrayIndex = -1;

    // Find the highest-valued axis index for any variable
    int naxes = 0;
    for (int i = 0 ; i < nActors ; i++)
    {
        avtDataAttributes &atts = 
            list[i]->GetBehavior()->GetInfo().GetAttributes();
        int nvars = atts.GetNumberOfVariables();
        for (int j = 0 ; j < nvars ; j++)
        {
            const char *var = atts.GetVariableName(j).c_str();
            if (atts.GetVariableType(var) == AVT_ARRAY_VAR)
            {
                naxes = atts.GetVariableDimension(var);
                arrayActor = i;
                arrayIndex = j;
                break;
            }
            int axis = atts.GetUseForAxis(var);
            if (axis == -1)
                continue;
            naxes = (axis+1) > naxes ? (axis+1) : naxes;
        }
    }

    // create the new axes limits
    axesNames.resize(naxes);
    axesXPos.resize(naxes);
    axesMin.resize(naxes);
    axesMax.resize(naxes);

    if (arrayActor>=0)
    {
        avtDataAttributes &atts = 
            list[arrayActor]->GetBehavior()->GetInfo().GetAttributes();
        const char *var = atts.GetVariableName(arrayIndex).c_str();
        int dim = atts.GetVariableDimension(var);
        avtExtents *e = atts.GetVariableComponentExtents(var);
        const vector<double> &bins = atts.GetVariableBinRanges(var);
        const vector<string> &subnames = atts.GetVariableSubnames(var);
        if (!e || !e->HasExtents())
        {
            char str[100];
            sprintf(str, "Did not have valid extents for var '%s'", var);
            EXCEPTION1(ImproperUseException, str);
        }
        double *extents = new double[2*dim];
        e->CopyTo(extents);
        for (int k=0; k<dim; k++)
        {
            if (bins.size() > k)
                axesXPos[k] = (bins[k]+bins[k+1])/2;
            else
                axesXPos[k] = k;
            axesMin[k] = extents[2*k+0];
            axesMax[k] = extents[2*k+1];
            axesNames[k] = subnames[k];
        }
        delete[] extents;
    }
    else
    {
        for (int i = 0 ; i < nActors ; i++)
        {
            avtDataAttributes &atts = 
                list[i]->GetBehavior()->GetInfo().GetAttributes();
            int nvars = atts.GetNumberOfVariables();
            for (int j = 0 ; j < nvars ; j++)
            {
                const char *var = atts.GetVariableName(j).c_str();
                int axis = atts.GetUseForAxis(var);
                if (axis == -1)
                    continue;
                avtExtents *ext = atts.GetCumulativeTrueDataExtents(var);
                if (!ext)
                {
                    char str[100];
                    sprintf(str, "Did not have valid extents for var '%s'", var);
                    EXCEPTION1(ImproperUseException, str);
                }
                double extents[2];
                atts.GetCumulativeTrueDataExtents(var)->CopyTo(extents);
                axesXPos[axis] = axis;
                axesMin[axis] = extents[0];
                axesMax[axis] = extents[1];
                axesNames[axis] = var;
            }
        }
    }

    // we know how many axes there are.  Create the new hotpoints
    // based on the saved values in the interface
    origHotPoints.clear();
    for (int i=0; i<naxes; i++)
    {
        float maxval = Interface.GetAxisMax(i);
        float minval = Interface.GetAxisMin(i);

        HotPoint h;
        h.radius = radius;
        h.tool = this;
        h.callback = MoveCallback;
        h.pt = avtVector(axesXPos[i],1.,0);

        h.data = origHotPoints.size();
        h.shape = 1;
        h.pt.y = (maxval-axesMin[i])/(axesMax[i]-axesMin[i]);
        origHotPoints.push_back(h);

        h.data = origHotPoints.size();
        h.shape = 2;
        h.pt.y = (minval-axesMin[i])/(axesMax[i]-axesMin[i]);
        origHotPoints.push_back(h);        
    }
    hotPoints = origHotPoints;

    // Set the real coords, and make sure they're within range
    DoClampAndTransformations();

    // create the new text actors (must be done after we know how many
    // axes there are)
    DeleteTextActors();
    CreateTextActors();
}

// ****************************************************************************
//  Method:  VisitAxisRestrictionTool::FullFrameOn
//
//  Purpose: Updates the tool.
//
//  Arguments:
//    <unused>   The axis scale factor.
//    <unused>   The axis scale type.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  1, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::FullFrameOn(const double, const int)
{
    if (IsEnabled())
    {
        UpdateTool();
    }
}


// ****************************************************************************
//  Method:  VisitAxisRestrictionTool::FullFrameOn
//
//  Purpose: Updates the tool.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  1, 2008
//
//  Modifications:
//
// ****************************************************************************

void
VisitAxisRestrictionTool::FullFrameOff()
{
    if (IsEnabled())
    {
        UpdateTool();
    }
}
