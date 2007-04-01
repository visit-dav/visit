#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#include <string.h>
#else
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <algorithm>

#include <visit-config.h> // To get the version number
#include <MDServerConnection.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtSIL.h>
#include <SILAttributes.h>

#include <ChangeDirectoryRPC.h>
#include <CreateGroupListRPC.h>
#include <ChangeDirectoryRPCExecutor.h>
#include <CloseDatabaseRPC.h>
#include <CloseDatabaseRPCExecutor.h>
#include <Connection.h>
#include <ConnectRPC.h>
#include <ConnectRPCExecutor.h>
#include <DatabaseException.h>
#include <DatabasePluginManager.h>
#include <DebugStream.h>
#include <ExpandPathRPC.h>
#include <ExpandPathRPCExecutor.h>
#include <GetDirectoryRPC.h>
#include <GetDirectoryRPCExecutor.h>
#include <GetFileListRPC.h>
#include <GetFileListRPCExecutor.h>
#include <GetMetaDataRPC.h>
#include <GetMetaDataRPCExecutor.h>
#include <GetSILRPC.h>
#include <GetSILRPCExecutor.h>
#include <KeepAliveRPC.h>
#include <KeepAliveRPCExecutor.h>
#include <LoadPluginsRPC.h>
#include <LoadPluginsRPCExecutor.h>
#include <CouldNotConnectException.h>
#include <IncompatibleVersionException.h>
#include <ParentProcess.h>
#include <QuitRPC.h>
#include <QuitRPCExecutor.h>
#include <SocketConnection.h>
#include <TimingsManager.h>
#include <Utility.h>
#include <Xfer.h>
#include <RPCExecutor.h>
#include <visitstream.h>

// Make the initial connection timeout be 2 minutes. If the mdserver cannnot
// connect to the client in that time, exit.
#define INITIAL_CONNECTION_TIMEOUT 120

#if !defined(_WIN32)
// ****************************************************************************
// Function: InitialConnectionHandler
//
// Purpose:
//   This is a signal handler that aborts the mdserver if it cannot connect
//   to a client in a certain amount of time.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 09:31:10 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
InitialConnectionHandler(int)
{
    debug1 << "MDSERVER: Could not connect to client in allotted time. "
              "The client may no longer wish to connect so this program "
              "will exit." << endl;
    exit(0);
}
#endif

// ****************************************************************************
// Method: RPCExecutor<CreateGroupListRPC>::Execute
//
// Purpose: 
//   Creates a group list.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 15:41:29 PST 2001
//
// Modifications:
//   
// ****************************************************************************
template<>
void
RPCExecutor<CreateGroupListRPC>::Execute(CreateGroupListRPC *rpc)
{
    // Create a file that has all of the files in the groupList listed in it.
    ofstream out(rpc->filename.c_str());

    out << "# VisIt group file, version " << VERSION << endl;

    std::vector<string>::iterator i;
    for(i=rpc->groupList.begin();i!=rpc->groupList.end();i++)
        out << i->c_str() << endl;

    out.close();
}

/////////////////////////////////////////////////////////////////////////////

// Static members.
bool            MDServerConnection::staticInit = false;
bool            MDServerConnection::pluginsLoaded = false;
avtDatabase    *MDServerConnection::currentDatabase;
std::string     MDServerConnection::currentDatabaseName;
int             MDServerConnection::currentDatabaseTimeState = 0;
bool            MDServerConnection::currentDatabaseHasInvariantMD = true;
MDServerConnection::VirtualFileInformationMap MDServerConnection::virtualFiles;

// ****************************************************************************
// Method: MDServerConnection::MDServerConnection
//
// Purpose: 
//   Constructor for the MDServerConnection class. This method connects to a
//   remote process and creates new state objects used for communicating back
//   and forth with the remote process.
//
// Arguments:
//   argc : The number of arguments in argv.
//   argv : A pointer to the argument list used in creating the ParentProcess
//          object used to communicate with the remote process.
//
// Note:
//   The RPCs must be registered with the xfer object before the RPC observers
//   are created since the RPC observers may register state objects with xfer
//   to carry back return values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:35:15 PST 2000
//
// Modifications:
//    Sean Ahern, Wed Feb 28 14:30:41 PST 2001
//    Added the CreateGroupListRPC.
//   
//    Hank Childs, Thu Mar 29 17:01:44 PST 2001
//    Added SIL.
//
//    Brad Whitlock, Thu Apr 26 13:04:41 PST 2001
//    Added code to handle the incompatible version exception.
//
//    Jeremy Meredith, Wed Oct 10 14:46:57 PDT 2001
//    Made currentDatabase static.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Brad Whitlock, Tue Feb 12 13:53:13 PST 2002
//    Added ExpandPathRPC.
//
//    Brad Whitlock, Mon Mar 25 16:51:30 PST 2002
//    Modified the communication code. Removed the code to get the groups.
//
//    Brad Whitlock, Tue Jul 30 11:12:43 PDT 2002
//    I added a new RPC to close the database.
//
//    Brad Whitlock, Mon Sep 30 09:36:34 PDT 2002
//    I added a signal handler that kills the mdserver if it cannot connect
//    to its client within a few minutes. It most likely indicates that 
//    the client does not want to connect anymore.
//
//    Brad Whitlock, Tue Jul 29 11:36:11 PDT 2003
//    I changed the interface to ParentProcess::Connect.
//
//    Hank Childs, Thu Jan 22 21:02:56 PST 2004
//    Added LoadPlugins.
//
//    Brad Whitlock, Fri Mar 12 10:19:24 PDT 2004
//    I added KeepAliveRPC.
//
// ****************************************************************************

