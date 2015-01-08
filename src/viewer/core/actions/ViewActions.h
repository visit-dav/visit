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

#ifndef VIEW_ACTIONS_H
#define VIEW_ACTIONS_H
#include <ViewerActionLogic.h>
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

class VIEWERCORE_API TogglePerspectiveViewAction : public ViewerActionLogic
{
public:
    TogglePerspectiveViewAction(ViewerWindow *win);
    virtual ~TogglePerspectiveViewAction(){}

    virtual void Execute();
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

class VIEWERCORE_API ResetViewAction : public ViewerActionLogic
{
public:
    ResetViewAction(ViewerWindow *win);
    virtual ~ResetViewAction(){}

    virtual void Execute();
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

class VIEWERCORE_API RecenterViewAction : public ViewerActionLogic
{
public:
    RecenterViewAction(ViewerWindow *win);
    virtual ~RecenterViewAction(){}

    virtual void Execute();
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

class VIEWERCORE_API UndoViewAction : public ViewerActionLogic
{
public:
    UndoViewAction(ViewerWindow *win);
    virtual ~UndoViewAction(){}

    virtual void Execute();
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

class VIEWERCORE_API RedoViewAction : public ViewerActionLogic
{
public:
    RedoViewAction(ViewerWindow *win);
    virtual ~RedoViewAction(){}

    virtual void Execute();
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

class VIEWERCORE_API ToggleFullFrameAction : public ViewerActionLogic
{
public:
    ToggleFullFrameAction(ViewerWindow *win);
    virtual ~ToggleFullFrameAction(){}

    virtual void Execute();
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

class VIEWERCORE_API SaveViewAction : public ViewerActionLogic
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

    virtual void Execute();

    virtual bool CopyFrom(const ViewerActionLogic *);
    virtual bool CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const std::string &);

    int NumSavedViews() const;
private:
    virtual void Setup(int activeAction, bool toggled);

    void DeleteViewsFromInterface();
    void DeleteViews();
    void SaveCurrentView();
    void UseSavedView(int index);
    void AddNewView(void *v, int vt);

    int            activeChoice;
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

class VIEWERCORE_API SetCenterOfRotationAction : public ViewerActionLogic
{
public:
    SetCenterOfRotationAction(ViewerWindow *win);
    virtual ~SetCenterOfRotationAction();

    virtual void Execute();
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

class VIEWERCORE_API ChooseCenterOfRotationAction : public ViewerActionLogic
{
public:
    ChooseCenterOfRotationAction(ViewerWindow *win);
    virtual ~ChooseCenterOfRotationAction();

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);

    static void FinishCB(void *data, bool success, const PickAttributes *);
    void FinishExecute(bool success, const PickAttributes *);

    INTERACTION_MODE oldMode;
};

// ****************************************************************************
// Class: ClearViewKeyframesAction
//
// Purpose:
//   Handles ViewerRPC::ClearViewKeyframesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ClearViewKeyframesAction : public ViewerActionLogic
{
public:
    ClearViewKeyframesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearViewKeyframesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DeleteViewKeyframeAction
//
// Purpose:
//   Handles ViewerRPC::DeleteViewKeyframeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeleteViewKeyframeAction : public ViewerActionLogic
{
public:
    DeleteViewKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeleteViewKeyframeAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: MoveViewKeyframeAction
//
// Purpose:
//   Handles ViewerRPC::MoveViewKeyframeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MoveViewKeyframeAction : public ViewerActionLogic
{
public:
    MoveViewKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~MoveViewKeyframeAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetView2DAction
//
// Purpose:
//   Handles ViewerRPC::SetView2DRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetView2DAction : public ViewerActionLogic
{
public:
    SetView2DAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetView2DAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetView3DAction
//
// Purpose:
//   Handles ViewerRPC::SetView3DRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetView3DAction : public ViewerActionLogic
{
public:
    SetView3DAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetView3DAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetViewAxisArrayAction
//
// Purpose:
//   Handles ViewerRPC::SetViewAxisArrayRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetViewAxisArrayAction : public ViewerActionLogic
{
public:
    SetViewAxisArrayAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetViewAxisArrayAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetViewCurveAction
//
// Purpose:
//   Handles ViewerRPC::SetViewCurveRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetViewCurveAction : public ViewerActionLogic
{
public:
    SetViewCurveAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetViewCurveAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetViewExtentsTypeAction
//
// Purpose:
//   Handles ViewerRPC::SetViewExtentsTypeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetViewExtentsTypeAction : public ViewerActionLogic
{
public:
    SetViewExtentsTypeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetViewExtentsTypeAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetViewKeyframeAction
//
// Purpose:
//   Handles ViewerRPC::SetViewKeyframeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetViewKeyframeAction : public ViewerActionLogic
{
public:
    SetViewKeyframeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetViewKeyframeAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ToggleCameraViewModeAction
//
// Purpose:
//   Handles ViewerRPC::ToggleCameraViewModeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ToggleCameraViewModeAction : public ViewerActionLogic
{
public:
    ToggleCameraViewModeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ToggleCameraViewModeAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ToggleMaintainViewModeAction
//
// Purpose:
//   Handles ViewerRPC::ToggleMaintainViewModeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ToggleMaintainViewModeAction : public ViewerActionLogic
{
public:
    ToggleMaintainViewModeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ToggleMaintainViewModeAction() {}

    virtual void Execute();
};

#endif
