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
// ****************************************************************************

ConnectRPC::ConnectRPC() : NonBlockingRPC("ssiii"), connectHost(""),
    connectKey("")
{
    connectPort = 0;
    connectNRead = 0;
    connectNWrite = 0;
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
//   host   : The host on which the remote process is running.
//   key    : The security key that the remote process must return.
//   port   : The port used to connect to the remote process.
//   nread  : The number of read sockets to open.
//   nwrite : The number of write sockets to open.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:50:57 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ConnectRPC::operator()(const std::string &host, const std::string &key,
                       int port, int nread, int nwrite)
{
    connectHost = host;
    connectKey = key;
    connectPort = port;
    connectNRead = nread;
    connectNWrite = nwrite;
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
// ****************************************************************************

void
ConnectRPC::SelectAll()
{
    Select(0, (void *)&connectHost);
    Select(1, (void *)&connectKey);
    Select(2, (void *)&connectPort);
    Select(3, (void *)&connectNRead);
    Select(4, (void *)&connectNWrite);
}

// ****************************************************************************
// Method: ConnectRPC::GetHost
//
// Purpose: 
//   Returns the host.
//
// Arguments:
//
// Returns:    Returns the host.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:53:50 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const std::string &
ConnectRPC::GetHost() const
{
    return connectHost;
}

// ****************************************************************************
// Method: ConnectRPC::GetKey
//
// Purpose: 
//   Returns the security key.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 26 16:01:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

const std::string &
ConnectRPC::GetKey() const
{
    return connectKey;
}

// ****************************************************************************
// Method: ConnectRPC::GetPort
//
// Purpose: 
//   Returns the port used in the RPC call.
//
// Arguments:
//
// Returns:     Returns the port used in the RPC call.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:54:10 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
ConnectRPC::GetPort() const
{
    return connectPort;
}

// ****************************************************************************
// Method: ConnectRPC::GetNRead
//
// Purpose: 
//   Returns the number of read sockets.
//
// Arguments:
//
// Returns:    Returns the number of read sockets.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:54:31 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
ConnectRPC::GetNRead() const
{
    return connectNRead;
}

// ****************************************************************************
// Method: ConnectRPC::GetNWrite
//
// Purpose: 
//   Returns the number of write sockets.
//
// Arguments:
//
// Returns:    Returns the number of write sockets.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:54:55 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
ConnectRPC::GetNWrite() const
{
     return connectNWrite;
}
