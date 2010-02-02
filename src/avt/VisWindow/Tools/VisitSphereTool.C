/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <VisitSphereTool.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <avtVector.h>

#define SPHERE_SIZE 1.

int VisitSphereTool::activeResizeHotpoint = 0;

// ****************************************************************************
// Method: VisitSphereTool::VisitSphereTool
//
// Purpose: 
//   This is the constructor for the sphere tool.
//
// Arguments:
//   p : A reference to the tool proxy.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 16:59:02 PST 2002
//
// Modifications:
//
// ****************************************************************************

VisitSphereTool::VisitSphereTool(VisWindowToolProxy &p) :
    VisitInteractiveTool(p), Interface(p)
{
    HotPoint h;
    h.radius = 1./60.; // See what a good value is.
    h.tool = this;

    //
    // Add the sphere origin hotpoint.
    //
    h.pt = avtVector(0.,  0.,  0.);
    h.callback = TranslateCallback;
    origHotPoints.push_back(h);

    //
    // Add the first resize hotpoint.
    //
    h.pt = avtVector(SPHERE_SIZE,  0.,  0.);
    h.callback = ResizeCallback1;
    origHotPoints.push_back(h);

    //
    // Add the first resize hotpoint.
    //
    h.pt = avtVector(0., SPHERE_SIZE,  0.);
    h.callback = ResizeCallback2;
    origHotPoints.push_back(h);

    //
    // Add the first resize hotpoint.
    //
    h.pt = avtVector(0., 0., SPHERE_SIZE);
    h.callback = ResizeCallback3;
    origHotPoints.push_back(h);

    //
    // Set up some defaults for the sphere equation.
    //
    double bounds[6];
    proxy.GetBounds(bounds);
    double dXd2 = 0.5 * (bounds[1] - bounds[0]);
    double dYd2 = 0.5 * (bounds[3] - bounds[2]);
    double dZd2 = 0.5 * (bounds[5] - bounds[4]);
#define spMIN(A,B) (((A)<(B))?(A):(B))
    double rad = spMIN(spMIN(dXd2, dYd2), dZd2);
    Interface.SetOrigin(bounds[0] + dXd2,
                        bounds[2] + dYd2,
                        bounds[4] + dZd2);
    Interface.SetRadius(rad);

    addedOutline = false;
    addedBbox    = false;

    hotPoints = origHotPoints;
    CreateSphereActor();
    CreateTextActors();
}

// ****************************************************************************
// Method: VisitSphereTool::~VisitSphereTool
//
// Purpose: 
//   This is the destructor for the sphere tool class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

VisitSphereTool::~VisitSphereTool()
{
    if(sphereActor != NULL)
    {
        sphereActor->Delete();
        sphereActor = NULL;
    }

    if(sphereMapper != NULL)
    {
        sphereMapper->Delete();
        sphereMapper = NULL;
    }

    if(sphereData != NULL)
    {
        sphereData->Delete();
        sphereData = NULL;
    }

    // Delete the text mappers and actors
    DeleteTextActors();
}

// ****************************************************************************
// Method: VisitSphereTool::Enable
//
// Purpose: 
//   This method enables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitSphereTool::Enable()
{
    bool val = IsEnabled();
    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if(!val)
    {
        UpdateTool();
        proxy.GetCanvas()->AddActor(sphereActor);
        AddText();
    }
}

// ****************************************************************************
// Method: VisitSphereTool::Disable
//
// Purpose: 
//   This method disables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitSphereTool::Disable()
{
    bool val = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    if(val)
    {
        proxy.GetCanvas()->RemoveActor(sphereActor);
        RemoveText();
    }
}

// ****************************************************************************
// Method: VisitSphereTool::IsAvailable
//
// Purpose: 
//   Returns whether or not the tool is available for use.
//
// Returns:    Whether or not the tool is available for use.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 10:18:35 PDT 2002
//   I made the tool unavailable unless there are plots.
//
// ****************************************************************************

bool
VisitSphereTool::IsAvailable() const
{
    return (proxy.GetMode() == WINMODE_3D) && proxy.HasPlots();
}

// ****************************************************************************
// Method: VisitSphereTool::Start2DMode
//
// Purpose: 
//   This method switches the tool to 2D mode. In this case, the tool is
//   turned off.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitSphereTool::Start2DMode()
{
    Disable();
}

// ****************************************************************************
// Method: VisitSphereTool::Stop3DMode
//
// Purpose: 
//   This method tells the tool that 3D mode is stopping. The tool is disabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitSphereTool::Stop3DMode()
{
    Disable();
}

