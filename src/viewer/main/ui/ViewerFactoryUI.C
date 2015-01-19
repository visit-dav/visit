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
#include <ViewerFactoryUI.h>

#include <ViewerProperties.h>

#include <AnimationActionsUI.h>
#include <ClientActionsUI.h>
#ifdef HAVE_DDT
#include <DDTActions.h>
#include <DDTActionsUI.h>
#endif
#include <LockActionsUI.h>
#include <PlotAndOperatorActionsUI.h>
#include <SetAppearanceAction.h>
#include <PrintWindowAction.h>
#include <WindowActionsUI.h>
#include <ViewActionsUI.h>

#include <QtVisWindow.h>
#include <ToolbarActionsUI.h>
#include <ViewerActionManagerUI.h>
#include <ViewerChangeUsernameUI.h>
#include <ViewerConnectionPrinterUI.h>
#include <ViewerConnectionProgressDialog.h>
#include <ViewerFileServer.h>
#include <ViewerEngineManager.h>
#include <ViewerHostProfileSelectorNoWin.h>
#include <ViewerHostProfileSelectorWithWin.h>
#include <ViewerMessagingUI.h>
#include <ViewerPasswordWindow.h>
#include <ViewerWindowUI.h>
#include <VisWindow.h>
#include <VisWindowWithInteractions.h>
#include <avtCallback.h>

// ****************************************************************************
// Method: ViewerFactoryUI::ViewerFactoryUI
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 00:18:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactoryUI::ViewerFactoryUI() : ViewerFactory()
{
}

