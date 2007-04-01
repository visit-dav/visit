#include <LoadPluginsRPCExecutor.h>
#include <MDServerConnection.h>
#include <LoadPluginsRPC.h>

// ****************************************************************************
// Method: LoadPluginsRPCExecutor::LoadPluginsRPCExecutor
//
// Purpose: 
//   Constructor for the LoadPluginsRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// Modifications:
//   
// ****************************************************************************

LoadPluginsRPCExecutor::LoadPluginsRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: LoadPluginsRPCExecutor::~LoadPluginsRPCExecutor
//
// Purpose: 
//   Destructor for the LoadPluginsRPCExecutor class.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// Modifications:
//   
// ****************************************************************************

LoadPluginsRPCExecutor::~LoadPluginsRPCExecutor()
{
}

// ****************************************************************************
// Method: LoadPluginsRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to load
//   the plugins.
//
// Arguments:
//   s : A pointer to the LoadPluginsRPC that caused this method to
//       be called.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// Modifications:
//
// ****************************************************************************

void
LoadPluginsRPCExecutor::Update(Subject *s)
{
    LoadPluginsRPC *rpc = (LoadPluginsRPC *)s;
    parent->LoadPlugins();
    rpc->SendReply();
}
