// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <DatabaseStateActions.h>

#include <ViewerEngineManagerInterface.h>
#include <ViewerFileServerInterface.h>
#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetMaterialAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetMaterialAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetMaterialAttributesAction::Execute()
{
    GetViewerStateManager()->SetClientMaterialAttsFromDefault();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetMeshManagementAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetMeshManagementAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetMeshManagementAttributesAction::Execute()
{
    GetViewerStateManager()->SetClientMeshManagementAttsFromDefault();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetBackendTypeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetBackendTypeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetBackendTypeAction::Execute()
{
    windowMgr->SetBackendType(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetCreateMeshQualityExpressionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetCreateMeshQualityExpressionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetCreateMeshQualityExpressionsAction::Execute()
{
    windowMgr->SetCreateMeshQualityExpressions(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetCreateTimeDerivativeExpressionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetCreateTimeDerivativeExpressionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetCreateTimeDerivativeExpressionsAction::Execute()
{
    windowMgr->SetCreateTimeDerivativeExpressions(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetCreateVectorMagnitudeExpressionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetCreateVectorMagnitudeExpressionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetCreateVectorMagnitudeExpressionsAction::Execute()
{
    windowMgr->SetCreateVectorMagnitudeExpressions(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultFileOpenOptionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultFileOpenOptionsRPC
//
// Notes:
//    Makes the current state file open options the default for all
//    future opening actions by broadcasting them to the existing
//    metadata servers and engines, and having the file server and
//    engine manager keep track of them and send them to new engines/
//    mdservers.
//
// Programmer: Jeremy Meredith
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultFileOpenOptionsAction::Execute()
{
    GetViewerFileServer()->BroadcastUpdatedFileOpenOptions();
    GetViewerEngineManager()->UpdateDefaultFileOpenOptions(GetViewerState()->GetFileOpenOptions());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultMaterialAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultMaterialAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultMaterialAttributesAction::Execute()
{
    GetViewerStateManager()->SetDefaultMaterialAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultMeshManagementAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultMeshManagementAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultMeshManagementAttributesAction::Execute()
{
    GetViewerStateManager()->SetDefaultMeshManagementAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetMaterialAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetMaterialAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetMaterialAttributesAction::Execute()
{
    // Do nothing; there is only a global copy, and nothing
    // is regenerated automatically just yet
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetMeshManagementAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetMeshManagementAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetMeshManagementAttributesAction::Execute()
{
    // Do nothing; there is only a global copy, and nothing
    // is regenerated automatically just yet
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPrecisionTypeAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetPrecisionTypeRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetPrecisionTypeAction::Execute()
{
    windowMgr->SetPrecisionType(args.GetIntArg1());
}


///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetRemoveDuplicateNodesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::RemoveDuplicateNodesRPC
//
// Programmer: Kathleen Biagas
// Creation:   December 18, 2014
//
// Modifications:
//
// ****************************************************************************

void
SetRemoveDuplicateNodesAction::Execute()
{
    windowMgr->SetRemoveDuplicateNodes(args.GetBoolFlag());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetTreatAllDBsAsTimeVaryingAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetTreatAllDBsAsTimeVaryingRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SetTreatAllDBsAsTimeVaryingAction::Execute()
{
    windowMgr->SetTreatAllDBsAsTimeVarying(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetTryHarderCyclesTimesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetTryHarderCyclesTimesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetTryHarderCyclesTimesAction::Execute()
{
    windowMgr->SetTryHarderCyclesTimes(args.GetIntArg1());
}
