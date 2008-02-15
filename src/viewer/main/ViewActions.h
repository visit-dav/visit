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

#ifndef VIEW_ACTIONS_H
#define VIEW_ACTIONS_H
#include <viewer_exports.h>
#include <ViewerAction.h>
#include <ViewerToggleAction.h>
#include <ViewerMultipleAction.h>
#include <VisWindowTypes.h>
#include <vector>

class PickAttributes;

// ****************************************************************************
// Class: TogglePerspectiveViewAction
//
// Purpose:
//   Handles the toggle perspective view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API TogglePerspectiveViewAction : public ViewerToggleAction
{
public:
    TogglePerspectiveViewAction(ViewerWindow *win);
    virtual ~TogglePerspectiveViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
};

// ****************************************************************************
// Class: ResetViewAction
//
// Purpose:
//   Handles the reset view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:43:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ResetViewAction : public ViewerAction
{
public:
    ResetViewAction(ViewerWindow *win);

    virtual ~ResetViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;

    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: RecenterViewAction
//
// Purpose:
//   Handles the recenter view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:43:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API RecenterViewAction : public ViewerAction
{
public:
    RecenterViewAction(ViewerWindow *win);
    virtual ~RecenterViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;

    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: UndoViewAction
//
// Purpose:
//   Handles the undo view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:44:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API UndoViewAction : public ViewerAction
{
public:
    UndoViewAction(ViewerWindow *win);
    virtual ~UndoViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: RedoViewAction
//
// Purpose:
//   Handles the redo view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:55:11 PST 2006
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API RedoViewAction : public ViewerAction
{
public:
    RedoViewAction(ViewerWindow *win);
    virtual ~RedoViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ToggleFullFrameAction
//
// Purpose:
//   Handles the toggle full frame view action.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   May 13, 2003 
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 13:34:10 PST 2003
//   
// ****************************************************************************

class VIEWER_API ToggleFullFrameAction : public ViewerToggleAction
{
public:
    ToggleFullFrameAction(ViewerWindow *win);
    virtual ~ToggleFullFrameAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: SaveViewAction
//
// Purpose:
//   Saves the current view into the action or sets the view from stored views.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 07:23:26 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Feb 27 14:35:57 PST 2003
//   I added VIEWCurve.
//
//   Brad Whitlock, Tue Jul 1 10:24:25 PDT 2003
//   I added SetFromNode and CreateNode.
//
//   Brad Whitlock, Thu Aug 14 16:12:22 PST 2003
//   I added DeleteViewsFromInterface.
//
//   Brad Whitlock, Wed Apr 27 15:15:35 PST 2005
//   I added CopyFrom.
//
//   Brad Whitlock, Wed Feb 13 14:19:32 PST 2008
//   Added argument to SetFromNode.
//
// ****************************************************************************

class VIEWER_API SaveViewAction : public ViewerMultipleAction
{
    static const int MAX_SAVED_VIEWS;

    static const int VIEWCurve;
    static const int VIEW2D;
    static const int VIEW3D;
    struct ViewInfo
    {
        int  viewType;
        void *view;
    };
    typedef std::vector<ViewInfo> ViewInfoVector;
public:
    SaveViewAction(ViewerWindow *win);
    virtual ~SaveViewAction();

    virtual void Execute(int);

    virtual bool CopyFrom(const ViewerActionBase *);

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceToggled(int i) const { return false;}

    virtual bool CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const std::string &);
private:
    void DeleteViewsFromInterface();
    void DeleteViews();
    void SaveCurrentView();
    void UseSavedView(int index);
    void AddNewView(void *v, int vt);

    ViewInfoVector views;
};

// ****************************************************************************
// Class: SetCenterOfRotationAction
//
// Purpose:
//   Handles setting the center of rotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 09:43:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API SetCenterOfRotationAction : public ViewerAction
{
public:
    SetCenterOfRotationAction(ViewerWindow *win);
    virtual ~SetCenterOfRotationAction();

    virtual void Execute();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ChooseCenterOfRotationAction
//
// Purpose:
//   Chooses the center of rotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 09:49:43 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ChooseCenterOfRotationAction : public ViewerAction
{
public:
    ChooseCenterOfRotationAction(ViewerWindow *win);
    virtual ~ChooseCenterOfRotationAction();

    virtual void Setup();
    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool MenuTopLevel() const { return true; }

protected:
    static void FinishCB(void *data, bool success, const PickAttributes *);
    void FinishExecute(bool success, const PickAttributes *);

    INTERACTION_MODE oldMode;
};

#endif