MDServerConnection::MDServerConnection(int *argc, char **argv[])
{
    int total = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();
    string connectStr("Connecting to client");

    // Initialize some static members.
    if(!staticInit)
    {
        staticInit = true;
        currentDatabase = NULL;
        currentDatabaseHasInvariantMD = true;
    }

    // Initialize some pointer members.
    currentMetaData = NULL;
    currentSIL      = NULL;

    // Set an internal flag to zero.
    readFileListReturnValue = 0;

    // Create a new ParentProcess and use it to connect to another
    // program.
    parent = new ParentProcess;
    TRY
    {
        parent->Connect(1, 1, argc, argv, true);
        visitTimer->StopTimer(timeid, connectStr);
    }
    CATCH(IncompatibleVersionException)
    {
        visitTimer->StopTimer(timeid, connectStr);
        debug1 << "The mdserver connected to a client that has a different "
               << "version number than the mdserver itself."
               << endl;
        RETHROW;
    }
    CATCH(CouldNotConnectException)
    {
        visitTimer->StopTimer(timeid, connectStr);
        debug1 << "The mdserver could not create a new connection." << endl;
        RETHROW;
    }
    ENDTRY

    // Create a new Xfer object that we'll use to talk to the other
    // process.
    xfer = new Xfer;
    xfer->SetInputConnection(parent->GetWriteConnection());
    xfer->SetOutputConnection(parent->GetReadConnection());

    // Create the RPCs
    quitRPC = new QuitRPC;
    keepAliveRPC = new KeepAliveRPC;
    getDirectoryRPC = new GetDirectoryRPC;
    changeDirectoryRPC = new ChangeDirectoryRPC;
    getFileListRPC = new GetFileListRPC;
    getMetaDataRPC = new GetMetaDataRPC;
    getSILRPC = new GetSILRPC;
    connectRPC = new ConnectRPC;
    createGroupListRPC = new CreateGroupListRPC;
    expandPathRPC = new ExpandPathRPC;
    closeDatabaseRPC = new CloseDatabaseRPC;
    loadPluginsRPC = new LoadPluginsRPC;

    // Hook up the RPCs to the xfer object.
    xfer->Add(quitRPC);
    xfer->Add(keepAliveRPC);
    xfer->Add(getDirectoryRPC);
    xfer->Add(changeDirectoryRPC);
    xfer->Add(getFileListRPC);
    xfer->Add(getMetaDataRPC);
    xfer->Add(getSILRPC);
    xfer->Add(connectRPC);
    xfer->Add(createGroupListRPC);
    xfer->Add(expandPathRPC);
    xfer->Add(closeDatabaseRPC);
    xfer->Add(loadPluginsRPC);

    // Create the RPC Observers.
    quitExecutor = new QuitRPCExecutor(quitRPC);
    keepAliveExecutor = new KeepAliveRPCExecutor(keepAliveRPC);
    getDirectoryExecutor = new GetDirectoryRPCExecutor(this, getDirectoryRPC);
    changeDirectoryExecutor = 
        new ChangeDirectoryRPCExecutor(this, changeDirectoryRPC);
    getFileListExecutor = new GetFileListRPCExecutor(this, getFileListRPC);
    getMetaDataExecutor = new GetMetaDataRPCExecutor(this, getMetaDataRPC);
    getSILExecutor = new GetSILRPCExecutor(this, getSILRPC);
    connectExecutor = new ConnectRPCExecutor(connectRPC);
    createGroupListExecutor = new RPCExecutor<CreateGroupListRPC>(createGroupListRPC);
    expandPathExecutor = new ExpandPathRPCExecutor(this, expandPathRPC);
    closeDatabaseExecutor = new CloseDatabaseRPCExecutor(this, closeDatabaseRPC);
    loadPluginsExecutor = new LoadPluginsRPCExecutor(this, loadPluginsRPC);

    // Indicate that the file list is not valid since we have not read
    // one yet.
    validFileList = false;

    // Get the current directory.
    ReadCWD();

    visitTimer->StopTimer(total, connectStr + " and setup");
}

// ****************************************************************************
// Method: MDServerConnection::~MDServerConnection
//
// Purpose: 
//   Destructor for the MDServerConnection class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:37:54 PST 2000
//
// Modifications:
//   Sean Ahern, Wed Feb 28 14:30:41 PST 2001
//   Added the CreateGroupListRPC.
//
//   Brad Whitlock, Tue Feb 12 13:54:43 PST 2002
//   Added ExpandPathRPC.
//
//   Brad Whitlock, Tue Jul 30 11:14:29 PDT 2002
//   I added closeDatabaseRPC.
//
//   Brad Whitlock, Wed Apr 2 15:51:27 PST 2003
//   Deleted createGroupListRPC and the currentSIL.
//
//   Hank Childs, Thu Jan 22 21:02:56 PST 2004
//   Added loadPlugins.
//
//   Brad Whitlock, Fri Mar 12 10:19:42 PDT 2004
//   I added KeepAliveRPC.
//
// ****************************************************************************

MDServerConnection::~MDServerConnection()
{
    // Delete the RPC executors.
    delete quitExecutor;
    delete keepAliveExecutor;
    delete getDirectoryExecutor;
    delete changeDirectoryExecutor;
    delete getFileListExecutor;
    delete getMetaDataExecutor;
    delete getSILExecutor;
    delete connectExecutor;
    delete createGroupListExecutor;
    delete expandPathExecutor;
    delete closeDatabaseExecutor;
    delete loadPluginsExecutor;

    // Delete the RPCs
    delete quitRPC;
    delete keepAliveRPC;
    delete getDirectoryRPC;
    delete changeDirectoryRPC;
    delete getFileListRPC;
    delete getMetaDataRPC;
    delete getSILRPC;
    delete connectRPC;
    delete createGroupListRPC;
    delete expandPathRPC;
    delete closeDatabaseRPC;
    delete loadPluginsRPC;

    // Delete the database.
    if(currentDatabase)
    {
        delete currentDatabase;
        currentDatabase = NULL;
    }

    // Delete the SIL
    if(currentSIL)
    {
        delete currentSIL;
        currentSIL = NULL;
    }

    // Delete the xfer and parent objects.
    delete xfer;
    delete parent;
}

// ****************************************************************************
// Method: MDServerConnection::KeepGoing
//
// Purpose: 
//   Tells whether the MDServerConnection's quit rpc state object wants to
//   keep going.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:38:42 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
MDServerConnection::KeepGoing() const
{
    return !(quitRPC->GetQuit());
}

// ****************************************************************************
// Method: MDServerConnection::ProcessInput
//
// Purpose: 
//   Tells the MDServerConnection to check for command input and execute it
//   if there was any.
//
// Arguments:
//
// Returns:    Returns true if there was command input.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:25:39 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 15 12:38:32 PDT 2001
//   Added code to throw an exception if we lost the connetion.
//
//   Brad Whitlock, Wed Mar 20 17:46:59 PST 2002
//   I abstracted the read code.
//
// ****************************************************************************

bool
MDServerConnection::ProcessInput()
{
    // Try reading input from the parent process's write socket.
    int amountRead = xfer->GetInputConnection()->Fill();

    // Process the new information.
    if(amountRead > 0)
        xfer->Process();

    return (amountRead > 0);
}

// ****************************************************************************
// Method: MDServerConnection::GetWriteConnection
//
// Purpose: 
//   Returns the MDServerConnection's write connection.
//
// Arguments:
//
// Returns:    The MDServerConnection's write connection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 09:35:43 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
MDServerConnection::GetWriteConnection() const
{
    return parent->GetWriteConnection();
}

// ****************************************************************************
// Method: MDServerConnection::LoadPlugins
//
// Purpose: 
//   Loads the plugins if they have not been loaded.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 9 10:58:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MDServerConnection::LoadPlugins()
{
    if(!pluginsLoaded)
    {
        debug2 << "Loading plugins!" << endl;
        DatabasePluginManager::Initialize(DatabasePluginManager::MDServer, false);
        DatabasePluginManager::Instance()->LoadPluginsNow();
        pluginsLoaded = true;
    }
}

