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

#include <ViewActionsUI.h>

#include <ViewActions.h>
#include <ViewerActionLogic.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerProperties.h>
#include <ViewerToolbar.h>
#include <ViewerWindowUI.h>
#include <ViewerWindowManager.h>

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

TogglePerspectiveViewActionUI::TogglePerspectiveViewActionUI(ViewerActionLogic *L) : ViewerActionUIToggle(L)
{
    SetAllText(tr("Perspective"));
    SetToolTip(tr("Toggle perspective view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(perspectiveon_xpm), QPixmap(perspectiveoff_xpm));
}

bool
TogglePerspectiveViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the ActionUI belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

bool
TogglePerspectiveViewActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetPerspectiveProjection();
}

///////////////////////////////////////////////////////////////////////////////

ResetViewActionUI::ResetViewActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Reset view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(resetview_xpm)));
}

bool
ResetViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the ActionUI belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

RecenterViewActionUI::RecenterViewActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Recenter view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(recenterview_xpm)));
}

bool
RecenterViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the ActionUI belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

UndoViewActionUI::UndoViewActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Undo view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(undoview_xpm)));
}

bool
UndoViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the 
    // ActionUI belongs has plots in it and there are views to undo.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0) &&
            GetLogic()->GetWindow()->UndoViewEnabled();
}

///////////////////////////////////////////////////////////////////////////////

RedoViewActionUI::RedoViewActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Redo view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(redoview_xpm)));
}

bool
RedoViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the 
    // ActionUI belongs has plots in it and there are views to Redo.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0) &&
            GetLogic()->GetWindow()->RedoViewEnabled();
}

///////////////////////////////////////////////////////////////////////////////

ToggleFullFrameActionUI::ToggleFullFrameActionUI(ViewerActionLogic *L) : 
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Full frame"));
    SetToolTip(tr("Toggle full frame"));
}

bool
ToggleFullFrameActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the ActionUI 
    // belongs has plots in it, and is 2D.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0) &&
           (GetLogic()->GetWindow()->GetWindowMode() == WINMODE_2D);
}

bool
ToggleFullFrameActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetFullFrameMode();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SaveViewActionUI::SaveViewActionUI
//
// Purpose: 
//   Constructor for the SaveViewActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:26 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:49:16 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

SaveViewActionUI::SaveViewActionUI(ViewerActionLogic *L) : ViewerActionUIMultiple(L)
{
    QString s1(tr("Clear saved views"));
    QString s2(tr("Save view"));
    QString s3(tr("Save current view"));

    SetAllText(s2);
    SetToolTip(s3);
    SetExclusive(false);

    if (!GetViewerProperties()->GetNowin())
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
// Method: SaveViewActionUI::~SaveViewActionUI
//
// Purpose: 
//   Destructor for the SaveViewActionUI class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:31 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SaveViewActionUI::~SaveViewActionUI()
{
}

int
SaveViewActionUI::NumSavedViews() const
{
    int n = 0;
    SaveViewAction *sva = dynamic_cast<SaveViewAction *>(GetLogic());
    if(sva != NULL)
    {
        n = sva->NumSavedViews();
    }
    return n;
}

void
SaveViewActionUI::Update()
{
    // Get the logic and caste it to SaveViewAction and update the number of options
    // if it is different.
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(GetLogic()->GetWindow());
    if(win == NULL)
        return;

    int nCurrentViews = NumSavedViews();
    int nGUIViews = (int)children.size() - 2;

    if(nCurrentViews > nGUIViews)
    {
        // Remove the ActionUI from the popup menu and the toolbar.
        win->GetPopupMenu()->RemoveAction(this);
        win->GetToolbar()->RemoveAction(this);

        // Add some new view buttons.
        int nAdditionalViews = nCurrentViews - nGUIViews;
        for(int i = 0; i < nAdditionalViews; ++i)
        {
            //
            // Add the view to the action
            //
            QString tmp(tr("Use saved view %1").arg(nGUIViews + i + 1));

            if (!GetViewerProperties()->GetNowin())
            {
                //
                // Create a pixmap from the blank camera pixmap that we can
                // draw on.
                //
                QPixmap icon(blankcamera_xpm);
                QPainter paint(&icon);
                QString str;
                str.sprintf("%d", nGUIViews + i + 1);
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

        UpdateConstruction();
    }
    else if(nCurrentViews < nGUIViews)
    {
        // Remove the ActionUI from the popup menu and the toolbar.
        win->GetPopupMenu()->RemoveAction(this);
        win->GetToolbar()->RemoveAction(this);

        // Take away some view buttons.
        int nFewerViews = nGUIViews - nCurrentViews;
        for(int i = 0; i < nFewerViews; ++i)
        {
            delete children.back();
            children.pop_back();
        }

        UpdateConstruction();
    }

    ViewerActionUIMultiple::Update();
}

// ****************************************************************************
// Method: SaveViewActionUI::Enabled
//
// Purpose: 
//   Tells when this ActionUI is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
SaveViewActionUI::Enabled() const
{
    return ViewerActionUIMultiple::Enabled() &&
           GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

// ****************************************************************************
// Method: SaveViewActionUI::ChoiceEnabled
//
// Purpose: 
//   Tells when the individual choices in this ActionUI are enabled.
//
// Arguments:
//   i : The ActionUI that to consider when returning the enabled flag.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:53:14 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
SaveViewActionUI::ChoiceEnabled(int i) const
{
    bool retval = true;

#if 0
    if(i == 0)
        retval = (views.size() > 0);
    else if(i == 1)
        retval = (views.size() < 15);
#endif

    return retval;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ChooseCenterOfRotationActionUI::ChooseCenterOfRotationActionUI
//
// Purpose: 
//   Constructor for the ChooseCenterOfRotationActionUI class.
//
// Arguments:
//   win : The window to which the ActionUI belongs.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:05:39 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

ChooseCenterOfRotationActionUI::ChooseCenterOfRotationActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Choose center"));
    SetToolTip(tr("Choose center of rotation"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(choosecenterofrotation_xpm)));
}

// ****************************************************************************
// Method: ChooseCenterOfRotationActionUI::~ChooseCenterOfRotationActionUI
//
// Purpose: 
//   Destructor for the ChooseCenterOfRotationActionUI class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 10:06:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

ChooseCenterOfRotationActionUI::~ChooseCenterOfRotationActionUI()
{
}

// ****************************************************************************
// Method: ChooseCenterOfRotationActionUI::Enabled
//
// Purpose: 
//   Returns when the ActionUI is enabled.
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
ChooseCenterOfRotationActionUI::Enabled() const
{
    return GetLogic()->GetWindow()->GetPlotList()->GetNumVisiblePlots() > 0 &&
           GetLogic()->GetWindow()->GetWindowMode() == WINMODE_3D;
}