// ****************************************************************************
// Method: VisitSphereTool::SetForegroundColor
//
// Purpose: 
//   This method sets the tool's foreground color.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002 
//   Use vtkTextProperty to set actor color instead of vtkProperty.
//
// ****************************************************************************

void
VisitSphereTool::SetForegroundColor(double r, double g, double b)
{
    // Set the colors for the text actors.
    double color[3] = {r, g, b};
    sphereActor->GetProperty()->SetColor(color);
    sphereActor->GetProperty()->SetAmbient(1.);
    sphereActor->GetProperty()->SetDiffuse(1.);
    sphereActor->GetProperty()->SetSpecular(1.);
    originTextActor->GetTextProperty()->SetColor(color);
    for(int i = 0; i < 3; ++i)
        radiusTextActor[i]->GetTextProperty()->SetColor(color);
}

// ****************************************************************************
// Method: VisitSphereTool::UpdateTool
//
// Purpose: 
//   Repostions the tool using the attributes stored in the Interface.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitSphereTool::UpdateTool()
{
    TMtx.MakeTranslate(Interface.GetOrigin()[0],
                       Interface.GetOrigin()[1],
                       Interface.GetOrigin()[2]);
    SMtx.MakeScale(Interface.GetRadius());
    DoTransformations();
    UpdateText();
}

// ****************************************************************************
// Method: VisitSphereTool::UpdateView
//
// Purpose: 
//   Updates the location of the text when the view changes.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 11:56:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitSphereTool::UpdateView()
{ 
    if(IsEnabled())
        UpdateText();
}

// ****************************************************************************
// Method: VisitSphereTool::CreateSphereActor
//
// Purpose: 
//   Creates the vector actor.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 10:09:43 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Jul 26 10:52:09 PDT 2002    
//   Fix memory leak: Delete vtkMatrix4x4 m, register sphereData, 
//   delete source.
//   
//   Hank Childs, Thu Jun  8 11:27:37 PDT 2006
//   Fix compiler warning for casts.
//
// ****************************************************************************

void
VisitSphereTool::CreateSphereActor()
{
    // Store the colors and points in the polydata.
    vtkSphereSource *source = vtkSphereSource::New();
    source->SetCenter(0, 0, 0);
    source->SetRadius(SPHERE_SIZE);
    source->SetLatLongTessellation(1);
    source->SetPhiResolution(15);
    source->SetThetaResolution(15);
    sphereData = source->GetOutput();
    sphereData->Register(NULL);

    sphereMapper = vtkPolyDataMapper::New();
    sphereMapper->SetInput(sphereData);

    sphereActor = vtkActor::New();
    sphereActor->GetProperty()->SetRepresentationToWireframe();
    vtkMatrix4x4 *m = vtkMatrix4x4::New(); m->Identity();
    sphereActor->SetUserMatrix(m);
    sphereActor->SetMapper(sphereMapper);
    m->Delete();
    source->Delete();
}

// ****************************************************************************
// Method: VisitSphereTool::CreateTextActors
//
// Purpose: 
//   Create the text actors and mappers used to draw the origin/radius info.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 17:17:27 PST 2002
//
// Modifications:
//
//   Hank Childs, Tue May 21 13:37:44 PDT 2002
//   Removed configurable hooks to prevent actors from being instantiated.
//   This makes maintenance issues easier.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
// ****************************************************************************

void
VisitSphereTool::CreateTextActors()
{
    originTextActor = vtkTextActor::New();
    originTextActor->ScaledTextOff();

    for(int i = 0; i < 3; ++i)
    {
        radiusTextActor[i] = vtkTextActor::New();
        radiusTextActor[i]->ScaledTextOff();
    }
}

// ****************************************************************************
// Method: VisitSphereTool::DeleteTextActors
//
// Purpose: 
//   Deletes the text actors and mappers.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 17:16:39 PST 2002
//
// Modifications:
//
//   Hank Childs, Tue May 21 13:37:44 PDT 2002
//   Removed configurable hooks to prevent text actors from being deleted.
//   This makes maintenance issues easier.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   TextMappers no longer required.
//
// ****************************************************************************

void
VisitSphereTool::DeleteTextActors()
{
    if(originTextActor != NULL)
    {
        originTextActor->Delete();
        originTextActor = NULL;
    }

    for(int i = 0; i < 3; ++i)
    {
        if(radiusTextActor[i] != NULL)
        {
            radiusTextActor[i]->Delete();
            radiusTextActor[i] = NULL;
        }
    }
}

// ****************************************************************************
// Method: VisitSphereTool::AddText
//
// Purpose: 
//   Adds the text actors to the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 17:15:11 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:08:46 PDT 2002
//   Added upAxis actors.
//
// ****************************************************************************

