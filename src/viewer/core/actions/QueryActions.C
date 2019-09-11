// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QueryActions.h>

#include <ViewerEngineManagerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerQueryManager.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

ViewerQueryManager *
QueryActionBase::GetQueryManager()
{
    return ViewerQueryManager::Instance();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ConstructDataBinningAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ConstructDataBinningRPC
//
// Programmer: Brad Whitlock (based on Hank's original)
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ConstructDataBinningAction::Execute()
{
    //
    // Perform the RPC.
    //
    ViewerPlotList *plist = GetWindow()->GetPlotList();
    intVector plotIDs;
    plist->GetActivePlotIDs(plotIDs);
    if (plotIDs.size() <= 0)
    {
        GetViewerMessaging()->Error(
            TR("To construct a data binning, you must have an active "
               "plot.  No data binning was created."));
        return;
    }
    if (plotIDs.size() > 1)
    {
        GetViewerMessaging()->Message(
           TR("Only one data binning can be created at a time.  VisIt is using "
              "the first active plot."));
    }

    ViewerPlot *plot = plist->GetPlot(plotIDs[0]);
    const EngineKey   &engineKey = plot->GetEngineKey();
    int networkId = plot->GetNetworkID();
    TRY
    {
        if (GetViewerEngineManager()->ConstructDataBinning(engineKey, networkId))
        {
            GetViewerMessaging()->Message(TR("Created data binning"));
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to create data binning"));
        }
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            ViewerText("(%1): %2\n").
            arg(e.GetExceptionType()).
            arg(e.Message()));
    }
    ENDTRY
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: GetQueryParametersAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::GetQueryParametersRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
GetQueryParametersAction::Execute()
{
    GetQueryManager()->GetQueryParameters(args.GetQueryName());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ProcessExpressionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ProcessExpressionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ProcessExpressionsAction::Execute()
{
    // NOT IMPLEMENTED YET
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: QueryAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::QueryRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
QueryAction::Execute()
{
    GetQueryManager()->Query(args.GetQueryParams());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RenamePickLabelAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::RenamePickLabelRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
RenamePickLabelAction::Execute()
{
    GetWindow()->RenamePickLabel(args.GetStringArg1(), args.GetStringArg2());
}


///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetLineoutColorAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetLineoutColorRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetLineoutColorAction::Execute()
{
    GetQueryManager()->ResetLineoutColor(); 
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetPickAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetPickAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetPickAttributesAction::Execute()
{
    GetQueryManager()->SetPickAttsFromDefault(); 
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetPickLetterAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetPickLetterRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetPickLetterAction::Execute()
{
    GetQueryManager()->ResetPickLetter();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetQueryOverTimeAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetQueryOverTimeAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetQueryOverTimeAttributesAction::Execute()
{
    GetQueryManager()->SetQueryOverTimeAttsFromDefault(); 
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultPickAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultPickAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultPickAttributesAction::Execute()
{
    GetQueryManager()->SetDefaultPickAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultQueryOverTimeAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultQueryOverTimeAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultQueryOverTimeAttributesAction::Execute()
{
    GetQueryManager()->SetDefaultQueryOverTimeAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetGlobalLineoutAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetGlobalLineoutAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetGlobalLineoutAttributesAction::Execute()
{
    GetQueryManager()->SetGlobalLineoutAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPickAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetPickAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetPickAttributesAction::Execute()
{
    GetQueryManager()->SetPickAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetQueryFloatFormatAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetQueryFloatFormatRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetQueryFloatFormatAction::Execute()
{
    GetQueryManager()->SetQueryFloatFormat(args.GetStringArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetQueryOverTimeAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetQueryOverTimeAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetQueryOverTimeAttributesAction::Execute()
{
    GetQueryManager()->SetQueryOverTimeAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetSuppressMessagesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetSuppressMessagesRPC
//
// Programmer: Brad Whitlock (from Cyrus' method)
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetSuppressMessagesAction::Execute()
{
    bool value = (bool) args.GetIntArg1();
    if(value)
        GetViewerMessaging()->EnableMessageSuppression();
    else
        GetViewerMessaging()->DisableMessageSuppression();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SuppressQueryOutputAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SuppressQueryOutputRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SuppressQueryOutputAction::Execute()
{
    GetQueryManager()->SuppressQueryOutput(args.GetBoolFlag());
}
