#include <ViewActions.h>
#include <ViewAttributes.h>
#include <ViewerMessaging.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerToolbar.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <snprintf.h>

#include <avtViewCurve.h>
#include <avtView2D.h>
#include <avtView3D.h>

#include <DataNode.h>
#include <DebugStream.h>
#include <PickAttributes.h>

#include <qaction.h>
#include <qapplication.h>
#include <qiconset.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qpixmap.h>

// Include icons
#include <perspectiveon.xpm>
#include <perspectiveoff.xpm>
#include <resetview.xpm>
#include <recenterview.xpm>
#include <undoview.xpm>
#include <viewlockon.xpm>
#include <viewlockoff.xpm>
#include <saveview.xpm>
#include <blankcamera.xpm>
#include <choosecenterofrotation.xpm>

///////////////////////////////////////////////////////////////////////////////

TogglePerspectiveViewAction::TogglePerspectiveViewAction(ViewerWindow *win) : ViewerToggleAction(win, "Perspective")
{
    SetAllText("Perspective");
    SetToolTip("Toggle perspective view");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(perspectiveon_xpm), QPixmap(perspectiveoff_xpm));
}

void
TogglePerspectiveViewAction::Execute()
{
    windowMgr->TogglePerspective(window->GetWindowId());
}

bool
TogglePerspectiveViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

bool
TogglePerspectiveViewAction::Toggled() const
{
    return window->GetPerspectiveProjection();
}

///////////////////////////////////////////////////////////////////////////////

ResetViewAction::ResetViewAction(ViewerWindow *win) : ViewerAction(win, "ResetViewAction")
{
    SetAllText("Reset view");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(resetview_xpm)));
}

void
ResetViewAction::Execute()
{
    windowMgr->ResetView(windowId);
}

bool
ResetViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

RecenterViewAction::RecenterViewAction(ViewerWindow *win) :
    ViewerAction(win, "RecenterViewAction")
{
    SetAllText("Recenter view");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(recenterview_xpm)));
}

void
RecenterViewAction::Execute()
{
    windowMgr->RecenterView(windowId);
}

bool
RecenterViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

UndoViewAction::UndoViewAction(ViewerWindow *win) :
    ViewerAction(win, "UndoViewAction")
{
    SetAllText("Undo view");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(undoview_xpm)));
}

void
UndoViewAction::Execute()
{
    windowMgr->UndoView(windowId);
}

bool
UndoViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}


///////////////////////////////////////////////////////////////////////////////

ToggleLockViewAction::ToggleLockViewAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ToggleLockViewAction")
{
    SetAllText("Lock view");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(viewlockon_xpm), QPixmap(viewlockoff_xpm));
}

void
ToggleLockViewAction::Execute()
{
    windowMgr->ToggleLockViewMode(windowId);
}

bool
ToggleLockViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

bool
ToggleLockViewAction::Toggled() const
{
    return window->GetViewIsLocked();
}

///////////////////////////////////////////////////////////////////////////////

ToggleFullFrameAction::ToggleFullFrameAction(ViewerWindow *win) : 
    ViewerToggleAction(win, "Full frame")
{
    SetAllText("Full frame");
    SetToolTip("Toggle full frame");
}

void
ToggleFullFrameAction::Execute()
{
    windowMgr->ToggleFullFrameMode(window->GetWindowId());
}

bool
ToggleFullFrameAction::Enabled() const
{
    // This action should only be enabled if the window to which the action 
    // belongs has plots in it, and is 2D.
    return (window->GetPlotList()->GetNumPlots() > 0) &&
           (window->GetWindowMode() == WINMODE_2D);
}

