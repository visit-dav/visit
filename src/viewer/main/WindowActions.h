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

#ifndef WINDOW_ACTIONS_H
#define WINDOW_ACTIONS_H
#include <viewer_exports.h>
#include <ViewerAction.h>
#include <ViewerToggleAction.h>
#include <ViewerMultipleAction.h>

// ****************************************************************************
// Class: AddWindowAction
//
// Purpose:
//   Handles the add window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API AddWindowAction : public ViewerAction
{
public:
    AddWindowAction(ViewerWindow *win);
    virtual ~AddWindowAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: CloneWindowAction
//
// Purpose:
//   Handles the clone window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:03 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API CloneWindowAction : public ViewerAction
{
public:
    CloneWindowAction(ViewerWindow *win);
    virtual ~CloneWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DeleteWindowAction
//
// Purpose:
//   Handles the delete window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API DeleteWindowAction : public ViewerAction
{
public:
    DeleteWindowAction(ViewerWindow *win);
    virtual ~DeleteWindowAction(){}

    virtual void Execute();
    virtual void Update();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ClearWindowAction
//
// Purpose:
//   Handles the clear window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearWindowAction : public ViewerAction
{
public:
    ClearWindowAction(ViewerWindow *win);
    virtual ~ClearWindowAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ClearAllWindowsAction
//
// Purpose:
//   Handles the clear all windows action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearAllWindowsAction : public ViewerAction
{
public:
    ClearAllWindowsAction(ViewerWindow *win);
    virtual ~ClearAllWindowsAction(){}

    virtual void Execute();
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ClearPickPointsAction
//
// Purpose:
//   Handles clearing of pick points.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:16:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearPickPointsAction : public ViewerAction
{
public:
    ClearPickPointsAction(ViewerWindow *win);
    virtual ~ClearPickPointsAction() { };

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ClearReferenceLinesAction
//
// Purpose:
//   Clears reference lines.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:16:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearReferenceLinesAction : public ViewerAction
{
public:
    ClearReferenceLinesAction(ViewerWindow *win);
    virtual ~ClearReferenceLinesAction() { };

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: SetActiveWindowAction
//
// Purpose:
//   Handles the set active window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API SetActiveWindowAction : public ViewerToggleAction
{
public:
    SetActiveWindowAction(ViewerWindow *win);
    virtual ~SetActiveWindowAction(){}

    virtual void Setup();
    virtual void Execute();
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleSpinModeAction
//
// Purpose:
//   Handles the toggle spin mode action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleSpinModeAction : public ViewerToggleAction
{
public:
    ToggleSpinModeAction(ViewerWindow *win);
    virtual ~ToggleSpinModeAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleBoundingBoxModeAction
//
// Purpose:
//   Handles the toggle bounding box mode action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:18:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleBoundingBoxModeAction : public ViewerToggleAction
{
public:
    ToggleBoundingBoxModeAction(ViewerWindow *win);
    virtual ~ToggleBoundingBoxModeAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: SetWindowLayoutAction
//
// Purpose:
//   Handles the set window layout action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:18:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API SetWindowLayoutAction : public ViewerMultipleAction
{
public:
    SetWindowLayoutAction(ViewerWindow *win);
    virtual ~SetWindowLayoutAction() { }

    virtual void Setup();
    virtual void Execute(int val);
    virtual bool ChoiceChecked(int i) const;
};

// ****************************************************************************
// Class: InvertBackgroundAction
//
// Purpose:
//   Handles the invert background color action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:19:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API InvertBackgroundAction : public ViewerAction
{
public:
    InvertBackgroundAction(ViewerWindow *win);
    virtual ~InvertBackgroundAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: SetWindowModeAction
//
// Purpose:
//   Handles the set window mode action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:19:23 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 22 16:28:08 PDT 2003
//   Added Setup.
//   
//   Brad Whitlock, Wed May 28 15:50:34 PDT 2008
//   Removed CanHaveOwnToolbar.
//
// ****************************************************************************

class VIEWER_API SetWindowModeAction : public ViewerMultipleAction
{
public:
    SetWindowModeAction(ViewerWindow *win);
    virtual ~SetWindowModeAction() { }

    virtual void Setup();
    virtual void Execute(int val);
    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const;

    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: EnableToolAction
//
// Purpose:
//   Handles the enable tool action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:20:03 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed May 28 15:50:34 PDT 2008
//   Removed CanHaveOwnToolbar.
//
// ****************************************************************************

class VIEWER_API EnableToolAction : public ViewerMultipleAction
{
public:
    EnableToolAction(ViewerWindow *win);
    virtual ~EnableToolAction() { }

    virtual void Setup();
    virtual void Execute(int val);
    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const;

    virtual bool MenuTopLevel() const { return true; }
};

#endif
