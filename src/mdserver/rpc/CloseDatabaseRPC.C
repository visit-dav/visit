#include <CloseDatabaseRPC.h>

// ****************************************************************************
// Method: CloseDatabaseRPC::CloseDatabaseRPC
//
// Purpose: 
//   This is the constructor for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

CloseDatabaseRPC::CloseDatabaseRPC() : BlockingRPC("")
{
}

// ****************************************************************************
// Method: CloseDatabaseRPC::~CloseDatabaseRPC
//
// Purpose: 
//   This is the destructor for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

CloseDatabaseRPC::~CloseDatabaseRPC()
{
}

// ****************************************************************************
// Method: CloseDatabaseRPC::operator()
//
// Purpose: 
//   This is the invokation method for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
CloseDatabaseRPC::operator()()
{
    Execute();
}

// ****************************************************************************
// Method: CloseDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all components of the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:58:42 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
CloseDatabaseRPC::SelectAll()
{
}