// ****************************************************************************
// Method: ViewerFactoryUI::~ViewerFactoryUI
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 00:18:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactoryUI::~ViewerFactoryUI()
{
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateVisWindow
//
// Purpose:
//   Creates a new vis window.
//
// Returns:    A new vis window.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 10:27:57 PDT 2014
//
// Modifications:
//
// ****************************************************************************

VisWindow *
ViewerFactoryUI::CreateVisWindow()
{

// QUESTION: if we make different factories based on -nowin then this check 
//           is not needed. Just return the Qt version.

    VisWindow *visWindow = NULL;
    if (GetViewerProperties()->GetNowin())
    {
        //visWindow = new VisWindow();
        if(!avtCallback::GetNowinInteractionMode()) {
            visWindow = new VisWindow();
        } else {
            visWindow = new VisWindowWithInteractions();
        }
    }
    else
    {
        visWindow = new QtVisWindow(GetViewerProperties()->GetWindowFullScreen());
    }
    return visWindow;
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateActionManager
//
// Purpose:
//   Creates an action manager with UI managing abilities.
//
// Arguments:
//   win : The window with which to associate the manager and actions.
//
// Returns:    A new action manager.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 10:26:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionManager *
ViewerFactoryUI::CreateActionManager(ViewerWindow *win)
{
    ViewerActionManagerUI *mgr = new ViewerActionManagerUI;
    win->SetActionManager(mgr); // HACK
    SetActionManagerLogic(mgr, win);
    SetActionManagerLogicUI(mgr, win);
    SetActionManagerUI(mgr, win);
    return mgr;
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateViewerWindow
//
// Purpose:
//   Creates a ViewerWindowUI object and initializes it.
//
// Arguments:
//   windowIndex : The index of the new window.
//
// Returns:    An initialized ViewerWindowUI object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 10:23:17 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerWindow *
ViewerFactoryUI::CreateViewerWindow(int windowIndex)
{
    ViewerWindowUI *win = new ViewerWindowUI(windowIndex);
    win->SetVisWindow(CreateVisWindow());
    win->SetActionManager(CreateActionManager(win));

    return win;
}

// ****************************************************************************
// Method: ViewerFactoryUI::SetActionManagerLogicUI
//
// Purpose:
//   Set the action logic for actions that are only installed if we have UI.
//
// Arguments:
//   mgr : The action manager.
//   win : The viewer window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 10:22:26 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerFactoryUI::SetActionManagerLogicUI(ViewerActionManager *mgr, ViewerWindow *win)
{
    // These action handlers are only installed when we have a UI.
    mgr->SetLogic(ViewerRPC::EnableToolbarRPC,              new EnableToolbarAction(win));
    mgr->SetLogic(ViewerRPC::HideToolbarsForAllWindowsRPC,  new HideToolbarsForAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::HideToolbarsRPC,               new HideToolbarsAction(win));
    mgr->SetLogic(ViewerRPC::PrintWindowRPC,                new PrintWindowAction(win));
    mgr->SetLogic(ViewerRPC::SetAppearanceRPC,              new SetAppearanceAction(win));
    mgr->SetLogic(ViewerRPC::SetToolbarIconSizeRPC,         new SetToolbarIconSizeAction(win));
    mgr->SetLogic(ViewerRPC::ShowToolbarsForAllWindowsRPC,  new ShowToolbarsForAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::ShowToolbarsRPC,               new ShowToolbarsAction(win));
    mgr->SetLogic(ViewerRPC::ToggleAllowPopupRPC,           new ToggleAllowPopupAction(win));

#ifdef HAVE_DDT
    mgr->SetLogic(ViewerRPC::DDTConnectRPC,                 new DDTConnectAction(win));
    mgr->SetLogic(ViewerRPC::DDTFocusRPC,                   new DDTFocusAction(win));
    mgr->SetLogic(ViewerRPC::ReleaseToDDTRPC,               new ReleaseToDDTAction(win));
    mgr->SetLogic(ViewerRPC::PlotDDTVispointVariablesRPC,   new PlotDDTVispointVariablesAction(win));
    // Override some existing actions with DDT variants.
    mgr->SetLogic(ViewerRPC::AnimationPlayRPC,              new AnimationPlayActionDDT(win));
    mgr->SetLogic(ViewerRPC::AnimationStopRPC,              new AnimationStopActionDDT(win));
    mgr->SetLogic(ViewerRPC::TimeSliderNextStateRPC,        new TimeSliderForwardStepActionDDT(win));
#endif
}

// ****************************************************************************
// Method: ViewerFactoryUI::SetActionManagerUI
//
// Purpose:
//   This method installs the UI components for the actions that have UI's.
//
// Arguments:
//   mgr0 : The action manager.
//   win  : The viewer window associated with the action manager.
//
// Note:       Not all actions have UI's.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 00:17:15 PDT 2014
//
// Modifications:
//
// ****************************************************************************

#define INSTALL_ACTION_UI(RPC, UI) \
{\
    ViewerActionLogic *logic = mgr->GetLogic(RPC);\
    if(logic != NULL)\
        mgr->SetUI(RPC, new UI(logic));\
}

void
ViewerFactoryUI::SetActionManagerUI(ViewerActionManager *mgr0, ViewerWindow *win)
{
    ViewerActionManagerUI *mgr = dynamic_cast<ViewerActionManagerUI *>(mgr0);
    if(mgr == NULL)
        return;

    // Add the action UI logic.
    INSTALL_ACTION_UI(ViewerRPC::AddOperatorRPC,                AddOperatorActionUI);
    INSTALL_ACTION_UI(ViewerRPC::AddPlotRPC,                    AddPlotActionUI);
    INSTALL_ACTION_UI(ViewerRPC::AddWindowRPC,                  AddWindowActionUI);
    INSTALL_ACTION_UI(ViewerRPC::AnimationPlayRPC,              AnimationPlayActionUI);
    INSTALL_ACTION_UI(ViewerRPC::AnimationReversePlayRPC,       AnimationReversePlayActionUI);
    INSTALL_ACTION_UI(ViewerRPC::AnimationStopRPC,              AnimationStopActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ChooseCenterOfRotationRPC,     ChooseCenterOfRotationActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ClearAllWindowsRPC,            ClearAllWindowsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ClearPickPointsRPC,            ClearPickPointsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ClearRefLinesRPC,              ClearReferenceLinesActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ClearWindowRPC,                ClearWindowActionUI);
    INSTALL_ACTION_UI(ViewerRPC::CloneWindowRPC,                CloneWindowActionUI);
    INSTALL_ACTION_UI(ViewerRPC::CopyActivePlotsRPC,            CopyPlotActionUI);
    INSTALL_ACTION_UI(ViewerRPC::DeleteActivePlotsRPC,          DeleteActivePlotsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::DeleteWindowRPC,               DeleteWindowActionUI);
    INSTALL_ACTION_UI(ViewerRPC::DrawPlotsRPC,                  DrawPlotsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::EnableToolRPC,                 EnableToolActionUI);
    INSTALL_ACTION_UI(ViewerRPC::HideActivePlotsRPC,            HideActivePlotsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::InvertBackgroundRPC,           InvertBackgroundActionUI);
    INSTALL_ACTION_UI(ViewerRPC::RecenterViewRPC,               RecenterViewActionUI);
    INSTALL_ACTION_UI(ViewerRPC::RedoViewRPC,                   RedoViewActionUI);
    INSTALL_ACTION_UI(ViewerRPC::RemoveAllOperatorsRPC,         RemoveAllOperatorsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::RemoveLastOperatorRPC,         RemoveLastOperatorActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ResetViewRPC,                  ResetViewActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SaveViewRPC,                   SaveViewActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SetActiveWindowRPC,            SetActiveWindowActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SetPlotFollowsTimeRPC,         SetPlotFollowsTimeActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SetToolUpdateModeRPC,          SetToolUpdateModeActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SetWindowLayoutRPC,            SetWindowLayoutActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SetWindowModeRPC,              SetWindowModeActionUI);
    INSTALL_ACTION_UI(ViewerRPC::TimeSliderNextStateRPC,        TimeSliderForwardStepActionUI);
    INSTALL_ACTION_UI(ViewerRPC::TimeSliderPreviousStateRPC,    TimeSliderReverseStepActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ToggleFullFrameRPC,            ToggleFullFrameActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ToggleLockTimeRPC,             ToggleLockTimeActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ToggleLockToolsRPC,            ToggleLockToolActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ToggleLockViewModeRPC,         ToggleLockViewActionUI);
    INSTALL_ACTION_UI(ViewerRPC::TogglePerspectiveViewRPC,      TogglePerspectiveViewActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ToggleSpinModeRPC,             ToggleSpinModeActionUI);
    INSTALL_ACTION_UI(ViewerRPC::TurnOffAllLocksRPC,            TurnOffAllLocksActionUI);
    INSTALL_ACTION_UI(ViewerRPC::UndoViewRPC,                   UndoViewActionUI);

#ifdef HAVE_DDT
    INSTALL_ACTION_UI(ViewerRPC::ReleaseToDDTRPC,               ReleaseToDDTActionUI);
    INSTALL_ACTION_UI(ViewerRPC::PlotDDTVispointVariablesRPC,   PlotDDTVispointVariablesActionUI);
#endif

    if(!GetViewerProperties()->GetLaunchedByClient())
    {
        INSTALL_ACTION_UI(ViewerRPC::OpenCLIClientRPC,          OpenCLIClientActionUI);
        INSTALL_ACTION_UI(ViewerRPC::OpenGUIClientRPC,          OpenGUIClientActionUI);
        INSTALL_ACTION_UI(ViewerRPC::MenuQuitRPC,               MenuQuitActionUI);
    }
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateViewerMessaging
//
// Purpose:
//   Create a new ViewerMessaging object.
//
// Returns:    A new ViewerMessagingUI object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:11:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessaging *
ViewerFactoryUI::CreateViewerMessaging()
{
    return new ViewerMessagingUI();
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateConnectionProgress
//
// Purpose:
//   Create a new connection progress object.
//
// Returns:    A new ViewerConnectionProgress object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:11:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerConnectionProgress *
ViewerFactoryUI::CreateConnectionProgress()
{
    ViewerConnectionProgressDialog *progress = new ViewerConnectionProgressDialog();

    // Register the dialog with the password window so we can set
    // the dialog's timeout to zero if we have to prompt for a
    // password.
    ViewerPasswordWindow::SetConnectionProgressDialog(progress);

    return progress;
}

// ****************************************************************************
// Method: ViewerFactory::CreateConnectionPrinter
//
// Purpose:
//   Create a new connection printer.
//
// Returns:    A new ViewerConnectionPrinter object.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:35:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerConnectionPrinter *
ViewerFactoryUI::CreateConnectionPrinter()
{
    if(override_CreateConnectionPrinter != NULL)
        return (*override_CreateConnectionPrinter)();

    return new ViewerConnectionPrinterUI();
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateHostProfileSelector
//
// Purpose:
//   Returns a host profile selector.
//
// Returns:    A pointer to the new host profile selector.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 15:57:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerHostProfileSelector *
ViewerFactoryUI::CreateHostProfileSelector()
{
    ViewerHostProfileSelector *selector = NULL;
    if (GetViewerProperties()->GetNowin())
        selector = new ViewerHostProfileSelectorNoWin();
    else
        selector = new ViewerHostProfileSelectorWithWin();
    return selector;
}

// ****************************************************************************
// Method: ViewerFactory::CreateChangeUsername
//
// Purpose:
//   Create an object to let us change the user name.
//
// Returns:    A new object for changing the user name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 15:09:59 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerChangeUsername *
ViewerFactoryUI::CreateChangeUsername()
{
    ViewerChangeUsername *obj = NULL;
    if (GetViewerProperties()->GetNowin())
        obj = new ViewerChangeUsername;
    else
        obj = new ViewerChangeUsernameUI;
    return obj;
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateFileServerInterface
//
// Purpose:
//   Creates the file server interface we'll use.
//
// Returns:    A pointer to the new file server interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 21 15:02:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFileServerInterface *
ViewerFactoryUI::CreateFileServerInterface()
{
    return new ViewerFileServer;
}

// ****************************************************************************
// Method: ViewerFactoryUI::CreateEngineManagerInterface
//
// Purpose:
//   Creates the engine manager interface we'll use.
//
// Returns:    A pointer to the new engine manager interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 21 15:02:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerEngineManagerInterface *
ViewerFactoryUI::CreateEngineManagerInterface()
{
    return new ViewerEngineManager;
}
