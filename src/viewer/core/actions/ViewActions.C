/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <ViewActions.h>
#include <ViewAttributes.h>
#include <ViewerMessaging.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <snprintf.h>

#include <avtViewCurve.h>
#include <avtView2D.h>
#include <avtView3D.h>

#include <DataNode.h>
#include <DebugStream.h>
#include <PickAttributes.h>

///////////////////////////////////////////////////////////////////////////////

TogglePerspectiveViewAction::TogglePerspectiveViewAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
TogglePerspectiveViewAction::Execute()
{
    windowMgr->TogglePerspective(window->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

ResetViewAction::ResetViewAction(ViewerWindow *win) : ViewerActionLogic(win)
{
}

void
ResetViewAction::Execute()
{
    windowMgr->ResetView(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

RecenterViewAction::RecenterViewAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
RecenterViewAction::Execute()
{
    windowMgr->RecenterView(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

UndoViewAction::UndoViewAction(ViewerWindow *win) : ViewerActionLogic(win)
{
}

void
UndoViewAction::Execute()
{
    windowMgr->UndoView(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

RedoViewAction::RedoViewAction(ViewerWindow *win) : ViewerActionLogic(win)
{
}

void
RedoViewAction::Execute()
{
    windowMgr->RedoView(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

ToggleFullFrameAction::ToggleFullFrameAction(ViewerWindow *win) : 
    ViewerActionLogic(win)
{
}

void
ToggleFullFrameAction::Execute()
{
    windowMgr->ToggleFullFrameMode(window->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

const int SaveViewAction::MAX_SAVED_VIEWS = 15;
const int SaveViewAction::VIEWCurve = 1;
const int SaveViewAction::VIEW2D = 2;
const int SaveViewAction::VIEW3D = 3;

// ****************************************************************************
// Method: SaveViewAction::SaveViewAction
//
// Purpose: 
//   Constructor for the SaveViewAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:26 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:49:16 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

SaveViewAction::SaveViewAction(ViewerWindow *win) : ViewerActionLogic(win),
    activeChoice(0), views()
{
}

// ****************************************************************************
// Method: SaveViewAction::~SaveViewAction
//
// Purpose: 
//   Destructor for the SaveViewAction class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:31 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SaveViewAction::~SaveViewAction()
{
    DeleteViews();
}

// ****************************************************************************
// Method: SaveViewAction::DeleteViews
//
// Purpose: 
//   Deletes the views in the view vector.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:37 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Feb 27 14:37:03 PST 2003
//   Added VIEWCurve.
//
// ****************************************************************************

void
SaveViewAction::DeleteViews()
{
    // Delete the views
    for(size_t i = 0; i < views.size(); ++i)
    {
        if(views[i].viewType == VIEWCurve)
        {
            avtViewCurve *v = (avtViewCurve *)views[i].view;
            delete v;
        }
        else if(views[i].viewType == VIEW2D)
        {
            avtView2D *v = (avtView2D *)views[i].view;
            delete v;
        }
        else
        {
            avtView3D *v = (avtView3D *)views[i].view;
            delete v;
        }
    }

    views.clear();
}

int
SaveViewAction::NumSavedViews() const
{
    return (int)views.size();
}

void
SaveViewAction::Setup(int activeAction, bool toggled)
{
    activeChoice = activeAction;
}

// ****************************************************************************
// Method: SaveViewAction::Execute
//
// Purpose: 
//   Executes the save view action.
//
// Arguments:
//   val : The choice to execute.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:39 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Feb 27 14:41:35 PST 2003
//   I added support for curve views.
//
//   Brad Whitlock, Thu Aug 14 16:13:03 PST 2003
//   I moved code into DeleteViewsFromInterface.
//
// ****************************************************************************

void
SaveViewAction::Execute()
{
    if(activeChoice == 0)
    {
        DeleteViews();
    }
    else if(activeChoice == 1)
    {
        // Save the current view.
        SaveCurrentView();
    }
    else
    {
        // Use a saved view.
        UseSavedView(activeChoice - 2);
    }
}

// ****************************************************************************
// Method: SaveViewAction::CopyFrom
//
// Purpose: 
//   Copies attributes from another SaveViewAction.
//
// Arguments:
//   obj : The action from which to copy attributes.
//
// Returns:    True if the views were copied; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 27 15:28:11 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu May 22 14:27:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

bool
SaveViewAction::CopyFrom(const ViewerActionLogic *obj)
{
    bool retval = false;

    if(GetName() == obj->GetName())
    {
        const SaveViewAction *saveView = (const SaveViewAction *)obj;

        // Delete the views from the interface.
        DeleteViews();

        for(size_t i = 0; i < saveView->views.size(); ++i)
        {
            void *newView = 0;

            if(saveView->views[i].viewType == VIEWCurve)
            {
                avtViewCurve *v = (avtViewCurve *)saveView->views[i].view;
                newView = (void *) new avtViewCurve(*v);
            }
            else if(saveView->views[i].viewType == VIEW2D)
            {
                avtView2D *v = (avtView2D *)saveView->views[i].view;
                newView = (void *) new avtView2D(*v);
            }
            else
            {
                avtView3D *v = (avtView3D *)saveView->views[i].view;
                newView = (void *) new avtView3D(*v);
            }

            if(newView)
            {
                AddNewView(newView, saveView->views[i].viewType);
                retval = true;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: SaveViewAction::SaveCurrentView
//
// Purpose: 
//   Saves the current view so we can use it later.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 15:13:47 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 1 14:50:11 PST 2003
//   I moved some code into AddNewView.
//
//   Eric Brugger, Wed Aug 20 10:56:24 PDT 2003
//   I Modified the routine to use the window mode to determine the type of
//   view to save.
//
//   Brad Whitlock, Tue Apr 29 11:50:01 PDT 2008
//   Added tr()
//
// ****************************************************************************

void
SaveViewAction::SaveCurrentView()
{
    if(views.size() < (size_t)MAX_SAVED_VIEWS)
    {
        void *saveView;
        int vt;

        // Create storage for the view based on the window type and dimension.
        if(window->GetWindowMode() == WINMODE_CURVE)
        {
            avtViewCurve *v = new avtViewCurve;
            *v = window->GetViewCurve();
            vt = VIEWCurve;
            saveView = (void *)v;
        }
        else if(window->GetWindowMode() == WINMODE_2D)
        {
            avtView2D *v = new avtView2D;
            *v = window->GetView2D();
            vt = VIEW2D;
            saveView = (void *)v;
        }
        else
        {
            avtView3D *v = new avtView3D;
            *v = window->GetView3D();
            vt = VIEW3D;
            saveView = (void *)v;
        }

        // Add the new view to the list of saved views and make a new
        // icon as necessary.
        AddNewView(saveView, vt);
    }
    else
    {
        GetViewerMessaging()->Warning(TR("You cannot save more than 15 views."));
    }
}

// ****************************************************************************
// Method: SaveViewAction::AddNewView
//
// Purpose: 
//   Saves the view in the views vector and creates a new choice for the user
//   in the menus.
//
// Arguments:
//   v  : A void pointer to the view.
//   vt : The type of view.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 14:51:22 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:50:45 PDT 2008
//   Added tr().
//
//   Jeremy Meredith, Thu Aug  7 14:56:48 EDT 2008
//   Use %ld for long values.
//
// ****************************************************************************

void
SaveViewAction::AddNewView(void *v, int vt)
{
    //
    // Save the view.
    //
    ViewInfo info;
    info.view = (void *)v;
    info.viewType = vt;
    views.push_back(info);
}

// ****************************************************************************
// Method: SaveViewAction::UseSavedView
//
// Purpose: 
//   Uses the numbered saved view.
//
// Arguments:
//   index : The index of the saved view that we want to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 15:10:00 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 10:56:24 PDT 2003
//   I modified the routine to use the window mode to determine if the saved
//   view is appropriate.  I changed the call to UpdateViewAtts.
//
//   Brad Whitlock, Tue Apr 29 11:51:55 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
SaveViewAction::UseSavedView(int index)
{
    if(index >= 0 && (size_t)index < views.size())
    {
        if(views[index].viewType == VIEWCurve)
        {
            if(window->GetWindowMode() == WINMODE_CURVE)
            {
                window->SetViewCurve(*((avtViewCurve *)views[index].view));
            }
            else
            {
                GetViewerMessaging()->Error(
                    TR("VisIt cannot use saved view %1 because "
                       "it is a curve view and the window does not "
                       "contain curves.").
                    arg(index + 1));
            }
        }
        else if(views[index].viewType == VIEW2D)
        {
            if(window->GetWindowMode() == WINMODE_2D)
            {
                window->SetView2D(*((avtView2D *)views[index].view));
                windowMgr->UpdateViewAtts(window->GetWindowId(),
                                          false, true, false);
            }
            else
            {
                GetViewerMessaging()->Error(
                    TR("VisIt cannot use saved view %1 because "
                       "it is a 2D view and the window does not "
                       "contain 2D plots.").
                    arg(index + 1));
            }
        }
        else
        {
            if(window->GetWindowMode() == WINMODE_3D)
            {
                window->SetView3D(*((avtView3D *)views[index].view));
                windowMgr->UpdateViewAtts(window->GetWindowId(),
                                          false, false, true);
            }
            else
            {
                GetViewerMessaging()->Error(
                    TR("VisIt cannot use saved view %1 because "
                       "it is a 3D view and the window does not "
                       "contain 3D plots.").
                    arg(index + 1));
            }
        }
    }
}

// ****************************************************************************
// Method: SaveViewAction::CreateNode
//
// Purpose: 
//   Lets the action save its views for use in a future session.
//
// Arguments:
//   parentNode : The node to which we're adding.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 14:26:23 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 10:56:24 PDT 2003
//   I modified the routine to write a ViewCurveAttribute when writing a curve
//   view.  I split the view attributes into 2d and 3d parts.  I added code
//   to delete any existing views before adding the new ones.
//
//   Brad Whitlock, Thu Oct 23 14:24:10 PST 2003
//   I removed inappropriate code that deleted the views before trying to
//   save them.
//
//   Brad Whitlock, Thu Dec 18 13:35:10 PST 2003
//   Added another argument to the view objects's CreateNode methods.
//
//   Brad Whitlock, Tue Jan 17 10:52:08 PDT 2006
//   I made the saved views contain all of their fields no matter what.
//
// ****************************************************************************

bool
SaveViewAction::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return false;

    DataNode *saveviewNode = new DataNode("SaveViewAction");
    bool haveViews = false;

    // Add a node for each view that is not the default view.
    intVector viewTypes;
    for(size_t i = 0; i < views.size(); ++i)
    {
        if(views[i].viewType == VIEWCurve)
        {
            ViewCurveAttributes viewAtts;

            avtViewCurve *v = (avtViewCurve *)views[i].view;
            v->SetToViewCurveAttributes(&viewAtts);

            if(viewAtts.CreateNode(saveviewNode, true, false))
            {
                haveViews = true;
                viewTypes.push_back(views[i].viewType);
            }
        }
        else if(views[i].viewType == VIEW2D)
        {
            View2DAttributes viewAtts;

            avtView2D *v = (avtView2D *)views[i].view;
            v->SetToView2DAttributes(&viewAtts);

            if(viewAtts.CreateNode(saveviewNode, true, false))
            {
                haveViews = true;
                viewTypes.push_back(views[i].viewType);
            }
        }
        else if(views[i].viewType == VIEW3D)
        {
            View3DAttributes viewAtts;

            avtView3D *v = (avtView3D *)views[i].view;
            v->SetToView3DAttributes(&viewAtts);

            if(viewAtts.CreateNode(saveviewNode, true, false))
            {
                haveViews = true;
                viewTypes.push_back(views[i].viewType);
            }
        }
    }

    if(haveViews)
    {
        saveviewNode->AddNode(new DataNode("viewTypes", viewTypes));
        parentNode->AddNode(saveviewNode);
    }
    else
        delete saveviewNode;

    return haveViews;
}

// ****************************************************************************
// Method: SaveViewAction::SetFromNode
//
// Purpose: 
//   Reads in the saved views from the config.
//
// Arguments:
//   parentNode : The node that we're looking at for the config.
//   configVersion : The version from the config file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 14:58:58 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 10:56:24 PDT 2003
//   I modified the routine to read a ViewCurveAttribute when reading a curve
//   view.  I split the view attributes into 2d and 3d parts.
//
//   Brad Whitlock, Tue Aug 12 11:31:04 PDT 2003
//   I made it clear out views before reading them back in so we don't get
//   an ever-increasing number of views.
//
//   Hank Childs, Wed Oct 15 12:58:19 PDT 2003
//   Copy over eye angle.
//
//   Brad Whitlock, Wed Feb 13 14:20:21 PST 2008
//   Added configVersion to satisfy new interface.
//
// ****************************************************************************

void
SaveViewAction::SetFromNode(DataNode *parentNode, 
    const std::string &configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *saveviewNode = parentNode->GetNode("SaveViewAction");
    if(saveviewNode == 0)
        return;

    DataNode *viewTypesNode = saveviewNode->GetNode("viewTypes");
    if(viewTypesNode == 0)
        return;

    if(!views.empty())
    {
        DeleteViews();
    }

    const intVector &viewTypes = viewTypesNode->AsIntVector();
    DataNode **views = saveviewNode->GetChildren();
    int index = 0;
    for(int i = 0; i < saveviewNode->GetNumChildren(); ++i)
    {
        if(views[i]->GetKey() == "ViewCurveAttributes")
        {
            // Read the view from the config node.
            ViewCurveAttributes viewAtts;
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
            viewAtts.SetFromNode(views[i]);

            // Create the curve view.
            avtViewCurve *v = new avtViewCurve;
            v->SetFromViewCurveAttributes(&viewAtts);

            // Add it to the saved views.
            AddNewView((void *)v, VIEWCurve);

            ++index;
        }
        else if(views[i]->GetKey() == "View2DAttributes")
        {
            // Read the view from the config node.
            View2DAttributes viewAtts;
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
            viewAtts.SetFromNode(views[i]);

            // Create the curve view.
            avtView2D *v = new avtView2D;
            v->SetFromView2DAttributes(&viewAtts);

            // Add it to the saved views.
            AddNewView((void *)v, VIEW2D);

            ++index;
        }
        else if(views[i]->GetKey() == "View3DAttributes")
        {
            // Read the view from the config node.
            View3DAttributes viewAtts;
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
            viewAtts.SetFromNode(views[i]);

            // Create the curve view.
            avtView3D *v = new avtView3D;
            v->SetFromView3DAttributes(&viewAtts);

            // Add it to the saved views.
            AddNewView((void *)v, VIEW3D);

            ++index;
        }
        else if(views[i]->GetKey() == "ViewAttributes")
        {
            //
            // This coding is for reading saved views from release 1.2 and
            // earlier.  It should be removed in release 1.3.
            //
            // Read the view from the config node.
            ViewAttributes viewAtts;
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
            viewAtts.SetFromNode(views[i]);

            // Use the information to create the appropriate view type.
            void *viewPtr = 0;
            if(viewTypes[index] == VIEW2D)
            {
                avtView2D *v = new avtView2D;
                for(int i = 0; i < 4; ++i)
                {
                    v->viewport[i] = viewAtts.GetViewportCoords()[i];
                    v->window[i] = viewAtts.GetWindowCoords()[i];
                }
                viewPtr = (void *)v;
            }
            else if(viewTypes[index] == VIEW3D)
            {
                avtView3D *v = new avtView3D;
                for(int i = 0; i < 3; ++i)
                {
                    v->normal[i] = viewAtts.GetViewNormal()[i];
                    v->focus[i]  = viewAtts.GetFocus()[i];
                    v->viewUp[i] = viewAtts.GetViewUp()[i];
                }

                v->viewAngle = viewAtts.GetViewAngle();
                v->parallelScale = viewAtts.GetParallelScale();
                v->eyeAngle = viewAtts.GetEyeAngle();
                v->nearPlane = viewAtts.GetNearPlane();
                v->farPlane = viewAtts.GetFarPlane();
                v->imagePan[0] = viewAtts.GetImagePan()[0];
                v->imagePan[1] = viewAtts.GetImagePan()[1];
                v->imageZoom = viewAtts.GetImageZoom();
                v->perspective = viewAtts.GetPerspective();
                viewPtr = (void *)v;
            }

            // If we created a new view, add it to the saved views.
            if(viewPtr)
            {
                AddNewView(viewPtr, viewTypes[index]);
            }

            ++index;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetCenterOfRotationAction::SetCenterOfRotationAction
//
// Purpose: 
//   Constructor for the SetCenterOfRotationAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:03:40 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

SetCenterOfRotationAction::SetCenterOfRotationAction(ViewerWindow *win) : 
    ViewerActionLogic(win)
{
}

// ****************************************************************************
// Method: SetCenterOfRotationAction::~SetCenterOfRotationAction
//
// Purpose: 
//   Destructor for the SetCenterOfRotationAction class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:04:08 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

SetCenterOfRotationAction::~SetCenterOfRotationAction()
{
}

// ****************************************************************************
// Method: SetCenterOfRotationAction::Execute
//
// Purpose: 
//   Executes the SetCenterOfRotation RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:04:38 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
SetCenterOfRotationAction::Execute()
{
    const double *pt = args.GetQueryPoint1();
    windowMgr->SetCenterOfRotation(window->GetWindowId(), pt[0], pt[1], pt[2]);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ChooseCenterOfRotationAction::ChooseCenterOfRotationAction
//
// Purpose: 
//   Constructor for the ChooseCenterOfRotationAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:05:39 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

ChooseCenterOfRotationAction::ChooseCenterOfRotationAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

// ****************************************************************************
// Method: ChooseCenterOfRotationAction::~ChooseCenterOfRotationAction
//
// Purpose: 
//   Destructor for the ChooseCenterOfRotationAction class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:06:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

ChooseCenterOfRotationAction::~ChooseCenterOfRotationAction()
{
}

// ****************************************************************************
// Method: ChooseCenterOfRotation::Setup
//
// Purpose: 
//   Called when the action is activated by button clicking. We want to make
//   sure that the bool flag is false so we can use pick to determine the
//   coordinates for the new center of rotation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:06:20 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ChooseCenterOfRotationAction::Setup(int activeAction, bool toggled)
{
    // Since this method is called when the action is activated by clicking
    // on it in the popup menu or in the toolbar, make sure that we do
    // interactive choose center.
    args.SetBoolFlag(false);
}

// ****************************************************************************
// Method: ChooseCenterOfRotationAction::Execute
//
// Purpose: 
//   Executes the ChooseCenterOfRotation RPC. 
//
// Note:       If the bool flag is false then we don't have coordinates and
//             we want to use pick to get them. If the bool flag is true then
//             the RPC was issued with the screen position of where we want to
//             pick for the new center of rotation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:07:47 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Sep  2 13:55:05 PDT 2004
//   Added bool flag to SetPickFunction -- indicates we only want Pick to
//   retrieve the intersection point.
//   
// ****************************************************************************

void
ChooseCenterOfRotationAction::Execute()
{
    bool haveCoordinates = args.GetBoolFlag();

    if(haveCoordinates)
    {
        double x = args.GetQueryPoint1()[0];
        double y = args.GetQueryPoint1()[1];
        windowMgr->ChooseCenterOfRotation(window->GetWindowId(), x, y);
    }
    else if(!GetViewerProperties()->GetNowin())
    {
        // We don't have the points so we want to choose them interactively.
        // In order to do this, we have to change the window into
        // pick mode and set up a pick handler so instead of adding a pick
        // point, which is the default pick behavior, we set the center
        // of rotation.
 
        //
        // Store the old interaction mode and set the new interaction mode
        // to zone pick.
        //
        oldMode = window->GetInteractionMode();
        window->SetInteractionMode(ZONE_PICK);
 
        //
        // Register a pick function that will be called when you pick on
        // the plots. Instead of picking, the function will set the center
        // of rotation using the pick point.
        //
        window->SetPickFunction(FinishCB, (void *)this, true);
    }
    else
    {
        GetViewerMessaging()->Error(
            TR("The center of rotation cannot be chosen interactively when "
               "VisIt is run in -nowin mode."));
    }
}

// ****************************************************************************
// Method: ChooseCenterOfRotationAction::FinishCB
//
// Purpose: 
//   Called by pick once it has determined the pick point that we will use
//   as the new center of rotation.
//
// Arguments:
//   data    : A pointer to the action.
//   success : Whether or not pick was able to return a pick point.
//   p       : The pick attributes, which include the pick point, etc.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:15:39 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ChooseCenterOfRotationAction::FinishCB(void *data, bool success,
    const PickAttributes *p)
{
    if(data)
    {
        ChooseCenterOfRotationAction *action = (ChooseCenterOfRotationAction *)data;
        action->FinishExecute(success, p);
    }
}

// ****************************************************************************
// Method: ChooseCenterOfRotationAction::FinishExecute
//
// Purpose: 
//   Finishes executing the ChooseCenterOfRotation RPC.
//
// Arguments:
//   success : Whether VisIt should set the center of rotation.
//   p       : A pointer to the pick attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:17:59 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Mar  7 08:38:29 PST 2006
//   Handle SR mode.
//
//   Brad Whitlock, Tue Apr 29 11:54:47 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ChooseCenterOfRotationAction::FinishExecute(bool success,
    const PickAttributes *p)
{
    // Restore the old interaction mode.
    window->SetInteractionMode(oldMode);

    float pt[3];
    if(success)
    {
        pt[0] = p->GetPickPoint()[0];
        pt[1] = p->GetPickPoint()[1];
        pt[2] = p->GetPickPoint()[2];
    }
    else if (window->GetScalableRendering())
    {
        PickAttributes pa;
        success = window->GetPickAttributesForScreenPoint(p->GetPickPoint()[0],
                                                p->GetPickPoint()[1],
                                                pa);
        if (success)
        {
            pt[0] = pa.GetPickPoint()[0];
            pt[1] = pa.GetPickPoint()[1];
            pt[2] = pa.GetPickPoint()[2];
        }
    }

    if (success)
    {
        // Tell the client about the new center of rotation.
        GetViewerMessaging()->Message(
            TR("The new center of rotation is: <%1, %2, %3>.").
            arg(pt[0]).
            arg(pt[1]).
            arg(pt[2]));

        // Set the new center of rotation.
        windowMgr->SetCenterOfRotation(window->GetWindowId(),
                                       pt[0], pt[1], pt[2]);
    }
    else 
    {
        GetViewerMessaging()->Warning(
            TR("VisIt could not set the center of rotation. "
               "You might not have clicked on a plot."));
    }

     //
     // Now since we've updated the interaction mode, we have to update
     // the actions so the toolbar will show the right interaction mode.
     //
     windowMgr->UpdateActions();
}

// ****************************************************************************
// Method: ClearViewKeyframesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ClearViewKeyframesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ClearViewKeyframesAction::Execute()
{
    windowMgr->ClearViewKeyframes();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteViewKeyframeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::DeleteViewKeyframeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
DeleteViewKeyframeAction::Execute()
{
    //
    // Perform the rpc.
    //
    windowMgr->DeleteViewKeyframe(args.GetFrame());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MoveViewKeyframeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::MoveViewKeyframeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
MoveViewKeyframeAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int oldFrame = args.GetIntArg1();
    int newFrame = args.GetIntArg2();
 
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->MoveViewKeyframe(oldFrame, newFrame);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetView2DAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetView2DRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetView2DAction::Execute()
{
    windowMgr->SetView2DFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetView3DAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetView3DRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetView3DAction::Execute()
{
    windowMgr->SetView3DFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetViewAxisArrayAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetViewAxisArrayRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetViewAxisArrayAction::Execute()
{
    windowMgr->SetViewAxisArrayFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetViewCurveAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetViewCurveRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetViewCurveAction::Execute()
{
    windowMgr->SetViewCurveFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetViewExtentsTypeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetViewExtentsTypeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetViewExtentsTypeAction::Execute()
{
     avtExtentType viewType = (avtExtentType)args.GetWindowLayout();
     windowMgr->SetViewExtentsType(viewType);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetViewKeyframeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetViewKeyframeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetViewKeyframeAction::Execute()
{
    windowMgr->SetViewKeyframe();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleCameraViewModeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ToggleCameraViewModeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ToggleCameraViewModeAction::Execute()
{
    windowMgr->ToggleCameraViewMode(-1);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleMaintainViewModeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ToggleMaintainViewModeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ToggleMaintainViewModeAction::Execute()
{
    windowMgr->ToggleMaintainViewMode(-1);
}