// ****************************************************************************
// Method: MDServerConnection::ReadMetaData
//
// Purpose:
//   Gets the metadata for a file.
//
// Arguments:
//   file      : The name of the file for which to get the metadata.
//   timeState : The timestate for which to get the metadata.
//
// Returns:
//   This method returns 0 on success and -1 on failure.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Hank Childs, Mon Sep 18 17:57:29 PDT 2000
//   Used database factory to get cur_db.
//
//   Brad Whitlock, Fri Nov 17 15:03:16 PST 2000
//   Turned it into a method of MDServerConnection and renamed it.
//
//   Hank Childs, Thu Mar 29 17:08:29 PST 2001
//   Re-use currentDatabase if it is valid, so SIL and meta-data can use the
//   same database.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Thu Feb 7 10:55:20 PDT 2002
//   Removed the code to catch database exceptions. They are now caught
//   higher up the chain.
//
//   Brad Whitlock, Tue Mar 25 15:02:06 PST 2003
//   I added code to set whether or not the file is virtual. If the file is
//   virtual then we set the timestep names for it.
//
//   Brad Whitlock, Tue May 13 15:42:59 PST 2003
//   I added timeState.
//
// ****************************************************************************

int
MDServerConnection::ReadMetaData(std::string file, int timeState)
{
    currentMetaData = NULL;

    int ts = (timeState == -1) ? currentDatabaseTimeState : timeState;
    debug2 << "Read the Metadata for " << file.c_str()
           << ", timeState=" << ts << endl;

    //
    // Try and read the database. This could throw an exception.
    //
    avtDatabase *db = GetDatabase(file, ts);
    if (db != NULL)
    {
        currentMetaData = db->GetMetaData(ts);
    }

    //
    // If we have metadata for the file then set whether or not the file is a
    // virtual file.
    //
    if(currentMetaData)
    {
        VirtualFileInformationMap::iterator pos = virtualFiles.find(ExpandPath(file));
        if(pos != virtualFiles.end())
        {
            currentMetaData->SetIsVirtualDatabase(true);
            currentMetaData->SetTimeStepPath(pos->second.path);
            currentMetaData->SetTimeStepNames(pos->second.files);
        }
        else
        {
            currentMetaData->SetIsVirtualDatabase(false);
            stringVector names;
            names.push_back(file);
            currentMetaData->SetTimeStepPath("");
            currentMetaData->SetTimeStepNames(names);
        }

        currentDatabaseHasInvariantMD = ! currentMetaData->GetMustRepopulateOnStateChange();
    }

    return (currentMetaData == NULL ? -1 : 0);
}

// ****************************************************************************
// Method: MDServerConnection::GetCurrentMetaData
//
// Purpose: 
//   Returns a pointer to the current file's metadata.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:04:17 PST 2000
//
// Modifications:
//   
// ****************************************************************************

avtDatabaseMetaData *
MDServerConnection::GetCurrentMetaData() const
{
    return currentMetaData;
}

// ****************************************************************************
// Method: MDServerConnection::ReadSIL
//
// Purpose:
//   Gets the SIL for a file.
//
// Arguments:
//   file      : The name of the file for which to get the metadata.
//   timeState : The timestate for which to get the metadata.
//
// Returns:
//   This method returns 0 on success and -1 on failure.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   Hank Childs, Wed Oct 10 13:22:51 PDT 2001
//   Do not assume that GetDatabase will re-assign the current database.
//
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//   Brad Whitlock, Thu Feb 7 10:55:20 PDT 2002
//   Removed the code to catch database exceptions. They are now caught
//   higher up the chain.
//
//   Brad Whitlock, Wed Apr 2 15:53:04 PST 2003
//   I added code to delete the SIL attributes.
//
//   Brad Whitlock, Tue May 13 15:42:59 PST 2003
//   I added timeState.
//
//   Brad Whitlock, Thu Oct 9 14:35:00 PST 2003
//   Fixed memory leak.
//
// ****************************************************************************

int
MDServerConnection::ReadSIL(std::string file, int timeState)
{
    if(currentSIL != NULL)
    {
        delete currentSIL;
        currentSIL = NULL;
    }

    int ts = (timeState == -1) ? currentDatabaseTimeState : timeState;
    debug2 << "Read the SIL for " << file.c_str()
           << ", timeState=" << ts << endl;

    //
    // Try and read the database. This could throw an exception.
    //
    avtDatabase *db = GetDatabase(file, ts);
    if (db != NULL)
    {
        avtSIL *s  = db->GetSIL(ts);

        // Delete the SIL attributes if they are not NULL.
        if(currentSIL != NULL)
            delete currentSIL;

        currentSIL = s->MakeSILAttributes();
    }

    return (currentSIL == NULL ? -1 : 0);
}

// ****************************************************************************
// Method: MDServerConnection::GetCurrentSIL
//
// Purpose: 
//   Returns a pointer to the current file's SIL.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// ****************************************************************************

SILAttributes *
MDServerConnection::GetCurrentSIL() const
{
    return currentSIL;
}

// ****************************************************************************
// Method: MDServerConnection::FilteredPath
//
// Purpose: 
//   Filters extra junk out of a path.
//
// Arguments:
//   path : The path string that we're filtering.
//
// Returns:    A filtered path string.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 13 14:08:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Mar 18 11:40:40 PDT 2004
//   I fixed the code so it filters out .. properly.
//
//   Brad Whitlock, Wed Apr 14 17:22:53 PST 2004
//   I fixed the .. filtering for Windows.
//
// ****************************************************************************

std::string
MDServerConnection::FilteredPath(const std::string &path) const
{
    // Remove multiple slashes in a row.
    int i, state = 0;
    std::string filteredPath;
    for(i = 0; i < path.length(); ++i)
    {
        if(state == 0)
        {
            filteredPath += path[i];
            if(path[i] == SLASH_CHAR)
                state = 1;
        }
        else if(path[i] != SLASH_CHAR)
        {
            filteredPath += path[i];
            state = 0;
        }
    }

    std::string path2(filteredPath);
    if(path2.length() > 0 && path2[path2.length() - 1] == SLASH_CHAR)
    {
        filteredPath = path2.substr(0, path2.length() - 1);
    }

    if(filteredPath.size() == 0)
        filteredPath = SLASH_STRING;
    else
    {
        // Filter out .. so we get the right path.
        stringVector tmpNames;
        std::string  tmp;
        state = 0;
        const char *str = filteredPath.c_str();
        for(i = 0; i < filteredPath.length() + 1; ++i)
        {
            if(str[i] == SLASH_CHAR || str[i] == '\0')
            {
                if(tmp.size() > 0)
                {
                    if(tmp == "..")
                        tmpNames.pop_back();
                    else
                        tmpNames.push_back(tmp);
                }
                tmp = "";
            }
            else
                tmp += str[i];
        }

        // Reassemble the path fragments.
        if(tmpNames.size() > 0)
        {
            filteredPath = "";
            for(i = 0; i < tmpNames.size(); ++i)
            { 
#if defined(_WIN32)
                if(i > 0)
                    filteredPath += SLASH_STRING;
#else
                filteredPath += SLASH_STRING;
#endif
                filteredPath += tmpNames[i];
            }
        }
    }

    return filteredPath;
}

