// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtDatabase.h>
#include <DBPluginInfoAttributes.h>
#include <GetDBPluginInfoRPCExecutor.h>
#include <GetDBPluginInfoRPC.h>
#include <MDServerConnection.h>
#include <DatabaseException.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: GetDBPluginInfoRPCExecutor::GetDBPluginInfoRPCExecutor
//
// Purpose: 
//   Constructor for the GetDBPluginInfoRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// ****************************************************************************

GetDBPluginInfoRPCExecutor::GetDBPluginInfoRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetDBPluginInfoRPCExecutor::~GetDBPluginInfoRPCExecutor
//
// Purpose: 
//   Destructor for the GetDBPluginInfoRPCExecutor class.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// ****************************************************************************

GetDBPluginInfoRPCExecutor::~GetDBPluginInfoRPCExecutor()
{
}

// ****************************************************************************
// Method: GetDBPluginInfoRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to get the
//   DB options for a plugin type.
//
// Arguments:
//   s : A pointer to the GetDBPluginInfoRPC that caused this method to
//       be called.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// Modifications:
//   Mark C. Miller, Mon Oct  8 13:44:36 PDT 2007
//   Put resulting atts from GetDBPluginInfo into local var so we could
//   properly delete it -- fixed leak.
//
// ****************************************************************************

void
GetDBPluginInfoRPCExecutor::Update(Subject *s)
{
    GetDBPluginInfoRPC *rpc = (GetDBPluginInfoRPC *)s;

    debug2 << "GetDBPluginInfoRPCExecutor::Update\n" << std::flush;

    TRY
    {
        // Either send a successful reply or send an error.
        DBPluginInfoAttributes *atts = parent->GetDBPluginInfo();
        rpc->SendReply(atts);
        delete atts;
    }
    CATCH2(DatabaseException, dbe)
    {
        rpc->SendError(dbe.Message(), dbe.GetExceptionType());
    }
    CATCH2(VisItException, ve)
    {
        rpc->SendError("An unknown error has occurred", ve.GetExceptionType());
    }
    ENDTRY
}
