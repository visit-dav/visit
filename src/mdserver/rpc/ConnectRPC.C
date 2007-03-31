#include <ConnectRPC.h>

// ****************************************************************************
// Method: ConnectRPC::ConnectRPC
//
// Purpose: 
//   Constructor for the ConnectRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:50:14 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 15:59:41 PST 2002
//   I added connectKey.
//
//   Brad Whitlock, Mon Jun 16 13:41:38 PST 2003
//   I replaced all of the fields with the arguments field.
//
// ****************************************************************************

ConnectRPC::ConnectRPC() : NonBlockingRPC("s*"), arguments()
{
}

// ****************************************************************************
// Method: ConnectRPC::~ConnectRPC
//
// Purpose: 
//   Destructor for the ConnectRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:50:33 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConnectRPC::~ConnectRPC()
{
}

// ****************************************************************************
// Method: ConnectRPC::operator()
//
// Purpose: 
//   This is the invokation method for this function object. It copies the
//   arguments into local attributes and calls RPC's Execute method.
//
// Arguments:
//   args : The arguments that describe how to connect to the client.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:50:57 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Jun 16 13:42:24 PST 2003
//   I made it use a single string vector.
//
// ****************************************************************************

void
ConnectRPC::operator()(const stringVector &args)
{
    arguments = args;
    Execute();
}

// ****************************************************************************
// Method: ConnectRPC::SelectAll
//
// Purpose: 
//   Gets the addresses of the local attributes so the attributes can be
//   transmitted over the network as AttributeGroups.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:53:00 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 16:00:55 PST 2002
//   I added connectKey.
//
//   Brad Whitlock, Mon Jun 16 13:43:01 PST 2003
//   I replaced all of the fields with a single string vector field.
//
// ****************************************************************************

void
ConnectRPC::SelectAll()
{
    Select(0, (void *)&arguments);
}

// ****************************************************************************
// Method: ConnectRPC::GetArguments
//
// Purpose: 
//   Returns the arguments.
//
// Arguments:
//
// Returns:    Returns the arguments.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:54:55 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const stringVector &
ConnectRPC::GetArguments() const
{
     return arguments;
}