// ****************************************************************************
// Function: MDServerConnection::ExpandPath
//
// Purpose:
//   Expands tilde paths into the correct directory, and relative
//   paths to absolute ones.
//
// Arguments:
//   dir : The new directory.
//
// Returns:
//   This routine returns the expanded path.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 17 15:10:54 PST 2000
//   Made it a method of MDServerConnection and changed how newPath
//   is calculated so it uses string methods instead of assuming C-style
//   strings.
//
//   Brad Whitlock, Tue Apr 9 17:13:26 PST 2002
//   Added a simple Windows implementation.
//
//   Brad Whitlock, Mon Aug 26 15:04:15 PST 2002
//   I made it return a filtered path.
//
//   Brad Whitlock, Wed Sep 11 14:33:51 PST 2002
//   I made the Windows version filter out the "My Computer" string.
//
//   Brad Whitlock, Wed Apr 2 12:48:09 PDT 2003
//   I moved the guts to ExpandPathHelper.
//
// ****************************************************************************

std::string
MDServerConnection::ExpandPath(const std::string &path)
{
    return ExpandPathHelper(path, currentWorkingDirectory);
}

// ****************************************************************************
// Method: MDServerConnection::ExpandPathHelper
//
// Purpose: 
//   Expands the path.
//
// Arguments:
//   path       : The path to be expanded.
//   workingDir : The current working dir to use.
//
// Returns:    The expanded path.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 2 12:47:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
MDServerConnection::ExpandPathHelper(const std::string &path,
    const std::string &workingDir) const
{
    std::string newPath(path);

#if defined(_WIN32)
    char driveLetter;

    if(path[0] == '~')
    {
        char username[256];
        int  i;

        // Find the user name portion of the path, ie ~user
        for (i = 1; isalnum(path[i]); i++)
        {
            username[i - 1] = path[i];
        }
        username[i - 1] = '\0';

        // Append the rest of the path to the home directory.
        std::string restOfPath(path.substr(i, path.length() - i + 1));
        std::string homeDir("C:\\Documents and Settings\\");
        newPath = homeDir + std::string(username) + restOfPath;
    }
    else if(sscanf(path.c_str(), "My Computer\\%c:", &driveLetter) == 1)
    {
         // Filter out the "My Computer" part of the path.
         char tmp[2] = {driveLetter, '\0'};
         newPath = std::string(tmp) + ":\\";
    }
    else if(sscanf(path.c_str(), "%c:\\", &driveLetter) == 1)
    {
         // absolute path. do nothing
    }
    else
    {
        // relative path:
        newPath = workingDir + "\\" + path;
    }
#else
    if(path[0]=='~')
    {
        char username[256];
        int  i;

        // Find the user name portion of the path, ie ~user
        for (i = 1; isalnum(path[i]); i++)
        {
            username[i - 1] = path[i];
        }
        username[i - 1] = '\0';

        // Check if the user specified '~' or '~name'.
        struct passwd *users_passwd_entry = NULL;
        if (i == 1)
        {
            // User just specified '~', get /etc/passwd entry
            users_passwd_entry = getpwuid(getuid());
        } else
        {
            // User specified '~name', get /etc/passwd entry
            users_passwd_entry = getpwnam(username);
        }

        // Now that we have a passwd entry, validate it.
        if (users_passwd_entry == NULL)
        {
            // Did not specify a valid user name.  Do nothing. 
            return path;
        }
        if (users_passwd_entry->pw_dir == NULL)
        {
            // Passwd entry is invalid.  Do nothing.
            return path;
        }

        // Append the rest of the path to the home directory.
        std::string restOfPath(path.substr(i, path.length() - i + 1));
        newPath = std::string(users_passwd_entry->pw_dir) + restOfPath;
    }
    else if(path[0] != '/')
    {
        // relative path:
        newPath = workingDir + "/" + path;
    }
    else
    {
        // absolute path: do nothing
    }
#endif

    return FilteredPath(newPath);
}

// ****************************************************************************
// Method: MDServerConnection::ChangeDirectory
//
// Purpose:
//   Changes to the current directory.
//
// Arguments:
//   dir : The new directory.
//
// Returns:
//   This routine returns 0 on success and -1 on failure.
//
// Programmer: Jeremy Meredith
// Creation:   Wed Aug 23 12:09:39 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 23 12:10:36 PDT 2000
//   I changed the code so it assumes the directory name that is
//   passed is an absolute path instead of a relative path.
//
//   Brad Whitlock, Tue Aug 29 15:52:13 PST 2000
//   I added a return code that tells if the operation was successful.
//
//   Jeremy Meredith, Fri Sep  1 17:27:15 PDT 2000
//   I extracted the function which resolved the path.
//
//   Brad Whitlock, Fri Nov 17 15:18:42 PST 2000
//   Made it a method of MDServerConnection.
//
//   Brad Whitlock, Tue Apr 9 15:35:21 PST 2002
//   Added a Windows implementation.
//
//   Brad Whitlock, Wed Sep 11 14:35:58 PST 2002
//   I made the Windows version ignore the "My Computer" string.
//
//   Brad Whitlock, Mon Mar 24 15:39:07 PST 2003
//   I made this routine read the file list.
//
// ****************************************************************************

int
MDServerConnection::ChangeDirectory(const std::string &dir)
{
    // Expand the directory to its full path.
    std::string expandedDir = ExpandPath(dir);

#if defined(_WIN32)
    if(expandedDir != "My Computer")
    {
        // Try and change the current working directory.
        if(_chdir(expandedDir.c_str()) != 0)
        {
            // Another return code was given. Assume a bad directory.
            return -1;
        }
    }
#else
    // Try and change the current working directory.
    if(chdir(expandedDir.c_str()) != 0)
    {
        // Another return code was given. Assume a bad directory.
        return -1;
    }
#endif

    // Record that we've successfully changed the directory.
    //ReadCWD();
    currentWorkingDirectory = expandedDir;

    // We changed to a new directory so read its file list.
    ReadFileList();

    return 0;
}

// ****************************************************************************
// Method: MDServerConnection::GetCurrentWorkingDirectory
//
// Purpose: 
//   Returns the current working directory.
//
// Arguments:
//
// Returns:  Returns the current working directory.  
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:29:30 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const std::string &
MDServerConnection::GetCurrentWorkingDirectory() const
{
    return currentWorkingDirectory;
}

// ****************************************************************************
// Method: MDServerConnection::ReadCWD
//
// Purpose:
//   Gets the current directory and stores it.
//
// Programmer: Jeremy Meredith
// Creation:   Wed Aug 23 12:09:39 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 17 15:18:42 PST 2000
//   Made it a method of MDServerConnection.
//
//   Brad Whitlock, Tue Apr 9 15:32:47 PST 2002
//   Added a Windows implementation.
//
//   Brad Whitlock, Mon Aug 26 15:14:56 PST 2002
//   I added code to expand the path.
//
// ****************************************************************************

