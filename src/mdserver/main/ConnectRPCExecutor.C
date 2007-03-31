#include <stdio.h>               // for sprintf
#include <ConnectRPCExecutor.h>
#include <ConnectRPC.h>
#include <MDServerApplication.h>
#include <VisItException.h>

// ****************************************************************************
// Method: ConnectRPCExecutor::ConnectRPCExecutor
//
// Purpose: 
//   Constructor for the ConnectRPCExecutor class.
//
// Arguments:
//   s : A pointer to the RPC that will call this RPC executor.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:53:17 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConnectRPCExecutor::ConnectRPCExecutor(Subject *s) : Observer(s)
{
}

// ****************************************************************************
// Method: ConnectRPCExecutor::~ConnectRPCExecutor
//
// Purpose: 
//   Destructor for the ConnectRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:53:56 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConnectRPCExecutor::~ConnectRPCExecutor()
{
}

// ****************************************************************************
// Method: ConnectRPCExecutor::StrDup
//
// Purpose: 
//   Duplicates a string.
//
// Arguments:
//   str : The string to duplicate.
//
// Returns:   a copy of the passed-in string. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:54:20 PST 2000
//
// Modifications:
//   
// ****************************************************************************

char *
ConnectRPCExecutor::StrDup(const char *str)
{
    if(str == NULL)
        return NULL;

    int len = strlen(str);
    char *retval = new char[len + 1];
    for(int i = 0; i < len; ++i)
        retval[i] = str[i];
    retval[len] = '\0';
    
    return retval;
}

// ****************************************************************************
// Method: ConnectRPCExecutor::Update
//
// Purpose: 
//   Tells the MDServerApplication to connect to another process.
//
// Arguments:
//   s : A pointer to the ConnectRPC that called this method.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:55:02 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 16:06:06 PST 2002
//   I added support for a security key.
//
//   Brad Whitlock, Mon Jun 16 13:51:43 PST 2003
//   I made the code more generic so different kinds of connections can be
//   used to talk to the client.
//
// ****************************************************************************

void
ConnectRPCExecutor::Update(Subject *s)
{
    ConnectRPC *rpc = (ConnectRPC *)s;

    // Create a fake argv that we can use to make a new MDServerConnection.
    const stringVector &args = rpc->GetArguments();
    int i, argc = args.size();
    int argc2 = argc;
    char **argv = new char *[argc + 1];
    char **argv2 = new char *[argc + 1];
    for(i = 0; i < argc; ++i)
    {
        argv[i] = StrDup(args[i].c_str());
        argv2[i] = argv[i];
    }
    argv[argc]  = NULL;
    argv2[argc2] = NULL;

    TRY
    {
        // Make the MDServerApplication connect to another program.
        MDServerApplication::Instance()->AddConnection(&argc2, &argv2);
    }
    CATCH(VisItException)
    {
        ; // The connection probably could not be created.
    }
    ENDTRY

    // Delete the temporary arguments.
    for(i = 0; i < argc; ++i)
        delete [] argv[i];
    delete [] argv;
    delete [] argv2;
}
