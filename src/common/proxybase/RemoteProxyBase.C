#include <RemoteProxyBase.h>
#include <RemoteProcess.h>
#include <ExistingRemoteProcess.h>

// ****************************************************************************
// Method: RemoteProxyBase::RemoteProxyBase
//
// Purpose: 
//   Constructor for the RemoteProxyBase class.
//
// Arguments:
//   compName : The name of the component.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:56:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

RemoteProxyBase::RemoteProxyBase(const std::string &compName) :
    componentName(compName), xfer(), quitRPC(), remoteUserName(), argv()
{
    component = 0;
    nWrite = nRead = 1;
    progressCallback = 0;
    progressCallbackData = 0;
}

// ****************************************************************************
// Method: RemoteProxyBase::~RemoteProxyBase
//
// Purpose: 
//   Destructor for the RemoteProxyBase class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:56:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

RemoteProxyBase::~RemoteProxyBase()
{
    delete component;
}

// ****************************************************************************
// Method: RemoteProxyBase::Create
//
// Purpose: 
//   Creates the remote process and hooks up RPCs so that it can be used.
//
// Arguments:
//   hostName        : The host where the proxy should be created.
//   connectCallback : A callback that can be used to launch the component.
//   data            : Data to be used with the connect callback.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:57:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::Create(const std::string &hostName,
    ConnectCallback *connectCallback, void *data,
    bool createAsThoughLocal)
{
    // Create a remote process object for the remote component.
    if(connectCallback == NULL)
    {
        component = new RemoteProcess(GetVisItString());
    }
    else
    {
        ExistingRemoteProcess *p = 
            new ExistingRemoteProcess(GetVisItString());
        p->SetConnectCallback(connectCallback);
        p->SetConnectCallbackData(data);
        component = p;
    }

    // Add the component's name as an extra argument.
    component->AddArgument(componentName);

    // Set the engine's launch progress callback.
    component->SetProgressCallback(progressCallback, progressCallbackData);

    // Add extra arguments to the remote process object.
    AddExtraArguments();

    //
    // Open the remote component.
    //
    component->Open(hostName, nRead, nWrite, createAsThoughLocal);

    //
    // Hook up the sockets to the xfer object.
    //
    xfer.SetOutputConnection(component->GetReadConnection());
    xfer.SetInputConnection(component->GetWriteConnection());
    xfer.Add(&quitRPC);

    //
    // Set up the RPC's for the remote component.
    //
    SetupComponentRPCs();

    //
    // List the objects that were hooked up. 
    //
    xfer.ListObjects();
}

// ****************************************************************************
// Method: RemoteProxyBase::Close
//
// Purpose: 
//   Tells the remote component to close.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:59:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::Close()
{
    quitRPC(true);
}

// ****************************************************************************
// Method: RemoteProxyBase::SetRemoteUserName
//
// Purpose: 
//   Sets the username to use on the remote machine.
//
// Arguments:
//   rName : The remote user name.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:59:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::SetRemoteUserName(const std::string &rName)
{
    remoteUserName = rName;
}

// ****************************************************************************
// Method: RemoteProxyBase::SetProgressCallback
//
// Purpose: 
//   This method sets a progress callback that is called while the remote
//   component is being launched.
//
// Arguments:
//   cb   : The progress callback.
//   data : Data to pass to the progress callback.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:59:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::SetProgressCallback(bool (*cb)(void *, int), void *data)
{
    progressCallback = cb;
    progressCallbackData = data;
}

// ****************************************************************************
// Method: RemoteProxyBase::AddArgument
//
// Purpose: 
//   Adds an argument to the list of arguments to be passed to the component.
//
// Arguments:
//   arg : The argument to be added to the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:00:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::AddArgument(const std::string &arg)
{
    argv.push_back(arg);
}

// ****************************************************************************
// Method: RemoteProxyBase::Parallel
//
// Purpose: 
//   Returns whether the application on the other side is parallel.
//
// Returns:    false.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:15:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
RemoteProxyBase::Parallel() const
{
    return false;
}

// ****************************************************************************
// Method: RemoteProxyBase::GetVisItString
//
// Purpose: 
//   Returns the path to the visit command on the remote computer.
//
// Returns:    The path to the visit command.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:01:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
RemoteProxyBase::GetVisItString() const
{ 
    // The -dir flag means that the visit script is not in our path, so we
    // must prepend it to the name of the visit script.  Do that check now.
    std::string visitString = "visit";
    for (int i = 0 ; i < argv.size() ; ++i)
    {
        if (argv[i] == "-dir" && (i + 1) < argv.size())
        {
            const std::string &dirArg = argv[i + 1];
            visitString = dirArg;
            if (dirArg[dirArg.size() - 1] != '/')
                visitString += "/";
            visitString += "bin/visit";
            ++i;
        }
    }

    return visitString;
}

// ****************************************************************************
// Method: RemoteProxyBase::AddExtraArguments
//
// Purpose: 
//   Adds extra arguments to the component before it is launched.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:03:15 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 10 14:47:35 PST 2003
//   Let -dir pass through.
//
// ****************************************************************************

void
RemoteProxyBase::AddExtraArguments()
{
    //
    // Set the remote user name.
    //
    if(remoteUserName.size() > 0)
        component->SetRemoteUserName(remoteUserName);

    //
    // Add any extra arguments to the component before opening it.
    //
    for(int i = 0; i < argv.size(); ++i)
        component->AddArgument(argv[i]);
}
