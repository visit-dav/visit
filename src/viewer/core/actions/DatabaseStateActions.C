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
