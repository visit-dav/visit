#include <ExpandPathRPCExecutor.h>
#include <ExpandPathRPC.h>
#include <DebugStream.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: ExpandPathRPCExecutor::ExpandPathRPCExecutor
//
// Purpose: 
//   Constructor for the ExpandPathRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:35:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ExpandPathRPCExecutor::ExpandPathRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: ExpandPathRPCExecutor::~ExpandPathRPCExecutor
//
// Purpose: 
//   Destructor for the ExpandPathRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:35:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ExpandPathRPCExecutor::~ExpandPathRPCExecutor()
{
}

// ****************************************************************************
// Method: ExpandPathRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to expand
//   the specified path.
//
// Arguments:
//   s : A pointer to the ExpandPathRPC that caused this method to
//       be called.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:35:20 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
ExpandPathRPCExecutor::Update(Subject *s)
{
    ExpandPathRPC *rpc = (ExpandPathRPC *)s;
#ifdef DEBUG
    debug2 << "ExpandPathRPCExecutor::Update: dir="
           << rpc->GetPath().c_str() << "\n"; debug2.flush();
#endif
    // Either send a successful reply or send an error.
    ExpandPathRPC::PathName path;
    path.name = parent->ExpandPath(rpc->GetPath());
    rpc->SendReply(&path);
}
