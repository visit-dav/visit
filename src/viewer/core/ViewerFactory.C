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
#include <ViewerFactory.h>

#include <VisWindow.h>
#include <VisWindowWithInteractions.h>
#include <ClientActions.h>
#include <AnimationActions.h>
#include <CopyActions.h>
#include <DatabaseActions.h>
#include <DatabaseStateActions.h>
#include <EngineActions.h>
#include <IOActions.h>
#include <LockActions.h>
#include <MDServerActions.h>
#include <PlotAndOperatorActions.h>
#include <QueryActions.h>
#include <WindowActions.h>
#include <ViewActions.h>
#include <SelectionActions.h>
#include <StateActions.h>
#include <avtCallback.h>

#include <OperatorPluginManager.h>
#include <PlotPluginManager.h>

#include <ViewerActionManager.h>
#include <ViewerChangeUsername.h>
#include <ViewerConnectionPrinter.h>
#include <ViewerHostProfileSelectorNoWin.h>
#include <ViewerMessaging.h>
#include <ViewerWindow.h>

// ****************************************************************************
// Method: ViewerFactory::ViewerFactory
//
// Purpose:
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:32:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactory::ViewerFactory() : ViewerBase()
{
    override_CreateConnectionPrinter = NULL;
}

// ****************************************************************************
// Method: ViewerFactory::~ViewerFactory
//
// Purpose:
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:32:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactory::~ViewerFactory()
{
}

// ****************************************************************************
// Method: ViewerFactory::CreateVisWindow
//
// Purpose:
//   Create a new vis window object.
//
// Returns:    A new vis window object.
//
// Note:       We only create the offscreen vis window with this factory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:31:58 PDT 2014
//
// Modifications:
//
// ****************************************************************************

VisWindow *
ViewerFactory::CreateVisWindow()
{
    //return new VisWindow;
    VisWindow* visWindow;
    if(!avtCallback::GetNowinInteractionMode()) {
        visWindow = new VisWindow();
    } else {
        visWindow = new VisWindowWithInteractions();
    }

    return visWindow;
}

// ****************************************************************************
// Method: ViewerFactory::CreateActionManager
//
// Purpose:
//   Creates a new ViewerActionManager and populates it with actions to handle
//   the viewer rpc's.
//
// Arguments:
//   win : The ViewerWindow object associated with the action manager.
//
// Returns:    A new ViewerActionManager object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:30:59 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionManager *
ViewerFactory::CreateActionManager(ViewerWindow *win)
{
    ViewerActionManager *mgr = new ViewerActionManager;
    SetActionManagerLogic(mgr, win);
    return mgr;
}

// ****************************************************************************
// Method: ViewerFactory::CreateViewerWindow
//
// Purpose:
//   Creates a new ViewerWindow object and populates it with the right objects.
//
// Arguments:
//   windowIndex : The index of the new ViewerWindow.
//
// Returns:    A new ViewerWindow object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:30:19 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerWindow *
ViewerFactory::CreateViewerWindow(int windowIndex)
{
    ViewerWindow *win = new ViewerWindow(windowIndex);

    win->SetVisWindow(CreateVisWindow());
    win->SetActionManager(CreateActionManager(win));

    return win;
}

// ****************************************************************************
// Method: ViewerFactory::SetActionManagerLogic
//
// Purpose:
//   Install handlers for various ViewerRPC's.
//
// Arguments:
//   mgr : The action manager getting the actions.
//   win : The viewer window associated with the action manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:29:20 PDT 2014
//
// Modifications:
//   Brad Whitlock, Mon Dec 15 15:05:49 PST 2014
//   Added ReadHostProfilesFromDirectoryRPC.
//
//   Kathleen Biagas, Mon Dec 22 10:21:22 PST 2014
//   Add SetRemoveDuplicateNodesRPC.
//
// ****************************************************************************

