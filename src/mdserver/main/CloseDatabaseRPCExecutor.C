#include <CloseDatabaseRPCExecutor.h>
#include <MDServerConnection.h>
#include <CloseDatabaseRPC.h>

// ****************************************************************************
// Method: CloseDatabaseRPCExecutor::CloseDatabaseRPCExecutor
//
// Purpose: 
//   Constructor for the CloseDatabaseRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 11:07:28 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

CloseDatabaseRPCExecutor::CloseDatabaseRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: CloseDatabaseRPCExecutor::~CloseDatabaseRPCExecutor
//
// Purpose: 
//   Destructor for the CloseDatabaseRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 11:07:28 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

CloseDatabaseRPCExecutor::~CloseDatabaseRPCExecutor()
{
}

// ****************************************************************************
// Method: CloseDatabaseRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to close
//   the open database.
//
// Arguments:
//   s : A pointer to the CloseDatabaseRPC that caused this method to
//       be called.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 11:07:28 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
CloseDatabaseRPCExecutor::Update(Subject *s)
{
    CloseDatabaseRPC *rpc = (CloseDatabaseRPC *)s;
    parent->CloseDatabase();
    rpc->SendReply();
}
