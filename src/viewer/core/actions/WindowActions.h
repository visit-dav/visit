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

#ifndef WINDOW_ACTIONS_H
#define WINDOW_ACTIONS_H
#include <ViewerActionLogic.h>

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

class VIEWERCORE_API AddWindowAction : public ViewerActionLogic
{
public:
    AddWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
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

class VIEWERCORE_API CloneWindowAction : public ViewerActionLogic
{
public:
    CloneWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
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

class VIEWERCORE_API DeleteWindowAction : public ViewerActionLogic
{
public:
    DeleteWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeleteWindowAction(){}

    virtual void Execute();
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

class VIEWERCORE_API ClearWindowAction : public ViewerActionLogic
{
public:
    ClearWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearWindowAction(){}

    virtual void Execute();
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

class VIEWERCORE_API ClearAllWindowsAction : public ViewerActionLogic
{
public:
    ClearAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearAllWindowsAction(){}

    virtual void Execute();
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

class VIEWERCORE_API ClearPickPointsAction : public ViewerActionLogic
{
public:
    ClearPickPointsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearPickPointsAction() { };

    virtual void Execute();
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

class VIEWERCORE_API ClearReferenceLinesAction : public ViewerActionLogic
{
public:
    ClearReferenceLinesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearReferenceLinesAction() { };

    virtual void Execute();
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

class VIEWERCORE_API SetActiveWindowAction : public ViewerActionLogic
{
public:
    SetActiveWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetActiveWindowAction(){}

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);
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

class VIEWERCORE_API ToggleSpinModeAction : public ViewerActionLogic
{
public:
    ToggleSpinModeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ToggleSpinModeAction(){}

    virtual void Execute();
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

class VIEWERCORE_API SetWindowLayoutAction : public ViewerActionLogic
{
public:
    SetWindowLayoutAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetWindowLayoutAction() { }

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);
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

class VIEWERCORE_API InvertBackgroundAction : public ViewerActionLogic
{
public:
    InvertBackgroundAction(ViewerWindow *win) : ViewerActionLogic(win) {}
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

class VIEWERCORE_API SetWindowModeAction : public ViewerActionLogic
{
public:
    SetWindowModeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetWindowModeAction() { }

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);
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

class VIEWERCORE_API EnableToolAction : public ViewerActionLogic
{
public:
    EnableToolAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~EnableToolAction() { }

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);
};

// ****************************************************************************
// Class: EnableToolAction
//
// Purpose:
//   Handles the enable tool action.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API SetToolUpdateModeAction : public ViewerActionLogic
{
public:
    SetToolUpdateModeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetToolUpdateModeAction() { }

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);
};

// ****************************************************************************
// Class: DeIconifyAllWindowsAction
//
// Purpose:
//   Handles ViewerRPC::DeIconifyAllWindowsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeIconifyAllWindowsAction : public ViewerActionLogic
{
public:
    DeIconifyAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeIconifyAllWindowsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DisableRedrawAction
//
// Purpose:
//   Handles ViewerRPC::DisableRedrawRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DisableRedrawAction : public ViewerActionLogic
{
public:
    DisableRedrawAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DisableRedrawAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: HideAllWindowsAction
//
// Purpose:
//   Handles ViewerRPC::HideAllWindowsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API HideAllWindowsAction : public ViewerActionLogic
{
public:
    HideAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~HideAllWindowsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: IconifyAllWindowsAction
//
// Purpose:
//   Handles ViewerRPC::IconifyAllWindowsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API IconifyAllWindowsAction : public ViewerActionLogic
{
public:
    IconifyAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~IconifyAllWindowsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: MoveAndResizeWindowAction
//
// Purpose:
//   Handles ViewerRPC::MoveAndResizeWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MoveAndResizeWindowAction : public ViewerActionLogic
{
public:
    MoveAndResizeWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MoveAndResizeWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: MoveWindowAction
//
// Purpose:
//   Handles ViewerRPC::MoveWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MoveWindowAction : public ViewerActionLogic
{
public:
    MoveWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MoveWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: RedrawAction
//
// Purpose:
//   Handles ViewerRPC::RedrawRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RedrawAction : public ViewerActionLogic
{
public:
    RedrawAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~RedrawAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResizeWindowAction
//
// Purpose:
//   Handles ViewerRPC::ResizeWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResizeWindowAction : public ViewerActionLogic
{
public:
    ResizeWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResizeWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetWindowAreaAction
//
// Purpose:
//   Handles ViewerRPC::SetWindowAreaRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetWindowAreaAction : public ViewerActionLogic
{
public:
    SetWindowAreaAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetWindowAreaAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ShowAllWindowsAction
//
// Purpose:
//   Handles ViewerRPC::ShowAllWindowsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ShowAllWindowsAction : public ViewerActionLogic
{
public:
    ShowAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ShowAllWindowsAction() {}

    virtual void Execute();
};

#endif