bool
ToggleFullFrameAction::Toggled() const
{
    return window->GetFullFrameMode();
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
//   
// ****************************************************************************

SaveViewAction::SaveViewAction(ViewerWindow *win) : ViewerMultipleAction(win,
    "SaveView"), views()
{
    const char *s1 = "Clear saved views";
    const char *s2 = "Save view";
    const char *s3 = "Save current view";

    SetAllText(s2);
    SetToolTip(s3);
    SetExclusive(false);

    if (!win->GetNoWinMode())
    {
        // Add the clear saved views choice.
        QPixmap clearIcon(saveview_xpm);
        QPainter paint(&clearIcon);
        QPen pen(QColor(255,0,0));
        pen.setWidth(2);
        paint.setPen(pen);
        paint.drawLine(clearIcon.width()-1, 0, 0, clearIcon.height()-1);
        AddChoice(s1, s1, clearIcon);
        
        // Add the save view choice
        QPixmap icon(saveview_xpm);
        SetIconSet(QIconSet(icon));
        AddChoice(s2, s3, icon);
    }
    else
    {
        AddChoice(s1);
        AddChoice(s2);
    }
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
    for(int i = 0; i < views.size(); ++i)
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

// ****************************************************************************
// Method: SaveViewAction::DeleteViewsFromInterface(
//
// Purpose: 
//   Deletes the saved views from the interface (toolbar, menu).
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 14 16:13:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
SaveViewAction::DeleteViewsFromInterface()
{
    // Delete the views.
    DeleteViews();

    // Remove the action from the popup menu and the toolbar.
    window->GetPopupMenu()->RemoveAction(this);
    window->GetToolbar()->RemoveAction(this);

    // Remove all of the choices after the second choice.
    int s = children.size();
    for(int i = 2; i < s; ++i)
    {
        delete children[s - i + 1];
        children.pop_back();
    }
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
SaveViewAction::Execute(int val)
{
    if(val == 0)
    {
        // Delete the views from the interface.
        DeleteViewsFromInterface();

        // Update the construction.
        UpdateConstruction();
    }
    else if(val == 1)
    {
        // Save the current view.
        SaveCurrentView();
    }
    else
    {
        // Use a saved view.
        UseSavedView(val - 2);
    }
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
// ****************************************************************************

void
SaveViewAction::SaveCurrentView()
{
    if(views.size() < MAX_SAVED_VIEWS)
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

        // Update the menus and the toolbar.
        UpdateConstruction();
    }
    else
    {
        // We only have this limit because I made ViewerMultipleActions
        // capable of containing a finite number of actions.
        Warning("You cannot save more than 15 views.");
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

    //
    // Add the view to the action so that it is available in the toolbar.
    //
    char tmp[20];
    SNPRINTF(tmp, 20, "Use saved view %d", views.size());

    if (!window->GetNoWinMode())
    {
        //
        // Create a pixmap from the blank camera pixmap that we can
        // draw on.
        //
        QPixmap icon(blankcamera_xpm);
        QPainter paint(&icon);
        QString str;
        str.sprintf("%d", views.size());
        paint.setPen(QColor(0,255,0));
        QFont f(QApplication::font());
        f.setBold(true);
        f.setPixelSize(28);
        paint.setFont(f);
        int x = icon.width();
        int y = icon.height();
        paint.drawText(icon.width() - x, 0, x, y, Qt::AlignCenter, str);

        AddChoice(tmp, tmp, icon);
    }
    else
    {
        AddChoice(tmp);
    }
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
// ****************************************************************************

void
SaveViewAction::UseSavedView(int index)
{
    if(index >= 0 && index < views.size())
    {
        if(views[index].viewType == VIEWCurve)
        {
            if(window->GetWindowMode() == WINMODE_CURVE)
            {
                window->SetViewCurve(*((avtViewCurve *)views[index].view));
            }
            else
            {
                char msg[200];
                SNPRINTF(msg, 200, "VisIt cannot use saved view %d because "
                         "it is a curve view and the window does not "
                         "contain curves.", index + 1);
                Error(msg);
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
                char msg[200];
                SNPRINTF(msg, 200, "VisIt cannot use saved view %d because "
                         "it is a 2D view and the window does not "
                         "contain 2D plots.", index + 1);
                Error(msg);
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
                char msg[200];
                SNPRINTF(msg, 200, "VisIt cannot use saved view %d because "
                         "it is a 3D view and the window does not "
                         "contain 3D plots.", index + 1);
                Error(msg);
            }
        }
    }
}

// ****************************************************************************
// Method: SaveViewAction::Enabled
//
// Purpose: 
//   Tells when this action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
SaveViewAction::Enabled() const
{
    return ViewerMultipleAction::Enabled() &&
           window->GetPlotList()->GetNumPlots() > 0;
}

// ****************************************************************************
// Method: SaveViewAction::ChoiceEnabled
//
// Purpose: 
//   Tells when the individual choices in this action are enabled.
//
// Arguments:
//   i : The action that to consider when returning the enabled flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:14 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
SaveViewAction::ChoiceEnabled(int i) const
{
    bool retval = true;

    if(i == 0)
        retval = (views.size() > 0);
    else if(i == 1)
        retval = (views.size() < 15);

    return retval;
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
    for(int i = 0; i < views.size(); ++i)
    {
        if(views[i].viewType == VIEWCurve)
        {
            ViewCurveAttributes viewAtts;

            avtViewCurve *v = (avtViewCurve *)views[i].view;
            v->SetToViewCurveAttributes(&viewAtts);

            if(viewAtts.CreateNode(saveviewNode, false, false))
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

            if(viewAtts.CreateNode(saveviewNode, false, false))
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

            if(viewAtts.CreateNode(saveviewNode, false, false))
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
// ****************************************************************************

void
SaveViewAction::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *saveviewNode = parentNode->GetNode("SaveViewAction");
    if(saveviewNode == 0)
        return;

    DataNode *viewTypesNode = saveviewNode->GetNode("viewTypes");
    if(viewTypesNode == 0)
        return;

    bool addedOrRemovedViews = false;
    if(views.size() > 0)
    {
        DeleteViewsFromInterface();
        addedOrRemovedViews = true;
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
            viewAtts.SetFromNode(views[i]);

            // Create the curve view.
            avtViewCurve *v = new avtViewCurve;
            v->SetFromViewCurveAttributes(&viewAtts);

            // Add it to the saved views.
            AddNewView((void *)v, VIEWCurve);
            addedOrRemovedViews = true;

            ++index;
        }
        else if(views[i]->GetKey() == "View2DAttributes")
        {
            // Read the view from the config node.
            View2DAttributes viewAtts;
            viewAtts.SetFromNode(views[i]);

            // Create the curve view.
            avtView2D *v = new avtView2D;
            v->SetFromView2DAttributes(&viewAtts);

            // Add it to the saved views.
            AddNewView((void *)v, VIEW2D);
            addedOrRemovedViews = true;

            ++index;
        }
        else if(views[i]->GetKey() == "View3DAttributes")
        {
            // Read the view from the config node.
            View3DAttributes viewAtts;
            viewAtts.SetFromNode(views[i]);

            // Create the curve view.
            avtView3D *v = new avtView3D;
            v->SetFromView3DAttributes(&viewAtts);

            // Add it to the saved views.
            AddNewView((void *)v, VIEW3D);
            addedOrRemovedViews = true;

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
                addedOrRemovedViews = true;
            }

            ++index;
        }
    }

    // If we added some views, update the menus.
    if(addedOrRemovedViews)
        UpdateConstruction();
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
    ViewerAction(win, "SetCenterOfRotationAction")
{
    DisableVisual();
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

// ****************************************************************************
// Method: SetCenterOfRotationAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True when there are plots; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:05:04 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Feb 23 11:52:44 PDT 2004
//   Got rid of the animation.
//
// ****************************************************************************

bool
SetCenterOfRotationAction::Enabled() const
{
    return window->GetPlotList()->GetNumPlots() > 0;    
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
    ViewerAction(win, "ChooseCenterOfRotationAction")
{
    SetAllText("Choose center");
    SetToolTip("Choose center of rotation");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(choosecenterofrotation_xpm)));
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
ChooseCenterOfRotationAction::Setup()
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
    else if(!ViewerWindow::GetNoWinMode())
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
        Error("The center of rotation cannot be chosen interactively when "
              "VisIt is run in -nowin mode.");
    }
}

// ****************************************************************************
// Method: ChooseCenterOfRotationAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if there are realized plots; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:11:24 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Feb 23 11:53:02 PDT 2004
//   Got rid of the animation.
//
// ****************************************************************************

bool
ChooseCenterOfRotationAction::Enabled() const
{
    return window->GetPlotList()->GetNumVisiblePlots() > 0 &&
           window->GetWindowMode() == WINMODE_3D;
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
//   
// ****************************************************************************

void
ChooseCenterOfRotationAction::FinishExecute(bool success,
    const PickAttributes *p)
{
     // Restore the old interaction mode.
     window->SetInteractionMode(oldMode);

     if(success)
     {
         // Tell the client about the new center of rotation.
         char msg[500];
         SNPRINTF(msg, 500, "The new center of rotation is: <%g, %g, %g>.",
                  p->GetPickPoint()[0],
                  p->GetPickPoint()[1],
                  p->GetPickPoint()[2]);
         Message(msg);

         // Set the new center of rotation.
         windowMgr->SetCenterOfRotation(window->GetWindowId(),
                                        p->GetPickPoint()[0],
                                        p->GetPickPoint()[1],
                                        p->GetPickPoint()[2]);
     }
     else
     {
         Warning("VisIt could not set the center of rotation. "
                 "You might not have clicked on a plot.");
     }

     //
     // Now since we've updated the interaction mode, we have to update
     // the actions so the toolbar will show the right interaction mode.
     //
     windowMgr->UpdateActions();
}
