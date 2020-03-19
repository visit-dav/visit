// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <CopyActions.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CopyAnnotationsToWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CopyAnnotationsToWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CopyAnnotationsToWindowAction::Execute()
{
    int from = args.GetWindowLayout();
    int to = args.GetWindowId();
    windowMgr->CopyAnnotationsToWindow(from-1, to-1);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CopyLightingToWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CopyLightingToWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CopyLightingToWindowAction::Execute()
{
    int from = args.GetWindowLayout();
    int to = args.GetWindowId();
    windowMgr->CopyLightingToWindow(from-1, to-1);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CopyPlotsToWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CopyPlotsToWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CopyPlotsToWindowAction::Execute()
{
    int from = args.GetWindowLayout();
    int to = args.GetWindowId();
    windowMgr->CopyPlotListToWindow(from-1, to-1);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CopyViewToWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CopyViewToWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CopyViewToWindowAction::Execute()
{
    int from = args.GetWindowLayout();
    int to = args.GetWindowId();
    windowMgr->CopyViewToWindow(from-1, to-1);
}
