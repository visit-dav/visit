#include <GetPluginErrorsRPCExecutor.h>
#include <MDServerConnection.h>
#include <GetPluginErrorsRPC.h>

// ****************************************************************************
// Method: GetPluginErrorsRPCExecutor::GetPluginErrorsRPCExecutor
//
// Purpose: 
//   Constructor for the GetPluginErrorsRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// ****************************************************************************

GetPluginErrorsRPCExecutor::GetPluginErrorsRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetPluginErrorsRPCExecutor::~GetPluginErrorsRPCExecutor
//
// Purpose: 
//   Destructor for the GetPluginErrorsRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// ****************************************************************************

GetPluginErrorsRPCExecutor::~GetPluginErrorsRPCExecutor()
{
}

// ****************************************************************************
// Method: GetPluginErrorsRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to return
//   errors generated during plugin initialization.
//
// Arguments:
//   s : A pointer to the GetPluginErrorsRPC that caused this method to
//       be called.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//
// ****************************************************************************

void
GetPluginErrorsRPCExecutor::Update(Subject *s)
{
    GetPluginErrorsRPC *rpc = (GetPluginErrorsRPC *)s;
    GetPluginErrorsRPC::PluginErrors err;
    err.errorString = parent->GetPluginErrors();
    rpc->SendReply(&err);
}
