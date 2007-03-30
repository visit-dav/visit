#include <DebugStream.h>
#include <GetDirectoryRPCExecutor.h>
#include <GetDirectoryRPC.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: GetDirectoryRPCExecutor::GetDirectoryRPCExecutor
//
// Purpose:
//   Constructor for the GetDirectoryRPCExecutor class.
//
// Arguments:
//   parent_ : The object that created this RPC executor.
//   s       : A pointer to the RPC that will use this RPC executor.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GetDirectoryRPCExecutor::GetDirectoryRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetDirectoryRPCExecutor::~GetDirectoryRPCExecutor
//
// Purpose:
//   Destructor for the GetDirectoryRPCExecutor class.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GetDirectoryRPCExecutor::~GetDirectoryRPCExecutor()
{
}

// ****************************************************************************
// Method: GetDirectoryRPCExecutor::Update
//
// Purpose:
//   This method executes the RPC's work.
//
// Arguments:
//   s : A pointer to the RPC that initiated the call to this method.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
GetDirectoryRPCExecutor::Update(Subject *s)
{
     GetDirectoryRPC *rpc = (GetDirectoryRPC *)s;
#ifdef DEBUG
     debug2 << "GetDirectoryRPCExecutor::Update\n"; debug2.flush();
#endif
     GetDirectoryRPC::DirectoryName dir;
     dir.name = parent->GetCurrentWorkingDirectory();
     rpc->SendReply(&dir);
}