void
VisitSphereTool::AddText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->AddActor2D(originTextActor);
    for(int i = 0; i < 3; ++i)
        proxy.GetForeground()->AddActor2D(radiusTextActor[i]);
#endif
}

// ****************************************************************************
// Method: VisitSphereTool::RemoveText
//
// Purpose: 
//   Removes the text actors from the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 17:15:11 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitSphereTool::RemoveText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->RemoveActor2D(originTextActor);
    for(int i = 0; i < 3; ++i)
        proxy.GetForeground()->RemoveActor2D(radiusTextActor[i]);
#endif
}

// ****************************************************************************
// Method: VisitSphereTool::UpdateText
//
// Purpose: 
//   Updates the info that the text actors display.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 17:15:11 PST 2002
//
// Modifications:
//
//   Hank Childs, Tue May 21 13:37:44 PDT 2002
//   Removed configurable hooks to prevent text actors from being updated.
//   This makes maintenance issues easier.
//
//   Hank Childs, Wed May 22 09:08:28 PDT 2002
//   Explicitly set the third value to be 0. for vtkCoordinate::SetValue calls.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace mapper with actor.
//
// ****************************************************************************

void
VisitSphereTool::UpdateText()
{
    char str[100];
    sprintf(str, "Origin <%1.3g %1.3g %1.3g>", hotPoints[0].pt.x,
            hotPoints[0].pt.y, hotPoints[0].pt.z);
    originTextActor->SetInput(str);
    avtVector originScreen = ComputeWorldToDisplay(hotPoints[0].pt);
    double pt[3] = {originScreen.x, originScreen.y, 0.};
    originTextActor->GetPositionCoordinate()->SetValue(pt);

    avtVector up(hotPoints[1].pt.x - hotPoints[0].pt.x,
                 hotPoints[1].pt.y - hotPoints[0].pt.y,
                 hotPoints[1].pt.z - hotPoints[0].pt.z);
    sprintf(str, "Radius = %1.3g", up.norm());
    for(int i = 0; i < 3; ++i)
    {
        radiusTextActor[i]->SetInput(str);
        avtVector radiusScreen = ComputeWorldToDisplay(hotPoints[i+1].pt);
        double pt2[3] = {radiusScreen.x, radiusScreen.y, 0.};
        radiusTextActor[i]->GetPositionCoordinate()->SetValue(pt2);
    }
}

// ****************************************************************************
// Method: VisitSphereTool::CallCallback
//
// Purpose: 
//   Lets the outside world know that the tool has a new slice plane.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 17:05:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitSphereTool::CallCallback()
{
    // Origin
    avtVector origin(hotPoints[0].pt);
   
    // Radius
    double radius = (hotPoints[1].pt - origin).norm();

    Interface.SetOrigin(origin.x, origin.y, origin.z);
    Interface.SetRadius(radius);
    Interface.ExecuteCallback();
}

// ****************************************************************************
// Method: VisitSphereTool::InitialActorSetup
//
// Purpose: 
//   Makes the text and outline actors active and starts bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 16:59:02 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Apr  3 11:01:17 PDT 2009
//   Don't do bounding box mode unless we have it selected.
//
// ****************************************************************************

void
VisitSphereTool::InitialActorSetup()
{
    // Enter bounding box mode if there are plots.
    if(proxy.GetBoundingBoxMode() && proxy.HasPlots())
    {
        addedBbox = true;
        proxy.StartBoundingBox();
    }
}

// ****************************************************************************
// Method: VisitSphereTool::FinalActorSetup
//
// Purpose: 
//   Removes certain actors from the renderer and ends bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 2 16:59:02 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  3 17:03:34 PST 2003 
//   If transparencies exist, have the plots recalculate render order, so
//   that this tool is rendered before the transparent actors. 
//
// ****************************************************************************

void
VisitSphereTool::FinalActorSetup()
{
    // End bounding box mode.
    if(addedBbox)
    {
        proxy.EndBoundingBox();
    }
    addedBbox = false;
    if (proxy.TransparenciesExist())
        proxy.RecalculateRenderOrder();
}

// ****************************************************************************
//  Method:  VisitSphereTool::DoTransformations
//
//  Purpose:
//    Applies the current transformation to the hotpoints.
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu May 2 17:11:06 PST 2002
//
//  Modifications:
//
// ****************************************************************************

void
VisitSphereTool::DoTransformations()
{
    avtMatrix M = TMtx * SMtx;

    for (int i=0; i<hotPoints.size(); i++)
        hotPoints[i].pt = M * origHotPoints[i].pt;

    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    tmp->DeepCopy(M.GetElements());
    sphereActor->SetUserMatrix(tmp);
    tmp->Delete();
}