void
MDServerConnection::ReadCWD()
{
    // Note -- this is not called by the GetDirectoryRPC.  It is only
    // called once at the start of the program and once at every
    // directory change.
    char tmpcwd[1024];
#if defined(_WIN32)
    _getcwd(tmpcwd,1023);
#else
    getcwd(tmpcwd,1023);
#endif
    tmpcwd[1023]='\0';

    currentWorkingDirectory = ExpandPath(tmpcwd);
#ifdef DEBUG
    debug2 << "CWD: '" << currentWorkingDirectory.c_str() << "'" << endl;
#endif
}

// ****************************************************************************
// Method: MDServerConnection::ReadFileList
//
// Purpose:
//   Gets the file list for the current directory (cwd).
//
// Returns:
//   This function returns 0 on success and -1 on failure.
//
// Programmer: Jeremy Meredith
// Creation:   Wed Aug 23 12:13:30 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 15:55:29 PST 2000
//   Added an error return code.
//
//   Brad Whitlock, Tue Apr 9 15:30:57 PST 2002
//   Added a Windows implementation.
//
//   Brad Whitlock, Wed Sep 11 14:38:14 PST 2002
//   I enhanced the Windows version so that it can return drive names. I
//   also fixed a bug that prevented some files from making it into the
//   file list.
//
//   Brad Whitlock, Mon Mar 24 15:48:12 PST 2003
//   I made it set the class variable readFileListReturnValue. I also made it
//   sort the file list to avoid potential problems arising from defining
//   virtual files.
//
// ****************************************************************************

void
MDServerConnection::ReadFileList()
{
    readFileListReturnValue = 0;
    validFileList = true;

#if defined(_WIN32)
    if(currentWorkingDirectory == "My Computer")
    {
        // Add the drives to the list.
        char buf[200];
        DWORD bufLen = 200;
        DWORD slen = GetLogicalDriveStrings(200, buf);

        if(slen > 0)
        {
            // Clear out the file list.
            currentFileList.Clear();

            char *ptr = buf;
            while(*ptr != 0)
            {
                std::string drive(ptr);
                currentFileList.names.push_back(drive);
                currentFileList.types.push_back(GetFileListRPC::DIR);
                currentFileList.sizes.push_back(0);
                currentFileList.access.push_back(1);
                ptr += (drive.size() + 1);
            }
        }
    }
    else
    {
        // Try and read the files in fullPath.
        std::string searchPath(currentWorkingDirectory + std::string("\\*"));
        WIN32_FIND_DATA fd;
        HANDLE dirHandle = FindFirstFile(searchPath.c_str(), &fd);
        if(dirHandle != INVALID_HANDLE_VALUE)
        {
            // Clear out the file list.
            currentFileList.Clear();

            do
            {
                bool isDir = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
                             (strcmp(fd.cFileName, "..") == 0);
                currentFileList.names.push_back(fd.cFileName);
                currentFileList.types.push_back(isDir ? GetFileListRPC::DIR :
                                                GetFileListRPC::REG);
                currentFileList.sizes.push_back((fd.nFileSizeHigh * MAXDWORD) +
                                                fd.nFileSizeLow);
                currentFileList.access.push_back(1);
            } while(FindNextFile(dirHandle, &fd));
            FindClose(dirHandle);

            // If the file list does not contain "." or ".." directories,
            // add them.
            static const char *dirstr[] = {".", ".."};
            for(int i = 0; i < 2; ++i)
            {
                const stringVector &n = currentFileList.names;
                if(std::find(n.begin(), n.end(), dirstr[i]) == n.end())
                {
                    currentFileList.names.push_back(dirstr[i]);
                    currentFileList.types.push_back(GetFileListRPC::DIR);
                    currentFileList.sizes.push_back(0);
                    currentFileList.access.push_back(1);
                }
            }
        }
    }
#else
    // Clear out the current file list.
    GetFileListRPC::FileList &fl = currentFileList;
    currentFileList.Clear();

    DIR     *dir;
    dirent  *ent;

    // If the directory cannot be opened, return an error code.
    dir = opendir(currentWorkingDirectory.c_str());
    if (!dir)
        readFileListReturnValue = -1;

    // Get the userId and the groups for that user so we can check the
    // file permissions.
    uid_t uid = getuid();
    gid_t gids[100];
    int ngids;
    ngids = getgroups(100, gids);

    // Add each directory entry to the file list.
    while ((ent = readdir(dir)) != NULL)
    {
        struct stat s;
        stat((currentWorkingDirectory + "/" + ent->d_name).c_str(), &s);
        fl.names.push_back(ent->d_name);

        mode_t mode = s.st_mode;

        bool isdir = S_ISDIR(mode);
        bool isreg = S_ISREG(mode);

        fl.types.push_back(isdir ? GetFileListRPC::DIR :
                           isreg ? GetFileListRPC::REG : 
                           GetFileListRPC::UNKNOWN);
        fl.sizes.push_back((long)s.st_size);

        bool isuser  = (s.st_uid == uid);
        bool isgroup = false;
        for (int i=0; i<ngids && !isgroup; i++)
            if (s.st_gid == gids[i])
                isgroup=true;

        bool canaccess = false;
        if (isdir)
        {
            if ((mode & S_IROTH) &&
                (mode & S_IXOTH))
                canaccess=true;
            else if (isuser &&
                     (mode & S_IRUSR) &&
                     (mode & S_IXUSR))
                canaccess=true;
            else if (isgroup &&
                     (mode & S_IRGRP) &&
                     (mode & S_IXGRP))
                canaccess=true;
        }
        else
        {
            if (mode & S_IROTH)
                canaccess=true;
            else if (isuser &&
                     (mode & S_IRUSR))
                canaccess=true;
            else if (isgroup &&
                     (mode & S_IRGRP))
                canaccess=true;
        }

        fl.access.push_back(canaccess ? 1 : 0);
    }

    closedir(dir);
#endif

    // Sort the file list.
    currentFileList.Sort();
}

// ****************************************************************************
// Method: MDServerConnection::GetReadFileListReturnValue
//
// Purpose: 
//   Returns a flag that indicates how the ReadFileList function performed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 24 15:50:09 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
MDServerConnection::GetReadFileListReturnValue() const
{
    return readFileListReturnValue;
}

// ****************************************************************************
// Method: MDServerConnection::GetCurrentFileList
//
// Purpose: 
//   Returns a reference to the current file list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:26:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GetFileListRPC::FileList *
MDServerConnection::GetCurrentFileList()
{
    return &currentFileList;
}

