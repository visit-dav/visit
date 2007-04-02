/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
