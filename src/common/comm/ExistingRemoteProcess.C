// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ExistingRemoteProcess.h>
#include <DebugStream.h>

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
//   profile  : The machine profile of the remote host to run on.
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
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
//
//   Eric Brugger, Mon May  2 16:45:21 PDT 2011
//   I added the ability to use a gateway machine when connecting to a
//   remote host.
//
//   Eric Brugger, Mon Sep 26 17:02:16 PDT 2011
//   I modified the remote launching to pass the remote user name to the
//   ssh command to the gateway machine instead of to the ssh command to
//   the remote machine.
//
//   Brad Whitlock, Fri Jan 13 15:13:12 PST 2012
//   I updated the signature for CreateCommandLine.
//
//   Brad Whitlock, Tue Jun  5 15:54:12 PDT 2012
//   Pass in MachineProfile.
//
//   Brad Whitlock, Mon Nov  5 10:04:45 PST 2012
//   Add printing of arguments. Don't pass host to StartMakingConnection since
//   it's no longer needed. ExistingRemoteProcess will not check for a host's
//   validity anymore.
//
// ****************************************************************************

bool
ExistingRemoteProcess::Open(const MachineProfile &profile,
                            int numRead, int numWrite,
                            bool createAsThoughLocal)
{
    // Write the arguments to the debug log.
    const char *mName = "ExistingRemoteProcess::Open: ";
    debug5 << mName << "Called with (profile";
    debug5 << ", numRead=" << numRead;
    debug5 << ", numWrite=" << numWrite;
    debug5 << ", createAsThoughLocal=" << (createAsThoughLocal?"true":"false");
    debug5 << ") where profile is:" << endl;
    if(DebugStream::Level5())
        profile.Print(DebugStream::Stream5());

    // Start making the connections and start listening.
    if(!StartMakingConnection(profile.GetHost(), numRead, numWrite))
        return false;

    // Add all of the relevant command line arguments to a vector of strings.
    stringVector commandLine;
    CreateCommandLine(commandLine, profile, numRead, numWrite);

    debug5 << "ExistingRemoteProcess::Open: commandLine = {" << endl;
    for(size_t i = 0; i < commandLine.size(); ++i)
        debug5 << "\t" << commandLine[i] << endl;
    debug5 << "}" << endl;

    //
    // Call a user-defined callback function to launch the process.
    //
    if(connectCallback != NULL)
    {
        (*connectCallback)(profile.GetHost(), commandLine, connectCallbackData);
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