// ****************************************************************************
// Method: MDServerConnection::FileMatchesFilterList
//
// Purpose: 
//   Checks a filename against a list of filters to see if it matches
//   any of them.
//
// Arguments:
//   fileName   : The filename we're checking against the filters.
//   filterList : The list of filters we're checking for.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 15:19:12 PST 2000
//
// Modifications:
//   Jeremy Meredith, Fri Mar 19 14:46:24 PST 2004
//   I made it use WildcardStringMatch from Utility.h.
//
// ****************************************************************************

bool
MDServerConnection::FileMatchesFilterList(const std::string &fileName,
    const stringVector &filterList) const
{
    // Try the filename against all the filters in the list until
    // it matches or we've tested all the filters.
    bool match = false;
    for(int i = 0; i < filterList.size() && !match; ++i)
    {
        match = WildcardStringMatch(filterList[i], fileName);
    }

    return match;
}

// ****************************************************************************
// Method: MDServerConnection::GetPattern
//
// Purpose: 
//   Return a pattern string of the form *[0-9]?[0-9]** that will match
//   the file name.
//
// Arguments:
//   file    : The filename.
//   pattern : The pattern string to be returned.
//
// Returns:    True if a pattern was detected.
//
// Note:       Based on Eric's pattern matching code in MeshTV.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 10:57:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
MDServerConnection::GetPattern(const std::string &file, std::string &p) const
{
    int i, isave = 0, ipat = 0;
    char pattern[256];
    for(i = 0; i < 256; ++i) pattern[i] = '\0';

    /* Go up to the beginning of the digit string.  */
    for (i = 0; i < file.size();)
    {
        for (; i < file.size() &&
             (file[i] < '0' || file[i] > '9'); i++)
        {
            pattern[i] = file[i];
        }
        if (i < file.size())
            isave = i;
        for (; file[i] >= '0' && file[i] <= '9'; i++)
            pattern[i] = file[i];
    }

    /* Skip over the digit string.  */
    for (i = isave; file[i] >= '0' && file[i] <= '9'; i++)
        /* do nothing */ ;

    /* We have a match on *[0-9]?.  Now let's determine the full pattern.  */
    if (file[isave] >= '0' && file[isave] <= '9' &&
        (file[i] == '\0' || file[i] == '.'))
    {
        ipat = isave;

        pattern[ipat++] = '*';
        for (; i < file.size(); i++)
            pattern[ipat++] = file[i];
        pattern[ipat++] = '\0';
    }

    p = std::string(pattern);
    return (ipat > 0);
}

// ****************************************************************************
// Method: MDServerConnection::FileHasVisItExtension
//
// Purpose: 
//   Determines whether a file has the ".visit" extension.
//
// Arguments:
//   file : The file we're checking for the extension.
//
// Returns:    True if the file ends with ".visit"; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 29 11:25:44 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
MDServerConnection::FileHasVisItExtension(const std::string &file) const
{
    bool retval = false;
    if(file.size() >= 6)
    {
        std::string visitExt(file.substr(file.size() - 6));
#if defined(_WIN32)
        retval = (_stricmp(visitExt.c_str(), ".visit") == 0);
#else
        retval = (visitExt == ".visit");
#endif
    }

    return retval;
}

// ****************************************************************************
// Method: MDServerConnection::GetFilteredFileList
//
// Purpose: 
//   Returns a file list in which the files match the passed in filter. The
//   file names are also checked so that like filenames are grouped into
//   virtual VisIt files.
//
// Arguments:
//   files  : The return file list.
//   filter : The filter strings.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 10:53:27 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 22 17:24:15 PST 2003
//   I fixed a bug that prevented it from working on Windows.
//
//   Brad Whitlock, Mon Oct 27 13:27:26 PST 2003
//   Changed to reflect that the virtualFiles map now has a VirtualFileName
//   key instead of a std::string key. I did this to fix sorting problems
//   with the virtual filenames list.
//
//   Brad Whitlock, Fri Apr 30 16:47:10 PST 2004
//   I added code to prevent VisIt from creating virtual databases that
//   contain .visit files.
//
//   Brad Whitlock, Thu Jul 29 11:26:38 PDT 2004
//   Moved some code into FileHasVisItExtension. I also added the
//   extraSmartGrouping flag that does additional work to try and prevent
//   certain Ale3D databases from being grouped into virtual databases.
//
// ****************************************************************************

