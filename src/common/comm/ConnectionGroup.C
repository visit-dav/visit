#include <ConnectionGroup.h>
#include <Connection.h>

#include <string.h>    // memset
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <strings.h>   // bzero by way of FD_ZERO
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// ****************************************************************************
// Method: ConnectionGroup::ConnectionGroup
//
// Purpose: 
//   Constructor for the ConnectionGroup class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 11:50:50 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

ConnectionGroup::ConnectionGroup()
{
    nConnections = 0;
    for(int i = 0; i < 10; ++i)
        needsRead[i] = false;
}

// ****************************************************************************
// Method: ConnectionGroup::~ConnectionGroup
//
// Purpose: 
//   Destructor for the ConnectionGroup class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 11:51:13 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

ConnectionGroup::~ConnectionGroup()
{
}

// ****************************************************************************
// Method: ConnectionGroup::AddConnection
//
// Purpose: 
//   Adds a connection to the group.
//
// Arguments:
//   conn : A pointer to a connection object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 11:51:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ConnectionGroup::AddConnection(Connection *conn)
{
    if(conn && (nConnections < 10))
        connections[nConnections++] = conn;
}

// ****************************************************************************
// Method: ConnectionGroup::NeedsRead
//
// Purpose: 
//   Returns whether or not a connection in the group needs to be read.
//
// Arguments:
//   index : The index of the connection we're checking.
//
// Returns:    Whether or not a connection in the group needs to be read.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 11:51:58 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ConnectionGroup::NeedsRead(int index) const
{
    return (index >= 0 && index < 10) ? needsRead[index] : false;
}

// ****************************************************************************
// Method: ConnectionGroup::CheckInput
//
// Purpose: 
//   Checks the connections to determine if any have input that needs to
//   be read.
//
// Note:       This method blocks until some of the connections in the 
//             group have input to read. Note that the way this is written
//             now, it will only work for SocketConnections since they are
//             presently the only type of connection whose GetDescriptor()
//             method returns anything other than -1. This method also assumes
//             socket coding in its use of select.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 11:52:42 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ConnectionGroup::CheckInput()
{
    //
    // Build the read set that we'll use for the select call.
    //
    fd_set readSet;
    FD_ZERO(&readSet);
    int i;
    DESCRIPTOR max_descriptor = 0;
    for(i = 0; i < nConnections; ++i)
    {
        DESCRIPTOR descriptor = connections[i]->GetDescriptor();
        //
        // Only consider the connection if it has a valid file descriptor.
        //
        if(descriptor != -1)
        {
            if (descriptor > max_descriptor)
                max_descriptor = descriptor;
            FD_SET(descriptor, &readSet);
        }

        needsRead[i] = false;
    }

    //
    // Get a list of how many file descriptors need to be read. If there is
    // an error, return false.
    //
    if(select(max_descriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
              (struct timeval*)NULL) < 0)
    {
        return false;
    }

    //
    // Test all of the connections that had valid file descriptors to see
    // if they need to be read.
    //
    for(i = 0; i < nConnections; ++i)
    {
        if(connections[i]->GetDescriptor() != -1)
            needsRead[i] = FD_ISSET(connections[i]->GetDescriptor(), &readSet);
    }

    return true;
}
