#include <ConnectSimRPC.h>

// ****************************************************************************
//  Method: ConnectSimRPC::ConnectSimRPC
//
//  Purpose: 
//    Constructor for the ConnectSimRPC class.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

ConnectSimRPC::ConnectSimRPC() : BlockingRPC("s*sis"), launchArgs()
{
}

// ****************************************************************************
//  Method: ConnectSimRPC::~ConnectSimRPC
//
//  Purpose: 
//    Destructor for the ConnectSimRPC class.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

ConnectSimRPC::~ConnectSimRPC()
{
}

// ****************************************************************************
//  Method: ConnectSimRPC::operator()
//
//  Purpose: 
//    This is the invokation method for this function object. It copies the
//    arguments into local attributes and calls RPC's Execute method.
//
//  Arguments:
//    args : The arguments to launch the program.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

void
ConnectSimRPC::operator()(const stringVector &args,
                          const std::string  &host,
                          int port,
                          const std::string  &securityKey)
{
    launchArgs     = args;
    simHost        = host;
    simPort        = port;
    simSecurityKey = securityKey;
    Execute();
}

// ****************************************************************************
//  Method: ConnectSimRPC::SelectAll
//
//  Purpose: 
//    Gets the addresses of the local attributes so the attributes can be
//    transmitted over the network as AttributeGroups.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

void
ConnectSimRPC::SelectAll()
{
    Select(0, (void *)&launchArgs);
    Select(1, (void *)&simHost);
    Select(2, (void *)&simPort);
    Select(3, (void *)&simSecurityKey);
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetLaunchArgs
//
//  Purpose: 
//    Returns the launch arguments vector.
//
//  Returns:    The launch arguments vector.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

const stringVector &
ConnectSimRPC::GetLaunchArgs() const
{
    return launchArgs;
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetSimHost
//
//  Purpose: 
//    Returns the host name the simulation is listening on.
//
//  Returns:    The host name.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

const std::string &
ConnectSimRPC::GetSimHost() const
{
    return simHost;
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetSimPort
//
//  Purpose: 
//    Returns the port number the simulation is listening on.
//
//  Returns:    The port number.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

int
ConnectSimRPC::GetSimPort() const
{
    return simPort;
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetSimSecurityKey
//
//  Purpose: 
//    Returns the security key simulation is waiting for.
//
//  Returns:    The security key.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2005
//
//  Modifications:
//
// ****************************************************************************

const std::string &
ConnectSimRPC::GetSimSecurityKey() const
{
    return simSecurityKey;
}