// ****************************************************************************
//  Method:  VisitSphereTool::Translate
//
//  Purpose:
//    This is the handler method that is called when the translate hotpoint
//    is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri May 3 10:56:19 PDT 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Feb  2 13:18:23 EST 2010
//    Depending on the tool update mode, either call the callback 
//    continuously, or don't even call it upon the mouse release.
//
// ****************************************************************************

void
VisitSphereTool::Translate(CB_ENUM e, int, int, int x, int y)
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
        avtMatrix T(avtMatrix::CreateTranslate(motion));
        TMtx =  T * TMtx;
        DoTransformations();

        // Update the text and outline actors.
        UpdateText();

        // Render the window
        proxy.Render();

        if (proxy.GetToolUpdateMode() == UPDATE_CONTINUOUS)
            CallCallback();
    }
    else
    {
        // Call the tool's callback.
        if (proxy.GetToolUpdateMode() != UPDATE_ONCLOSE)
            CallCallback();

        // Remove the right actors.
        FinalActorSetup();
    }
}

// ****************************************************************************
//  Method:  VisitSphereTool::Resize
//
//  Purpose:
//    This is the handler method that is called when the resize hotpoint
//    is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu May 2 16:59:02 PST 2002
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 17:00:44 PDT 2007
//    Fix divide by zero.
//
//    Jeremy Meredith, Tue Feb  2 13:18:23 EST 2010
//    Depending on the tool update mode, either call the callback 
//    continuously, or don't even call it upon the mouse release.
//    Also, fixed inconsistencies in the way actors/bboxmode were set up.
//
// ****************************************************************************

void
VisitSphereTool::Resize(CB_ENUM e, int, int, int x, int y)
{
    HotPoint &origin = hotPoints[0];
    HotPoint &resize = hotPoints[activeResizeHotpoint];

    double dX, dY;
    avtVector originScreen, resizeScreen;

    if(e == CB_START)
    {
        originScreen = ComputeWorldToDisplay(origin.pt);
        resizeScreen = ComputeWorldToDisplay(resize.pt);

        dX = originScreen.x - resizeScreen.x;
        dY = originScreen.y - resizeScreen.y;
        originalDistance = sqrt(dX * dX + dY * dY);

        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        originScreen = ComputeWorldToDisplay(origin.pt);

        dX = originScreen.x - double(x);
        dY = originScreen.y - double(y);
        double dist = sqrt(dX * dX + dY * dY);
        if (originalDistance == 0.)
            originalDistance = dist;
        double scale = dist / originalDistance;

        // Do the hotpoint and actor transformations
        avtMatrix S(avtMatrix::CreateScale(scale));
        SMtx = S * SMtx;
        DoTransformations();

        // Save the distance for next time through.
        originalDistance = dist;

        // Render the window
        proxy.Render();

        if (proxy.GetToolUpdateMode() == UPDATE_CONTINUOUS)
            CallCallback();
    }
    else
    {
        // Call the tool's callback.
        if (proxy.GetToolUpdateMode() != UPDATE_ONCLOSE)
            CallCallback();

        FinalActorSetup();
    }
}


// ****************************************************************************
//  Method:  VisitSphereTool::ReAddToWindow
//
//  Purpose:
//    Allows the tool to re-add any actors affected by anti-aliasing to remove
//    and re-add themselves back to the renderer, so that they will be rendered
//    after plots.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    Wed May 28 16:09:47 PDT 2003 
//
//  Modifications:
//
// ****************************************************************************

void
VisitSphereTool::ReAddToWindow()
{
    if(IsEnabled())
    {
        proxy.GetCanvas()->RemoveActor(sphereActor);
        proxy.GetCanvas()->AddActor(sphereActor);
    }
}


//
// Static callback functions.
//

void
VisitSphereTool::TranslateCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y, int)
{
    VisitSphereTool *pt = (VisitSphereTool *)it;
    pt->Translate(e, ctrl, shift, x, y);
}

void
VisitSphereTool::ResizeCallback1(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y, int)
{
    VisitSphereTool *pt = (VisitSphereTool *)it;
    activeResizeHotpoint = 1;
    pt->Resize(e, ctrl, shift, x, y);
}

void
VisitSphereTool::ResizeCallback2(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y, int)
{
    VisitSphereTool *pt = (VisitSphereTool *)it;
    activeResizeHotpoint = 2;
    pt->Resize(e, ctrl, shift, x, y);
}

void
VisitSphereTool::ResizeCallback3(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y, int)
{
    VisitSphereTool *pt = (VisitSphereTool *)it;
    activeResizeHotpoint = 3;
    pt->Resize(e, ctrl, shift, x, y);
}