void
MDServerConnection::GetFilteredFileList(GetFileListRPC::FileList &files,
    const std::string &filter, bool extraSmartGrouping)
{
    //
    // If we have not yet read a file list, read it now.
    //
    if(!validFileList)
        ReadFileList();

    //
    // Parse the filter string into a list of filter strings.
    //

    // Parse the filter string and store the result in filterList.
    char *temp  = new char[filter.size() + 1];
    char *ptr, *temp2 = temp;
    memcpy((void *)temp, (void *)filter.c_str(), filter.size() + 1);
    stringVector filterList;
    while((ptr = strtok(temp2, " ")) != NULL)
    {
        filterList.push_back(std::string(ptr));
        temp2 = NULL;
    }
    delete[] temp;
    if(filterList.size() == 0)
        filterList.push_back("*");

    //
    // Compare each name in the current file list against the list of
    // filters to see if it is an acceptable filename.
    //
    int i;
    const stringVector &names = currentFileList.names;
    std::string pattern;
    VirtualFileInformationMap newVirtualFiles;
    VirtualFileInformationMap::iterator pos;
    for(i = 0; i < names.size(); ++i)
    {
        if(currentFileList.types[i] == GetFileListRPC::REG)
        {
            //
            // See if the file matches any of the filters.
            //
            if(FileMatchesFilterList(names[i], filterList))
            {
                //
                // See if the filename matches a pattern for related files.
                //
                bool matchesPattern = GetPattern(names[i], pattern);

                //
                // See if the filename is a .visit file.
                //
                bool notVisItFile = !FileHasVisItExtension(names[i]);

                //
                // If the file matches a pattern and it's not a .visit file
                // then add it to the list of possible virtual files.
                //
                if(matchesPattern && notVisItFile)
                {
                    //
                    // Try to not group certain ale3d files. Those files
                    // are of the form: X, X.1, X.2, X.3, ... We only want
                    // to group the X files and leave the X.1, X.2, X.3
                    // files unrelated.
                    //
                    if(extraSmartGrouping &&
                       pattern.size() >= 2 &&
                       pattern.substr(pattern.size() - 2) == ".*")
                    {
                        std::string pattern2;
                        bool found2ndPattern = GetPattern(pattern, pattern2);
                        if(found2ndPattern)
                        {
                            std::string basepattern(pattern2.substr(0,pattern2.size() - 2));
                            pos = newVirtualFiles.find(basepattern);
                            if(pos != newVirtualFiles.end())
                            {
                                // Add the filename to the list of
                                // filtered files.
                                files.names.push_back(names[i]);
                                files.types.push_back(currentFileList.types[i]);
                                files.sizes.push_back(currentFileList.sizes[i]);
                                files.access.push_back(currentFileList.access[i]);
                                continue;
                            }
                        }
                    }

                    //
                    // Look for the pattern in the newVirtualFiles map.
                    //
                    pos = newVirtualFiles.find(pattern);
                    if(pos != newVirtualFiles.end())
                    {
                        // The file is a new timestep in an existing virtual database.
                        pos->second.files.push_back(names[i]);
                    }
                    else
                    {
                        // Add the filename to the list of filtered files.
                        files.names.push_back(pattern);
                        files.types.push_back(currentFileList.types[i]);
                        files.sizes.push_back(currentFileList.sizes[i]);
                        files.access.push_back(currentFileList.access[i]);

                        // Add a new virtual filename.
                        newVirtualFiles[pattern].path = currentWorkingDirectory;
                        newVirtualFiles[pattern].files.push_back(names[i]);
                    }
                }
                else
                {
                    // Add the filename to the list of filtered files.
                    files.names.push_back(names[i]);
                    files.types.push_back(currentFileList.types[i]);
                    files.sizes.push_back(currentFileList.sizes[i]);
                    files.access.push_back(currentFileList.access[i]);
                }
            }
        }
        else
        {
            // Add the filename to the list of filtered files.
            files.names.push_back(names[i]);
            files.types.push_back(currentFileList.types[i]);
            files.sizes.push_back(currentFileList.sizes[i]);
            files.access.push_back(currentFileList.access[i]);
        }
    }

    //
    // Now that we've assembled a list of filenames, go back and fix the
    // ones that had a pattern but had no successors and thus were only
    // one file long.
    //
    for(pos = newVirtualFiles.begin(); pos != newVirtualFiles.end(); ++pos)
    {
        if(pos->second.files.size() == 1)
        {
            // Change the name in the files list back to the original file name.
            for(i = 0; i < files.names.size(); ++i)
            {
                if(files.names[i] == pos->first.name)
                {
                    files.names[i] = pos->second.files[0];
                    break;
                }
            }
        }
        else
        {
            // Determine a good root name for the database.
            std::string rootName(pos->first.name + " database");

            // Change the name in the files list from the pattern name
            // to the new root name. Also change the file type to VIRTUAL.
            for(i = 0; i < files.names.size(); ++i)
            {
                if(files.names[i] == pos->first.name)
                {
                    files.names[i] = rootName;
                    files.types[i] = GetFileListRPC::VIRTUAL;
                    break;
                }
            }

            // Add the timestep names to the file list's virtual files list.
            for(i = 0; i < pos->second.files.size(); ++i)
                files.virtualNames.push_back(pos->second.files[i]);
            files.numVirtualFiles.push_back(pos->second.files.size());

            // Create a good path.
            std::string path(currentWorkingDirectory);
            if(path[path.size() - 1] != SLASH_CHAR)
                path += SLASH_STRING;

            // Add the file to the virtual files map. Give the data from the
            // new vector of filenames to the existing vector of filenames
            // because swapping is cheaper than copying. Note that if the
            // key does not exist in the virtualFiles map, calling
            // its [] operator, as we are doing, automatically adds the key
            // to the map.
            std::string key(path + rootName);
            virtualFiles[key].path = path;
            virtualFiles[key].files.swap(pos->second.files);
        }
    }
}

// ****************************************************************************
// Method: MDServerConnection::GetVirtualFileDefinition
//
// Purpose: 
//   Gets an iterator to the definition of a virtual file.
//
// Arguments:
//   file : The file to get the definition for.
//
// Returns:    An iterator that lets us access the definition.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 9 16:58:40 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Jul 29 12:12:59 PDT 2004
//   Now assumes extra smart file grouping when called from the cli.
//
// ****************************************************************************

const MDServerConnection::VirtualFileInformationMap::iterator
MDServerConnection::GetVirtualFileDefinition(const std::string &file)
{
    // Look for the file in the virtual files map.
    VirtualFileInformationMap::iterator virtualFile = virtualFiles.find(file);

    //
    // If the file was not found in the virtual file map, but it contains
    // a wildcard character, which means that it is a virtual file, read
    // the file list to try and create a definition for the virtual file
    // so we can open it even if we've never seen it before.
    //
    if (virtualFile == virtualFiles.end() &&
        file.find("*") != std::string::npos)
    {
        int index = file.rfind(SLASH_STRING);
        if(index != std::string::npos)
        {
            debug2 << "A virtual database was requested but we've never seen "
                      "it before. Try and get a definition for it." << endl;

            // Remember the old path.
            std::string oldPath(currentWorkingDirectory);

            // We found the last separator so we can change to that path
            // to ensure that the file list is read.
            std::string path(file.substr(0, index));
            ChangeDirectory(path);

            // Get the filtered file list. This creates virtual files.
            GetFileListRPC::FileList f;
            GetFilteredFileList(f, "*", true);

            // Change the path back to the old path.
            ChangeDirectory(oldPath);

            // Look for the file again in the virtual file map.
            virtualFile = virtualFiles.find(file);
        }
    }

    return virtualFile;
}

// ****************************************************************************
//  Method: MDServerConnection::GetDatabase
//
//  Purpose:
//      Gets the requested database.  Provided a single point of source for
//      routines for getting SILs and MetaData.
//
//  Arguments:
//    file       The file's name.
//    timeState  The timestate that we want to open.
//
//  Returns:     The database, NULL if it could not be opened.
//
//  Programmer:  Hank Childs
//  Creation:    March 30, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Brad Whitlock, Thu Feb 7 10:48:58 PDT 2002
//    Removed code to catch database errors.
//
//    Brad Whitlock, Tue Mar 25 10:41:01 PDT 2003
//    I added support for virtual VisIt files.
//
//    Brad Whitlock, Mon Jun 9 11:00:12 PDT 2003
//    I added code to make sure that plugins are loaded. I also added timing
//    information.
//
//    Mark C. MillerThu Oct  9 11:13:01 PDT 2003
//    Added logic to only force close and re-open if the timeState is different
//    and the current database has invariant metadata. If the current database
//    does NOT have invariant metadata, then metadata at a different time
//    step can be correctly read without having to close and re-open. 
//
//    Hank Childs, Mon Mar  1 08:48:26 PST 2004
//    Set the time state to the database factory.
//
//    Brad Whitlock, Mon Mar 22 09:37:08 PDT 2004
//    Added code to print virtual file definition to debug3.
//
//    Brad Whitlock, Fri Apr 16 12:41:09 PDT 2004
//    Added code to switch current directories so we can always successfully
//    read virtual databases on the Windows platform when we use the CLI.
//
//    Brad Whitlock, Tue Jul 27 16:54:02 PST 2004
//    Added code to prevent crashes when we ask for a time state that is 
//    greater than the end of the size of a virtual database. This can happen
//    if we're trying to reopen a virtual database after files have disappeared.
//
//    Brad Whitlock, Thu Jul 29 11:19:37 PDT 2004
//    I changed the test for determining whether a file is a .visit file
//    so directories can contain ".visit".
//
// ****************************************************************************

