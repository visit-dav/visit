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

#include <ViewActions.h>
#include <ViewAttributes.h>
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

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QMenu>
#include <QPixmap>

// Include icons
#include <perspectiveon.xpm>
#include <perspectiveoff.xpm>
#include <resetview.xpm>
#include <recenterview.xpm>
#include <undoview.xpm>
#include <redoview.xpm>
#include <saveview.xpm>
#include <blankcamera.xpm>
#include <choosecenterofrotation.xpm>

///////////////////////////////////////////////////////////////////////////////

TogglePerspectiveViewAction::TogglePerspectiveViewAction(ViewerWindow *win) : ViewerToggleAction(win)
{
    SetAllText(tr("Perspective"));
    SetToolTip(tr("Toggle perspective view"));
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
TogglePerspectiveViewAction::Checked() const
{
    return window->GetPerspectiveProjection();
}

///////////////////////////////////////////////////////////////////////////////

ResetViewAction::ResetViewAction(ViewerWindow *win) : ViewerAction(win)
{
    SetAllText(tr("Reset view"));
    if (!win->GetNoWinMode())
        SetIcon(QIcon(QPixmap(resetview_xpm)));
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
    ViewerAction(win)
{
    SetAllText(tr("Recenter view"));
    if (!win->GetNoWinMode())
        SetIcon(QIcon(QPixmap(recenterview_xpm)));
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
    ViewerAction(win)
{
    SetAllText(tr("Undo view"));
    if (!win->GetNoWinMode())
        SetIcon(QIcon(QPixmap(undoview_xpm)));
}

void
UndoViewAction::Execute()
{
    windowMgr->UndoView(windowId);
}

bool
UndoViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the 
    // action belongs has plots in it and there are views to undo.
    return (window->GetPlotList()->GetNumPlots() > 0) &&
            window->UndoViewEnabled();
}

///////////////////////////////////////////////////////////////////////////////

RedoViewAction::RedoViewAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Redo view"));
    if (!win->GetNoWinMode())
        SetIcon(QIcon(QPixmap(redoview_xpm)));
}

void
RedoViewAction::Execute()
{
    windowMgr->RedoView(windowId);
}

bool
RedoViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the 
    // action belongs has plots in it and there are views to Redo.
    return (window->GetPlotList()->GetNumPlots() > 0) &&
            window->RedoViewEnabled();
}

///////////////////////////////////////////////////////////////////////////////

ToggleFullFrameAction::ToggleFullFrameAction(ViewerWindow *win) : 
    ViewerToggleAction(win)
{
    SetAllText(tr("Full frame"));
    SetToolTip(tr("Toggle full frame"));
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
ToggleFullFrameAction::Checked() const
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
//   Brad Whitlock, Tue Apr 29 11:49:16 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

SaveViewAction::SaveViewAction(ViewerWindow *win) : ViewerMultipleAction(win),
    views()
{
    QString s1(tr("Clear saved views"));
    QString s2(tr("Save view"));
    QString s3(tr("Save current view"));

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
        SetIcon(QIcon(icon));
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
SaveViewAction::CopyFrom(const ViewerActionBase *obj)
{
    bool retval = false;

    if(GetName() == obj->GetName())
    {
        const SaveViewAction *saveView = (const SaveViewAction *)obj;

        // Delete the views from the interface.
        DeleteViewsFromInterface();

        for(int i = 0; i < saveView->views.size(); ++i)
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

        UpdateConstruction();
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
        Warning(tr("You cannot save more than 15 views."));
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

    //
    // Add the view to the action so that it is available in the toolbar.
    //
    QString tmp(tr("Use saved view %1").arg(views.size()));

    if (!window->GetNoWinMode())
    {
        //
        // Create a pixmap from the blank camera pixmap that we can
        // draw on.
        //
        QPixmap icon(blankcamera_xpm);
        QPainter paint(&icon);
        QString str;
        str.sprintf("%ld", views.size());
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
//   Brad Whitlock, Tue Apr 29 11:51:55 PDT 2008
//   Support for internationalization.
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
                QString msg(
                    tr("VisIt cannot use saved view %1 because "
                       "it is a curve view and the window does not "
                       "contain curves.").arg(index + 1));
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
                QString msg(
                    tr("VisIt cannot use saved view %1 because "
                       "it is a 2D view and the window does not "
                       "contain 2D plots.").arg(index + 1));
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
                QString msg(
                    tr("VisIt cannot use saved view %1 because "
                       "it is a 3D view and the window does not "
                       "contain 3D plots.").arg(index + 1));
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
    for(int i = 0; i < views.size(); ++i)
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
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
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
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
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
            viewAtts.ProcessOldVersions(views[i], configVersion.c_str());
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
    ViewerAction(win)
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
    ViewerAction(win)
{
    SetAllText(tr("Choose center"));
    SetToolTip(tr("Choose center of rotation"));
    if (!win->GetNoWinMode())
        SetIcon(QIcon(QPixmap(choosecenterofrotation_xpm)));
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
        Error(tr("The center of rotation cannot be chosen interactively when "
              "VisIt is run in -nowin mode."));
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
        QString num; num.sprintf("<%g, %g, %g>", pt[0], pt[1], pt[2]);
        QString msg = tr("The new center of rotation is: %1.").arg(num);
        Message(msg);

        // Set the new center of rotation.
        windowMgr->SetCenterOfRotation(window->GetWindowId(),
                                       pt[0], pt[1], pt[2]);
    }
    else 
    {
        Warning(tr("VisIt could not set the center of rotation. "
                "You might not have clicked on a plot."));
    }

     //
     // Now since we've updated the interaction mode, we have to update
     // the actions so the toolbar will show the right interaction mode.
     //
     windowMgr->UpdateActions();
}