void
ViewerFactory::SetActionManagerLogic(ViewerActionManager *mgr, ViewerWindow *win)
{
    // Add the action logic.
    mgr->SetLogic(ViewerRPC::ActivateDatabaseRPC,           new ActivateDatabaseAction(win));
    mgr->SetLogic(ViewerRPC::AddAnnotationObjectRPC,        new AddAnnotationObjectAction(win));
    mgr->SetLogic(ViewerRPC::AddEmbeddedPlotRPC,            new AddEmbeddedPlotAction(win));
    mgr->SetLogic(ViewerRPC::AddInitializedOperatorRPC,     new AddInitializedOperatorAction(win));
    mgr->SetLogic(ViewerRPC::AddOperatorRPC,                new AddOperatorAction(win));
    mgr->SetLogic(ViewerRPC::AddPlotRPC,                    new AddPlotAction(win));
    mgr->SetLogic(ViewerRPC::AddWindowRPC,                  new AddWindowAction(win));
    mgr->SetLogic(ViewerRPC::AlterDatabaseCorrelationRPC,   new AlterDatabaseCorrelationAction(win));
    mgr->SetLogic(ViewerRPC::AnimationPlayRPC,              new AnimationPlayAction(win));
    mgr->SetLogic(ViewerRPC::AnimationReversePlayRPC,       new AnimationReversePlayAction(win));
    mgr->SetLogic(ViewerRPC::AnimationSetNFramesRPC,        new AnimationSetNFramesAction(win));
    mgr->SetLogic(ViewerRPC::AnimationStopRPC,              new AnimationStopAction(win));
    mgr->SetLogic(ViewerRPC::ApplyNamedSelectionRPC,        new ApplyNamedSelectionAction(win));
    mgr->SetLogic(ViewerRPC::ChangeActivePlotsVarRPC,       new ChangeActivePlotsVarAction(win));
    mgr->SetLogic(ViewerRPC::CheckForNewStatesRPC,          new CheckForNewStatesAction(win));
    mgr->SetLogic(ViewerRPC::ChooseCenterOfRotationRPC,     new ChooseCenterOfRotationAction(win));
    mgr->SetLogic(ViewerRPC::ClearAllWindowsRPC,            new ClearAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::ClearCacheForAllEnginesRPC,    new ClearCacheForAllEnginesAction(win));
    mgr->SetLogic(ViewerRPC::ClearCacheRPC,                 new ClearCacheAction(win));
    mgr->SetLogic(ViewerRPC::ClearPickPointsRPC,            new ClearPickPointsAction(win));
    mgr->SetLogic(ViewerRPC::ClearRefLinesRPC,              new ClearReferenceLinesAction(win));
    mgr->SetLogic(ViewerRPC::ClearViewKeyframesRPC,         new ClearViewKeyframesAction(win));
    mgr->SetLogic(ViewerRPC::ClearWindowRPC,                new ClearWindowAction(win));
    mgr->SetLogic(ViewerRPC::CloneWindowRPC,                new CloneWindowAction(win));
    mgr->SetLogic(ViewerRPC::CloseComputeEngineRPC,         new CloseComputeEngineAction(win));
    mgr->SetLogic(ViewerRPC::CloseDatabaseRPC,              new CloseDatabaseAction(win));
    mgr->SetLogic(ViewerRPC::ConstructDataBinningRPC,       new ConstructDataBinningAction(win));
    mgr->SetLogic(ViewerRPC::CopyActivePlotsRPC,            new CopyPlotAction(win));
    mgr->SetLogic(ViewerRPC::CopyAnnotationsToWindowRPC,    new CopyAnnotationsToWindowAction(win));
    mgr->SetLogic(ViewerRPC::CopyLightingToWindowRPC,       new CopyLightingToWindowAction(win));
    mgr->SetLogic(ViewerRPC::CopyPlotsToWindowRPC,          new CopyPlotsToWindowAction(win));
    mgr->SetLogic(ViewerRPC::CopyViewToWindowRPC,           new CopyViewToWindowAction(win));
    mgr->SetLogic(ViewerRPC::CreateDatabaseCorrelationRPC,  new CreateDatabaseCorrelationAction(win));
    mgr->SetLogic(ViewerRPC::CreateNamedSelectionRPC,       new CreateNamedSelectionAction(win));
    mgr->SetLogic(ViewerRPC::DeIconifyAllWindowsRPC,        new DeIconifyAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::DeleteActiveAnnotationObjectsRPC,new DeleteActiveAnnotationObjectsAction(win));
    mgr->SetLogic(ViewerRPC::DeleteActivePlotsRPC,          new DeleteActivePlotsAction(win));
    mgr->SetLogic(ViewerRPC::DeleteDatabaseCorrelationRPC,  new DeleteDatabaseCorrelationAction(win));
    mgr->SetLogic(ViewerRPC::DeleteNamedSelectionRPC,       new DeleteNamedSelectionAction(win));
    mgr->SetLogic(ViewerRPC::DeletePlotDatabaseKeyframeRPC, new DeletePlotDatabaseKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::DeletePlotKeyframeRPC,         new DeletePlotKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::DeleteViewKeyframeRPC,         new DeleteViewKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::DeleteWindowRPC,               new DeleteWindowAction(win));
    mgr->SetLogic(ViewerRPC::DemoteOperatorRPC,             new DemoteOperatorAction(win));
    mgr->SetLogic(ViewerRPC::DisableRedrawRPC,              new DisableRedrawAction(win));
    mgr->SetLogic(ViewerRPC::DrawPlotsRPC,                  new DrawPlotsAction(win));
    mgr->SetLogic(ViewerRPC::EnableToolRPC,                 new EnableToolAction(win));
    mgr->SetLogic(ViewerRPC::ExportColorTableRPC,           new ExportColorTableAction(win));
    mgr->SetLogic(ViewerRPC::ExportDBRPC,                   new ExportDBAction(win));
    mgr->SetLogic(ViewerRPC::ExportEntireStateRPC,          new ExportEntireStateAction(win));
    mgr->SetLogic(ViewerRPC::GetProcInfoRPC,                new GetProcInfoAction(win));
    mgr->SetLogic(ViewerRPC::GetQueryParametersRPC,         new GetQueryParametersAction(win));
    mgr->SetLogic(ViewerRPC::GetQueryParametersRPC,         new GetQueryParametersAction(win));
    mgr->SetLogic(ViewerRPC::HideActiveAnnotationObjectsRPC,new HideActiveAnnotationObjectsAction(win));
    mgr->SetLogic(ViewerRPC::HideActivePlotsRPC,            new HideActivePlotsAction(win));
    mgr->SetLogic(ViewerRPC::HideAllWindowsRPC,             new HideAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::IconifyAllWindowsRPC,          new IconifyAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::ImportEntireStateRPC,          new ImportEntireStateAction(win));
    mgr->SetLogic(ViewerRPC::ImportEntireStateWithDifferentSourcesRPC,new ImportEntireStateWithDifferentSourcesAction(win));
    mgr->SetLogic(ViewerRPC::InitializeNamedSelectionVariablesRPC,new InitializeNamedSelectionVariablesAction(win));
    mgr->SetLogic(ViewerRPC::InvertBackgroundRPC,           new InvertBackgroundAction(win));
    mgr->SetLogic(ViewerRPC::LoadNamedSelectionRPC,         new LoadNamedSelectionAction(win));
    mgr->SetLogic(ViewerRPC::LowerActiveAnnotationObjectsRPC,new LowerActiveAnnotationObjectsAction(win));
    mgr->SetLogic(ViewerRPC::MenuQuitRPC,                   new MenuQuitAction(win));
    mgr->SetLogic(ViewerRPC::MoveAndResizeWindowRPC,        new MoveAndResizeWindowAction(win));
    mgr->SetLogic(ViewerRPC::MovePlotDatabaseKeyframeRPC,   new MovePlotDatabaseKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::MovePlotKeyframeRPC,           new MovePlotKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::MovePlotOrderTowardFirstRPC,   new MovePlotOrderTowardFirstAction(win));
    mgr->SetLogic(ViewerRPC::MovePlotOrderTowardLastRPC,    new MovePlotOrderTowardLastAction(win));
    mgr->SetLogic(ViewerRPC::MoveViewKeyframeRPC,           new MoveViewKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::MoveWindowRPC,                 new MoveWindowAction(win));
    mgr->SetLogic(ViewerRPC::OpenCLIClientRPC,              new OpenCLIClientAction(win));
    mgr->SetLogic(ViewerRPC::OpenComputeEngineRPC,          new OpenComputeEngineAction(win));
    mgr->SetLogic(ViewerRPC::OpenDatabaseRPC,               new OpenDatabaseAction(win));
    mgr->SetLogic(ViewerRPC::OpenGUIClientRPC,              new OpenGUIClientAction(win));
    mgr->SetLogic(ViewerRPC::OpenMDServerRPC,               new OpenMDServerAction(win));
    mgr->SetLogic(ViewerRPC::OverlayDatabaseRPC,            new OverlayDatabaseAction(win));
    mgr->SetLogic(ViewerRPC::ProcessExpressionsRPC,         new ProcessExpressionsAction(win));
    mgr->SetLogic(ViewerRPC::PromoteOperatorRPC,            new PromoteOperatorAction(win));
    mgr->SetLogic(ViewerRPC::QueryRPC,                      new QueryAction(win));
    mgr->SetLogic(ViewerRPC::RaiseActiveAnnotationObjectsRPC,new RaiseActiveAnnotationObjectsAction(win));
    mgr->SetLogic(ViewerRPC::ReOpenDatabaseRPC,             new ReOpenDatabaseAction(win));
    mgr->SetLogic(ViewerRPC::ReadHostProfilesFromDirectoryRPC, new ReadHostProfilesFromDirectoryAction(win));
    mgr->SetLogic(ViewerRPC::RecenterViewRPC,               new RecenterViewAction(win));
    mgr->SetLogic(ViewerRPC::RedoViewRPC,                   new RedoViewAction(win));
    mgr->SetLogic(ViewerRPC::RedrawRPC,                     new RedrawAction(win));
    mgr->SetLogic(ViewerRPC::RemoveAllOperatorsRPC,         new RemoveAllOperatorsAction(win));
    mgr->SetLogic(ViewerRPC::RemoveLastOperatorRPC,         new RemoveLastOperatorAction(win));
    mgr->SetLogic(ViewerRPC::RemoveOperatorRPC,             new RemoveOperatorAction(win));
    mgr->SetLogic(ViewerRPC::RenamePickLabelRPC,            new RenamePickLabelAction(win));
    mgr->SetLogic(ViewerRPC::RenamePickLabelRPC,            new RenamePickLabelAction(win));
    mgr->SetLogic(ViewerRPC::ReplaceDatabaseRPC,            new ReplaceDatabaseAction(win));
    mgr->SetLogic(ViewerRPC::RequestMetaDataRPC,            new RequestMetaDataAction(win));
    mgr->SetLogic(ViewerRPC::ResetAnnotationAttributesRPC,  new ResetAnnotationAttributesAction(win));
    mgr->SetLogic(ViewerRPC::ResetAnnotationObjectListRPC,  new ResetAnnotationObjectListAction(win));
    mgr->SetLogic(ViewerRPC::ResetInteractorAttributesRPC,  new ResetInteractorAttributesAction(win));
    mgr->SetLogic(ViewerRPC::ResetLightListRPC,             new ResetLightListAction(win));
    mgr->SetLogic(ViewerRPC::ResetLineoutColorRPC,          new ResetLineoutColorAction(win));
    mgr->SetLogic(ViewerRPC::ResetMaterialAttributesRPC,    new ResetMaterialAttributesAction(win));
    mgr->SetLogic(ViewerRPC::ResetMeshManagementAttributesRPC,new ResetMeshManagementAttributesAction(win));
    mgr->SetLogic(ViewerRPC::ResetOperatorOptionsRPC,       new ResetOperatorOptionsAction(win));
    mgr->SetLogic(ViewerRPC::ResetPickAttributesRPC,        new ResetPickAttributesAction(win));
    mgr->SetLogic(ViewerRPC::ResetPickLetterRPC,            new ResetPickLetterAction(win));
    mgr->SetLogic(ViewerRPC::ResetPlotOptionsRPC,           new ResetPlotOptionsAction(win));
    mgr->SetLogic(ViewerRPC::ResetQueryOverTimeAttributesRPC,new ResetQueryOverTimeAttributesAction(win));
    mgr->SetLogic(ViewerRPC::ResetViewRPC,                  new ResetViewAction(win));
    mgr->SetLogic(ViewerRPC::ResizeWindowRPC,               new ResizeWindowAction(win));
    mgr->SetLogic(ViewerRPC::SaveNamedSelectionRPC,         new SaveNamedSelectionAction(win));
    mgr->SetLogic(ViewerRPC::SaveViewRPC,                   new SaveViewAction(win));
    mgr->SetLogic(ViewerRPC::SaveWindowRPC,                 new SaveWindowAction(win));
    mgr->SetLogic(ViewerRPC::SendSimulationCommandRPC,      new SendSimulationCommandAction(win));
    mgr->SetLogic(ViewerRPC::SetActivePlotsRPC,             new SetActivePlotsAction(win));
    mgr->SetLogic(ViewerRPC::SetActiveTimeSliderRPC,        new SetActiveTimeSliderAction(win));
    mgr->SetLogic(ViewerRPC::SetActiveWindowRPC,            new SetActiveWindowAction(win));
    mgr->SetLogic(ViewerRPC::SetAnimationAttributesRPC,     new SetAnimationAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetAnnotationAttributesRPC,    new SetAnnotationAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetAnnotationObjectOptionsRPC, new SetAnnotationObjectOptionsAction(win));
    mgr->SetLogic(ViewerRPC::SetBackendTypeRPC,             new SetBackendTypeAction(win));
    mgr->SetLogic(ViewerRPC::SetCenterOfRotationRPC,        new SetCenterOfRotationAction(win));
    mgr->SetLogic(ViewerRPC::SetCreateMeshQualityExpressionsRPC,new SetCreateMeshQualityExpressionsAction(win));
    mgr->SetLogic(ViewerRPC::SetCreateTimeDerivativeExpressionsRPC,new SetCreateTimeDerivativeExpressionsAction(win));
    mgr->SetLogic(ViewerRPC::SetCreateVectorMagnitudeExpressionsRPC,new SetCreateVectorMagnitudeExpressionsAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultAnnotationAttributesRPC,new SetDefaultAnnotationAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultAnnotationObjectListRPC,new SetDefaultAnnotationObjectListAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultFileOpenOptionsRPC,  new SetDefaultFileOpenOptionsAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultInteractorAttributesRPC,new SetDefaultInteractorAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultLightListRPC,        new SetDefaultLightListAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultMaterialAttributesRPC,new SetDefaultMaterialAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultMeshManagementAttributesRPC,new SetDefaultMeshManagementAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultOperatorOptionsRPC,  new SetDefaultOperatorOptionsAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultPickAttributesRPC,   new SetDefaultPickAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultPlotOptionsRPC,      new SetDefaultPlotOptionsAction(win));
    mgr->SetLogic(ViewerRPC::SetDefaultQueryOverTimeAttributesRPC,new SetDefaultQueryOverTimeAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetGlobalLineoutAttributesRPC, new SetGlobalLineoutAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetInteractorAttributesRPC,    new SetInteractorAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetKeyframeAttributesRPC,      new SetKeyframeAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetLightListRPC,               new SetLightListAction(win));
    mgr->SetLogic(ViewerRPC::SetMaterialAttributesRPC,      new SetMaterialAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetMeshManagementAttributesRPC,new SetMeshManagementAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetNamedSelectionAutoApplyRPC, new SetNamedSelectionAutoApplyAction(win));
    mgr->SetLogic(ViewerRPC::SetOperatorOptionsRPC,         new SetOperatorOptionsAction(win));
    mgr->SetLogic(ViewerRPC::SetPickAttributesRPC,          new SetPickAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotDatabaseStateRPC,       new SetPlotDatabaseStateAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotDescriptionRPC,         new SetPlotDescriptionAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotFollowsTimeRPC,         new SetPlotFollowsTimeAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotFrameRangeRPC,          new SetPlotFrameRangeAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotOptionsRPC,             new SetPlotOptionsAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotOrderToFirstRPC,        new SetPlotOrderToFirstAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotOrderToLastRPC,         new SetPlotOrderToLastAction(win));
    mgr->SetLogic(ViewerRPC::SetPlotSILRestrictionRPC,      new SetPlotSILRestrictionAction(win));
    mgr->SetLogic(ViewerRPC::SetPrecisionTypeRPC,           new SetPrecisionTypeAction(win));
    mgr->SetLogic(ViewerRPC::SetQueryFloatFormatRPC,        new SetQueryFloatFormatAction(win));
    mgr->SetLogic(ViewerRPC::SetQueryOverTimeAttributesRPC, new SetQueryOverTimeAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetRemoveDuplicateNodesRPC,    new SetRemoveDuplicateNodesAction(win));
    mgr->SetLogic(ViewerRPC::SetRenderingAttributesRPC,     new SetRenderingAttributesAction(win));
    mgr->SetLogic(ViewerRPC::SetSuppressMessagesRPC,        new SetSuppressMessagesAction(win));
    mgr->SetLogic(ViewerRPC::SetTimeSliderStateRPC,         new SetTimeSliderStateAction(win));
    mgr->SetLogic(ViewerRPC::SetToolUpdateModeRPC,          new SetToolUpdateModeAction(win));
    mgr->SetLogic(ViewerRPC::SetTreatAllDBsAsTimeVaryingRPC,new SetTreatAllDBsAsTimeVaryingAction(win));
    mgr->SetLogic(ViewerRPC::SetTryHarderCyclesTimesRPC,    new SetTryHarderCyclesTimesAction(win));
    mgr->SetLogic(ViewerRPC::SetView2DRPC,                  new SetView2DAction(win));
    mgr->SetLogic(ViewerRPC::SetView3DRPC,                  new SetView3DAction(win));
    mgr->SetLogic(ViewerRPC::SetViewAxisArrayRPC,           new SetViewAxisArrayAction(win));
    mgr->SetLogic(ViewerRPC::SetViewCurveRPC,               new SetViewCurveAction(win));
    mgr->SetLogic(ViewerRPC::SetViewExtentsTypeRPC,         new SetViewExtentsTypeAction(win));
    mgr->SetLogic(ViewerRPC::SetViewKeyframeRPC,            new SetViewKeyframeAction(win));
    mgr->SetLogic(ViewerRPC::SetWindowAreaRPC,              new SetWindowAreaAction(win));
    mgr->SetLogic(ViewerRPC::SetWindowLayoutRPC,            new SetWindowLayoutAction(win));
    mgr->SetLogic(ViewerRPC::SetWindowModeRPC,              new SetWindowModeAction(win));
    mgr->SetLogic(ViewerRPC::ShowAllWindowsRPC,             new ShowAllWindowsAction(win));
    mgr->SetLogic(ViewerRPC::SuppressQueryOutputRPC,        new SuppressQueryOutputAction(win));
    mgr->SetLogic(ViewerRPC::TimeSliderNextStateRPC,        new TimeSliderForwardStepAction(win));
    mgr->SetLogic(ViewerRPC::TimeSliderPreviousStateRPC,    new TimeSliderReverseStepAction(win));
    mgr->SetLogic(ViewerRPC::ToggleCameraViewModeRPC,       new ToggleCameraViewModeAction(win));
    mgr->SetLogic(ViewerRPC::ToggleFullFrameRPC,            new ToggleFullFrameAction(win));
    mgr->SetLogic(ViewerRPC::ToggleLockTimeRPC,             new ToggleLockTimeAction(win));
    mgr->SetLogic(ViewerRPC::ToggleLockToolsRPC,            new ToggleLockToolAction(win));
    mgr->SetLogic(ViewerRPC::ToggleLockViewModeRPC,         new ToggleLockViewAction(win));
    mgr->SetLogic(ViewerRPC::ToggleMaintainViewModeRPC,     new ToggleMaintainViewModeAction(win));
    mgr->SetLogic(ViewerRPC::TogglePerspectiveViewRPC,      new TogglePerspectiveViewAction(win));
    mgr->SetLogic(ViewerRPC::ToggleSpinModeRPC,             new ToggleSpinModeAction(win));
    mgr->SetLogic(ViewerRPC::TurnOffAllLocksRPC,            new TurnOffAllLocksAction(win));
    mgr->SetLogic(ViewerRPC::UndoViewRPC,                   new UndoViewAction(win));
    mgr->SetLogic(ViewerRPC::UpdateColorTableRPC,           new UpdateColorTableAction(win));
    mgr->SetLogic(ViewerRPC::UpdateNamedSelectionRPC,       new UpdateNamedSelectionAction(win));
    mgr->SetLogic(ViewerRPC::WriteConfigFileRPC,            new WriteConfigFileAction(win));
}