avtDatabase *
MDServerConnection::GetDatabase(string file, int timeState)
{
    //
    // Make sure that the plugins are loaded.
    //
    LoadPlugins();

    //
    // Expand the filename so it has the whole path.
    //
    file = ExpandPath(file);

    if (file != currentDatabaseName ||
        (timeState != currentDatabaseTimeState && currentDatabaseHasInvariantMD))
    {
        string timerMessage(string("Time to open ") + file);
        int    timeid = visitTimer->StartTimer();
        debug2 << "MDServerConnection::GetDatabase: Need to get a new database"
               << ". file=" << file.c_str()
               << ", timeState=" << timeState << endl;

        if (currentDatabase != NULL)
        {
            delete currentDatabase;
            currentDatabase = NULL;
        }
        currentDatabaseName = file;
        currentDatabaseTimeState = timeState;
        const char *fn = file.c_str();
        VirtualFileInformationMap::iterator virtualFile = 
            GetVirtualFileDefinition(file);

        if (virtualFile != virtualFiles.end())
        {
#if defined(_WIN32)
            // Save the old path in case we have to change it.
            std::string oldPath(currentWorkingDirectory);
#endif
            // Make an array of strings that contain the entire name of the
            // various timesteps so we can pass it to the database factory.
            const stringVector &fileNames = virtualFile->second.files;
            const std::string &path = virtualFile->second.path;
            char **names = new char *[fileNames.size()];
            int i;
            debug3 << "New virtual database: " << file.c_str()
                   << ", path=" << path.c_str() << endl;
            for(i = 0; i < fileNames.size(); ++i)
            {
                std::string name(ExpandPathHelper(fileNames[i], path));
                char *charName = new char[name.size() + 1];
                strcpy(charName, name.c_str());
                names[i] = charName;
                debug3 << "\t" << charName << endl;
            }

            TRY
            {
#if defined(_WIN32)
                // Change to the virtual file's path if we're not already
                // there so we can read the files.
                if(currentWorkingDirectory != path)
                {
                    debug2 << "Have to change to the virtual file's path: "
                           << path.c_str() << endl;
                    ChangeDirectory(path);
                }
#endif
                //
                // If we're asking for a time state that is larger than the
                // number of time states in the virtual database, such as
                // when we reopen a virtual database after files have
                // disappeared, we should clamp the desired time state.
                //
                if(timeState >= fileNames.size())
                {
                    debug2 << "The desired time state is larger than the "
                              "number of time states in the virtual database "
                              "so the time state is being clamped to "
                           << fileNames.size() - 1 << endl;
                    timeState = fileNames.size() - 1;
                }

                // Try and make a database out of the filenames.
                currentDatabase = avtDatabaseFactory::FileList(names,
                    fileNames.size(), timeState);

                // Free the memory that we used.
                for(i = 0; i < fileNames.size(); ++i)
                    delete [] names[i];
                delete [] names;

#if defined(_WIN32)
                if(oldPath != currentWorkingDirectory)
                {
                    debug2 << "Changing back to the real current directory: "
                           << oldPath.c_str() << endl;
                    ChangeDirectory(oldPath);
                }
#endif
            }
            CATCH(VisItException)
            {
                visitTimer->StopTimer(timeid, timerMessage);

                // Free the memory that we used.
                for(i = 0; i < fileNames.size(); ++i)
                    delete [] names[i];
                delete [] names;

#if defined(_WIN32)
                if(oldPath != currentWorkingDirectory)
                {
                    debug2 << "Changing back to the real current directory: "
                           << oldPath.c_str() << endl;
                    ChangeDirectory(oldPath);
                }
#endif
                RETHROW;
            }
            ENDTRY
        }
        else if (FileHasVisItExtension(file))
        {
            currentDatabase = avtDatabaseFactory::VisitFile(fn, timeState);
        }
        else
        {
            currentDatabase = avtDatabaseFactory::FileList(&fn, 1, timeState);
        }

        visitTimer->StopTimer(timeid, timerMessage);
    }

    return currentDatabase;
}

// ****************************************************************************
// Method: MDServerConnection::CloseDatabase
//
// Purpose: 
//   Closes the open database so it is re-read the next time we access it.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:33:03 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Mar 22 09:43:27 PDT 2004
//   Added code to remove the open database from the virtual file map if
//   it is a virtual file so the next time we ask for it, we'll read the
//   directory and get the right list of time states.
//
// ****************************************************************************

void
MDServerConnection::CloseDatabase()
{
    VirtualFileInformationMap::iterator virtualFile = virtualFiles.find(currentDatabaseName);

    if(virtualFile != virtualFiles.end())
    {
        debug1 << "Removing " << currentDatabaseName.c_str()
               << " from the virtual file map." << endl;
        virtualFiles.erase(virtualFile);
    }

    if (currentDatabase != NULL)
    {
        debug1 << "Closing database: " << currentDatabaseName.c_str() << endl;
        delete currentDatabase;
        currentDatabase = NULL;
        currentDatabaseName = "";
    }
}

// ****************************************************************************
// Class: VirtualFileName
//
// Purpose:
//   This class is a string that sorts using numeric string sorting.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 27 11:43:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MDServerConnection::VirtualFileName::VirtualFileName() : name()
{
}

MDServerConnection::VirtualFileName::VirtualFileName(const MDServerConnection::VirtualFileName &obj) : name(obj.name)
{
}

MDServerConnection::VirtualFileName::VirtualFileName(const std::string &obj) : name(obj)
{
}

MDServerConnection::VirtualFileName::~VirtualFileName()
{
}

void 
MDServerConnection::VirtualFileName::operator = (const MDServerConnection::VirtualFileName &obj)
{
    name = obj.name;
}

bool
MDServerConnection::VirtualFileName::operator == (const MDServerConnection::VirtualFileName &obj) const
{
    return (name == obj.name);
}

bool
MDServerConnection::VirtualFileName::operator < (const MDServerConnection::VirtualFileName &obj) const
{
    bool retval = false;

    if(name != obj.name)
        retval = NumericStringCompare(name, obj.name);

    return retval;
}

// ****************************************************************************
// Class: MDServerConnection::VirtualFileInformation
//
// Purpose:
//   This class is a container for a path and list of filenames that can
//   be found at that path.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 2 12:32:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MDServerConnection::VirtualFileInformation::VirtualFileInformation() : path(),
    files()
{
}

MDServerConnection::VirtualFileInformation::VirtualFileInformation(
   const MDServerConnection::VirtualFileInformation &obj) : path(obj.path),
   files(obj.files)
{
}

MDServerConnection::VirtualFileInformation::~VirtualFileInformation()
{
}

void
MDServerConnection::VirtualFileInformation::operator = (
   const MDServerConnection::VirtualFileInformation &obj)
{
    path = obj.path;
    files = obj.files;
}
