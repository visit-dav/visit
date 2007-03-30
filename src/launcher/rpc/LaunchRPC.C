#include <LaunchRPC.h>

// ****************************************************************************
// Method: LaunchRPC::LaunchRPC
//
// Purpose: 
//   Constructor for the LaunchRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:29:10 PST 2003
//
// Modifications:
//
// ****************************************************************************

LaunchRPC::LaunchRPC() : BlockingRPC("s*"), launchArgs()
{
}

// ****************************************************************************
// Method: LaunchRPC::~LaunchRPC
//
// Purpose: 
//   Destructor for the LaunchRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:50:33 PST 2000
//
// Modifications:
//   
// ****************************************************************************

LaunchRPC::~LaunchRPC()
{
}

// ****************************************************************************
// Method: LaunchRPC::operator()
//
// Purpose: 
//   This is the invokation method for this function object. It copies the
//   arguments into local attributes and calls RPC's Execute method.
//
// Arguments:
//   args : The arguments to launch the program.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:30:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
LaunchRPC::operator()(const stringVector &args)
{
    launchArgs = args;
    Execute();
}

// ****************************************************************************
// Method: LaunchRPC::SelectAll
//
// Purpose: 
//   Gets the addresses of the local attributes so the attributes can be
//   transmitted over the network as AttributeGroups.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:30:34 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
LaunchRPC::SelectAll()
{
    Select(0, (void *)&launchArgs);
}

// ****************************************************************************
// Method: LaunchRPC::GetLaunchArgs
//
// Purpose: 
//   Returns the launch arguments vector.
//
// Returns:    The launch arguments vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:44:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const stringVector &
LaunchRPC::GetLaunchArgs() const
{
    return launchArgs;
}

