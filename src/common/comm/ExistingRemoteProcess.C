/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ExistingRemoteProcess.h>

// ****************************************************************************
// Method: ExistingRemoteProcess::ExistingRemoteProcess
//
// Purpose: 
//   Constructor for the ExistingRemoteProcess class.
//
// Arguments:
//    rProgram : The name of the remote program to execute.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 15:20:31 PST 2000
//
// Modifications:
//
// ****************************************************************************

ExistingRemoteProcess::ExistingRemoteProcess(const std::string &rProgram) : 
    RemoteProcess(rProgram)
{
    connectCallback = NULL;
    connectCallbackData = NULL;
}

// ****************************************************************************
// Method: ExistingRemoteProcess::~ExistingRemoteProcess
//
// Purpose: 
//   Destructor for the ExistingRemoteProcess class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 15:19:54 PST 2000
//
// Modifications:
//
// ****************************************************************************

ExistingRemoteProcess::~ExistingRemoteProcess()
{
    // nothing here.
}

// ****************************************************************************
// Method: ExistingRemoteProcess::Open
//
// Purpose: 
//   Opens sockets and launches a remote process using ssh.
//
// Arguments:
//   rHost    : The remote host to run on.
//   numRead  : The number of read sockets to create to the remote process.
//   numWrite : The number of write sockets to create to the remote process.
//   createAsThoughLocal : Forces local process creation.
//
// Returns:    
//   true if it worked, false if it did not.
//
// Note:       
//   numRead and numWrite cannot both be 0, otherwise the method
//   will return without creating the remote process.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 15:21:41 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 19 11:36:59 PDT 2002
//   I added the securityKey argument.
//
//   Brad Whitlock, Mon May 5 13:59:08 PST 2003
//   I made all of the command line options be passed to the callback
//   function so launching processes can be done with more flexibility.
//
//   Jeremy Meredith, Thu Oct  9 14:04:18 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
//   Jeremy Meredith, Thu May 24 11:10:15 EDT 2007
//   Added SSH tunneling argument; pass it along to CreateCommandLine.
//
// ****************************************************************************

bool
ExistingRemoteProcess::Open(const std::string &rHost,
                            HostProfile::ClientHostDetermination chd,
                            const std::string &clientHostName,
                            bool manualSSHPort,
                            int sshPort, bool useTunneling,
                            int numRead, int numWrite,
                            bool createAsThoughLocal)
{
    // Start making the connections and start listening.
    if(!StartMakingConnection(rHost, numRead, numWrite))
        return false;

    // Add all of the relevant command line arguments to a vector of strings.
    stringVector commandLine;
    CreateCommandLine(commandLine, rHost,
                      chd, clientHostName, manualSSHPort, sshPort,useTunneling,
                      numRead, numWrite,
                      createAsThoughLocal);

    //
    // Call a user-defined callback function to launch the process.
    //
    if(connectCallback != NULL)
    {
        (*connectCallback)(rHost, commandLine, connectCallbackData);
    }

    // Finish the connections.
    FinishMakingConnection(numRead, numWrite);

    return true;
}

// ****************************************************************************
// Method: ExistingRemoteProcess::SetConnectCallback
//
// Purpose: 
//   Sets the connect callback that is called just before the loops to listen
//   for the remote process trying to connect back.
//
// Arguments:
//   cb  : The address of the connect callback function.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 15:22:18 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ExistingRemoteProcess::SetConnectCallback(ConnectCallback *cb)
{
    connectCallback = cb;
}

// ****************************************************************************
// Method: ExistingRemoteProcess::SetConnectCallbackData
//
// Purpose: 
//   Sets the data that is passed to the connection callback.
//
// Arguments:
//   data  : The address of the data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 15:22:18 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ExistingRemoteProcess::SetConnectCallbackData(void *data)
{
    connectCallbackData = data;
}