// ****************************************************************************
// Method: ViewerFactory::CreateViewerMessaging
//
// Purpose:
//   Create a new ViewerMessaging object.
//
// Returns:    A new ViewerMessaging object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:11:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessaging *
ViewerFactory::CreateViewerMessaging()
{
    return new ViewerMessaging();
}

// ****************************************************************************
// Method: ViewerFactory::CreateConnectionProgress
//
// Purpose:
//   Create a new connection progress object.
//
// Returns:    A new ViewerMessaging object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:11:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerConnectionProgress *
ViewerFactory::CreateConnectionProgress()
{
    return NULL;
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
ViewerFactory::CreateConnectionPrinter()
{
    if(override_CreateConnectionPrinter != NULL)
        return (*override_CreateConnectionPrinter)();

    return new ViewerConnectionPrinter();
}

// ****************************************************************************
// Method: ViewerFactory::CreateFileServerInterface
//
// Purpose:
//   Creates the file server interface we'll use.
//
// Returns:    A pointer to the new file server interface.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 17:43:58 PDT 2014
//
// Modifications:
//   Brad Whitlock, Tue Oct 21 15:03:47 PDT 2014
//   I made it return NULL.
//
// ****************************************************************************

ViewerFileServerInterface *
ViewerFactory::CreateFileServerInterface()
{
    return NULL;
}

// ****************************************************************************
// Method: ViewerFactory::CreateEngineManagerInterface
//
// Purpose:
//   Creates the engine manager interface we'll use.
//
// Returns:    A pointer to the new engine manager interface.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 17:43:58 PDT 2014
//
// Modifications:
//   Brad Whitlock, Tue Oct 21 15:03:47 PDT 2014
//   I made it return NULL.
//
// ****************************************************************************

ViewerEngineManagerInterface *
ViewerFactory::CreateEngineManagerInterface()
{
    return NULL;
}

// ****************************************************************************
// Method: ViewerFactory::CreateHostProfileSelector
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
ViewerFactory::CreateHostProfileSelector()
{
    return new ViewerHostProfileSelectorNoWin;
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
ViewerFactory::CreateChangeUsername()
{
    return new ViewerChangeUsername;
}

// ****************************************************************************
// Method: ViewerFactory::CreateOperatorPluginManager
//
// Purpose:
//   Creates a new operator plugin manager.
//
// Returns:    A pointer to a new operator plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 12:38:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

OperatorPluginManager *
ViewerFactory::CreateOperatorPluginManager()
{
    return new OperatorPluginManager;
}

// ****************************************************************************
// Method: ViewerFactory::CreatePlotPluginManager
//
// Purpose:
//   Creates a new plot plugin manager.
//
// Returns:    A pointer to a new plot plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 12:38:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

PlotPluginManager *
ViewerFactory::CreatePlotPluginManager()
{
    return new PlotPluginManager;
}

// ****************************************************************************
// Method: ViewerFactory::OverrideCreateConnectionPrinter
//
// Purpose:
//   Set an override callback function for creating a connection printer object.
//
// Arguments:
//   cb : The new creation callback.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:44:54 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerFactory::OverrideCreateConnectionPrinter(ViewerConnectionPrinter *(*cb)(void))
{
    override_CreateConnectionPrinter = cb;
}
