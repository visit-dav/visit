#include <ChangeDirectoryRPCExecutor.h>
#include <ChangeDirectoryRPC.h>
#include <DebugStream.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: ChangeDirectoryRPCExecutor::ChangeDirectoryRPCExecutor
//
// Purpose: 
//   Constructor for the ChangeDirectoryRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

ChangeDirectoryRPCExecutor::ChangeDirectoryRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: ChangeDirectoryRPCExecutor::~ChangeDirectoryRPCExecutor
//
// Purpose: 
//   Destructor for the ChangeDirectoryRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

ChangeDirectoryRPCExecutor::~ChangeDirectoryRPCExecutor()
{
}

// ****************************************************************************
// Method: ChangeDirectoryRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to change
//   the current directory.
//
// Arguments:
//   s : A pointer to the ChangeDirectoryRPC that caused this method to
//       be called.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 17 17:05:31 PST 2000
//   Modified the routine to use methods in MDServerConnection.
//
// ****************************************************************************

void
ChangeDirectoryRPCExecutor::Update(Subject *s)
{
    ChangeDirectoryRPC *rpc = (ChangeDirectoryRPC *)s;
#ifdef DEBUG
    debug2 << "ChangeDirectoryRPCExecutor::Update: dir="
           << rpc->GetDirectory().c_str() << "\n"; debug2.flush();
#endif
    // Either send a successful reply or send an error.
    if(parent->ChangeDirectory(rpc->GetDirectory()) == 0)
        rpc->SendReply();
    else
        rpc->SendError();
}
