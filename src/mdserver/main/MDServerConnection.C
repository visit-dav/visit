/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
#include <snprintf.h>

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
#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <DBOptionsAttributes.h>
#include <DBPluginInfoAttributes.h>
#include <DebugStream.h>
#include <ExpandPathRPC.h>
#include <ExpandPathRPCExecutor.h>
#include <GetDBPluginInfoRPC.h>
#include <GetDBPluginInfoRPCExecutor.h>
#include <GetDirectoryRPC.h>
#include <GetDirectoryRPCExecutor.h>
#include <GetFileListRPC.h>
#include <GetFileListRPCExecutor.h>
#include <GetMetaDataRPC.h>
#include <GetMetaDataRPCExecutor.h>
#include <GetPluginErrorsRPC.h>
#include <GetPluginErrorsRPCExecutor.h>
#include <GetSILRPC.h>
#include <GetSILRPCExecutor.h>
#include <KeepAliveRPC.h>
#include <KeepAliveRPCExecutor.h>
#include <LoadPluginsRPC.h>
#include <LoadPluginsRPCExecutor.h>
#include <CouldNotConnectException.h>
#include <IncompatibleVersionException.h>
#include <InvalidFilesException.h>
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
//    Brad Whitlock, Fri Feb 4 15:07:42 PST 2005
//    I added filterList and extraSmartFileGrouping.
//
//    Jeremy Meredith, Tue Feb  8 08:48:34 PST 2005
//    Added the ability to query for plugin initialization errors.
//
//    Hank Childs, Wed May 25 10:29:30 PDT 2005
//    Added new RPCs.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support for specifying cycle number regular expression 
// ****************************************************************************

MDServerConnection::MDServerConnection(int *argc, char **argv[])
{
    int total = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();
    string connectStr("Connecting to client");

    for (int i = 0; i < *argc; i++)
    {
        if (strcmp((*argv)[i], "-cycleregex") == 0)
        {
            avtDatabaseMetaData::SetCycleFromFilenameRegex((*argv)[i+1]);
            i++;
        }
    }

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

    // Make the default filter be "*"
    filterList.push_back("*");
    extraSmartFileGrouping = true;

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
    getPluginErrorsRPC = new GetPluginErrorsRPC;
    getDBPluginInfoRPC = new GetDBPluginInfoRPC;

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
    xfer->Add(getPluginErrorsRPC);
    xfer->Add(getDBPluginInfoRPC);

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
    getPluginErrorsRPCExecutor = new GetPluginErrorsRPCExecutor(this, getPluginErrorsRPC);
    getDBPluginInfoRPCExecutor = new GetDBPluginInfoRPCExecutor(this, getDBPluginInfoRPC);

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
//   Jeremy Meredith, Tue Feb  8 08:48:34 PST 2005
//   Added the ability to query for plugin initialization errors.
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
    delete getPluginErrorsRPCExecutor;

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
    delete getPluginErrorsRPC;

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
//    Jeremy Meredith, Tue Feb  8 08:49:46 PST 2005
//    Move the initialization of the plugins to the program initialization.
//    It is the cheaper of the operations and guarantees that we can later
//    query for their initialization errors and have a meaningful answer.
//
// ****************************************************************************

void
MDServerConnection::LoadPlugins()
{
    if(!pluginsLoaded)
    {
        debug2 << "Loading plugins!" << endl;
        DatabasePluginManager::Instance()->LoadPluginsNow();
        pluginsLoaded = true;
    }
}


// ****************************************************************************
//  Method:  MDServerConnection::GetPluginErrors
//
//  Purpose:
//    Query the plugin manager(s) for initialization errors.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  7, 2005
//
// ****************************************************************************
std::string
MDServerConnection::GetPluginErrors()
{
    return DatabasePluginManager::Instance()->GetPluginInitializationErrors();
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
//   Jeremy Meredith, Wed Aug 25 11:41:54 PDT 2004
//   Handle errors through exceptions instead of error codes.   This allows
//   real error messages to make it to the user.
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added bool arg forceReadAllCyclesAndTimes
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added use of forceReadAllCyclesAndTimes in call to GetDatabase
//
//   Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//   Added ability to force using a specific plugin when reading
//   the metadata from a file (if it causes the file to be opened).
//
//   Hank Childs, Thu Jan 11 16:14:25 PST 2007
//   Capture the list of plugins used to open a file.
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added flags controlling creation of MeshQuality and TimeDerivative 
//   expressions. Send the flags to the database factory.
//
//   Cyrus Harrison, Wed Nov 28 11:17:23 PST 2007
//   Added flag for creation of vector magnitude expressions
//
//   Hank Childs, Wed Dec 19 08:39:46 PST 2007
//   Added timing information.
//
// ****************************************************************************

void
MDServerConnection::ReadMetaData(std::string file, int timeState,
                                 bool forceReadAllCyclesAndTimes,
                                 std::string forcedFileType,
                                 bool treatAllDBsAsTimeVarying,
                                 bool createMeshQualityExpressions,
                                 bool createTimeDerivativeExpressions,
                                 bool createVectorMagnitudeExpressions)
{
    currentMetaData = NULL;

    int ts = (timeState == -1) ? currentDatabaseTimeState : timeState;
    debug2 << "Read the Metadata for " << file.c_str()
           << ", timeState=" << ts
           << ", forceReadAllCyclesAndTimes=" << forceReadAllCyclesAndTimes
           << ", forcedFileType=" << forcedFileType
           << ", treatAllDBsAsTimeVarying = " << treatAllDBsAsTimeVarying
           << ", createMeshQualityExpressions = " 
           << createMeshQualityExpressions
           << ", createTimeDerivativeExpressions = " 
           << createTimeDerivativeExpressions      
           << ", createVectorMagnitudeExpressions = " 
           << createVectorMagnitudeExpressions
           << endl;

    avtDatabaseFactory::SetCreateMeshQualityExpressions(
        createMeshQualityExpressions);
    avtDatabaseFactory::SetCreateTimeDerivativeExpressions(
        createTimeDerivativeExpressions);
    avtDatabaseFactory::SetCreateVectorMagnitudeExpressions(
        createVectorMagnitudeExpressions);

    //
    // Try and read the database. This could throw an exception.
    //
    std::vector<std::string> plugins;
    avtDatabase *db = NULL;
    TRY
    {
        int t0 = visitTimer->StartTimer();
        db = GetDatabase(file, ts, forceReadAllCyclesAndTimes,
                 plugins, forcedFileType, treatAllDBsAsTimeVarying);
        visitTimer->StopTimer(t0, "Get database from inside ReadMetaData");
    }
    CATCH2(VisItException, e)
    {
        EXCEPTION3(InvalidFilesException, file.c_str(), plugins, 
                                          e.Message().c_str());
    }
    ENDTRY

    if (db != NULL)
    {
        TRY
        {
            int t0 = visitTimer->StartTimer();
            currentMetaData = db->GetMetaData(ts, forceReadAllCyclesAndTimes,
                                              treatAllDBsAsTimeVarying);
            visitTimer->StopTimer(t0, "Get metadata from inside ReadMetaData");
        }
        CATCHALL(...)
        {
            EXCEPTION2(InvalidFilesException, file.c_str(), plugins);
        }
        ENDTRY
    }
    else
    {
        EXCEPTION2(InvalidFilesException, file.c_str(), plugins);
    }

    //
    // If we didn't get metadata, then the file was somehow invalid
    //
    if (!currentMetaData)
    {
        char message[1024];
        sprintf(message, "The %s file format was unable to retrieve metadata "
                "for this file.", db->GetFileFormat().c_str());
        EXCEPTION2(InvalidFilesException, file.c_str(), message);
    }

    //
    // If we have metadata for the file then set whether or not the file is a
    // virtual file.
    //
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
//   Jeremy Meredith, Wed Aug 25 11:41:54 PDT 2004
//   Handle errors through exceptions instead of error codes.   This allows
//   real error messages to make it to the user.
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added bool arg to GetDatabase
//
//   Hank Childs, Thu Jan 11 16:14:25 PST 2007
//   Capture the list of plugins used to open a file.
//
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Added treatAllDBsAsTimeVarying
// ****************************************************************************

void
MDServerConnection::ReadSIL(std::string file, int timeState,
    bool treatAllDBsAsTimeVarying)
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
    std::vector<std::string> plugins;
    avtDatabase *db = GetDatabase(file, ts, false, plugins);
    if (db != NULL)
    {
        avtSIL *s  = db->GetSIL(ts, treatAllDBsAsTimeVarying);

        // Delete the SIL attributes if they are not NULL.
        if(currentSIL != NULL)
            delete currentSIL;

        currentSIL = s->MakeSILAttributes();
    }
    else
    {
        EXCEPTION2(InvalidFilesException, file.c_str(), plugins);
    }

    if (!currentSIL)
    {
        char message[1024];
        sprintf(message, "The %s file format was unable to retrieve the SIL "
                "for this file.", db->GetFileFormat().c_str());
        EXCEPTION2(InvalidFilesException, file.c_str(), message);
    }
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
// Method: MDServerConnection::GetDBPluginInfo
//
// Purpose: 
//   Returns the DB plugin info.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// Modifications:
//
//   Hank Childs, Wed Jul  6 07:19:03 PDT 2005
//   Fix memory leak.
//
//   Jeremy Meredith, Thu Oct 11 14:54:13 EDT 2007
//   Enhancements to attribute groups allowed separate vectors for the
//   read and write options, so I switched to this simpler organization.
//
// ****************************************************************************

DBPluginInfoAttributes *
MDServerConnection::GetDBPluginInfo()
{
    LoadPlugins();

    DBPluginInfoAttributes *rv = new DBPluginInfoAttributes;

    DatabasePluginManager *manager = DatabasePluginManager::Instance();
    int nPlugins = manager->GetNEnabledPlugins();
    std::vector<std::string> types(nPlugins);
    std::vector<std::string> fullnames(nPlugins);
    std::vector<int>         hasWriter(nPlugins);
    for (int i = 0 ; i < manager->GetNEnabledPlugins() ; i++)
    {
        std::string fullname = manager->GetEnabledID(i);
        fullnames[i] = fullname;
        std::string name = manager->GetPluginName(fullname);
        types[i] = name;
        hasWriter[i] = manager->PluginHasWriter(fullname);
        CommonDatabasePluginInfo *info =manager->GetCommonPluginInfo(fullname);
        DBOptionsAttributes *a = info->GetReadOptions();
        rv->AddDbReadOptions(*a);
        delete a;
        a = info->GetWriteOptions();
        rv->AddDbWriteOptions(*a);
        delete a;
    }
    rv->SetTypes(types);
    rv->SetTypesFullNames(fullnames);
    rv->SetHasWriter(hasWriter);

    return rv;
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
//   Brad Whitlock, Thu Feb 17 15:05:34 PST 2005
//   Moved some code into the utility library, callable from ExpandUserPath.
//
// ****************************************************************************

std::string
MDServerConnection::ExpandPathHelper(const std::string &path,
    const std::string &workingDir) const
{
    std::string newPath;

#if defined(_WIN32)
    char driveLetter;

    if(path[0] == '~')
    {
        newPath = ExpandUserPath(path);
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
        newPath = path;
    }
    else
    {
        // relative path:
        newPath = workingDir + "\\" + path;
    }
#else
    if(path[0]=='~')
    {
        newPath = ExpandUserPath(path);
    }
    else if(path[0] != '/')
    {
        // relative path:
        newPath = workingDir + "/" + path;
    }
    else
    {
        // absolute path: do nothing
        newPath = path;
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
//   Hank Childs, Wed Nov 10 16:15:19 PST 2004
//   Do not call readdir if opendir failed.  This is because readdir will
//   segfault on MCR due to bizarre Lustre group read permission issues.
//   
//   Brad Whitlock, Wed Dec 14 17:01:53 PST 2005
//   I moved the code to stat files out of this method so we now just get
//   file list (on UNIX anyway).
//
// ****************************************************************************

void
MDServerConnection::ReadFileList()
{
    int timeid, total = visitTimer->StartTimer();

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
    timeid = visitTimer->StartTimer();
    GetFileListRPC::FileList &fl = currentFileList;
    currentFileList.Clear();
    visitTimer->StopTimer(timeid, "Clearing file list");

    // If the directory cannot be opened, return an error code.
    DIR     *dir;
    dirent  *ent;
    timeid = visitTimer->StartTimer();
    dir = opendir(currentWorkingDirectory.c_str());
    if (!dir)
        readFileListReturnValue = -1;
    visitTimer->StopTimer(timeid, "Opening directory");

    // Add each directory entry to the file list.
    timeid = visitTimer->StartTimer();
    bool haveDot = false;
    bool haveDotDot = false;
    if (dir != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            haveDot = haveDot || (strcmp(ent->d_name, ".") == 0);
            haveDotDot = haveDotDot || (strcmp(ent->d_name, "..") == 0);

            fl.names.push_back(ent->d_name);
            fl.types.push_back(GetFileListRPC::UNCHECKED);
            fl.sizes.push_back(0);
            fl.access.push_back(1);
        }
    }
    visitTimer->StopTimer(timeid, "Copying filenames");

    closedir(dir);
#endif

    // Sort the file list.
    timeid = visitTimer->StartTimer();
    currentFileList.Sort();
    visitTimer->StopTimer(timeid, "Sorting file list");
    visitTimer->StopTimer(total, std::string("ReadFileList: ") +
                                 currentWorkingDirectory);
}

// ****************************************************************************
// Method: MDServerConnection::ReadFileListAttributes
//
// Purpose: 
//   This method calls stat on all of the files in the passed in FileList 
//   when they have UNCHECKED type.
//
// Arguments:
//   fl : The file list whose files we're checking.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 14 17:03:28 PST 2005
//
// Modifications:
//   
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
//
// ****************************************************************************

void
MDServerConnection::ReadFileListAttributes(GetFileListRPC::FileList &fl,
    bool allowRemoval)
{
#if !defined(_WIN32)
    int total = visitTimer->StartTimer();

    // Get the userId and the groups for that user so we can check the
    // file permissions.
    uid_t uid = getuid();
    gid_t gids[100];
    int ngids;
    ngids = getgroups(100, gids);

    // Check each of the UNCHECKED files.
    unsigned int nStat = 0;
    GetFileListRPC::FileList fl2;
    for(unsigned int i = 0; i < fl.names.size(); ++i)
    {
        GetFileListRPC::file_types origType = (GetFileListRPC::file_types)fl.types[i];

        if(origType == GetFileListRPC::UNCHECKED ||
           (allowRemoval &&
            origType == GetFileListRPC::UNCHECKED_REMOVE_IF_NOT_DIR))
        {
            ++nStat;
            VisItStat_t s;
            VisItStat((currentWorkingDirectory + "/" + fl.names[i]).c_str(), &s);
    
            mode_t mode = s.st_mode;

            bool isdir = S_ISDIR(mode);
            bool isreg = S_ISREG(mode);
    
            fl.types[i] = (isdir ? GetFileListRPC::DIR :
                           isreg ? GetFileListRPC::REG : 
                                   GetFileListRPC::UNKNOWN);
            fl.sizes[i] = (long)s.st_size;
    
            bool isuser  = (s.st_uid == uid);
            bool isgroup = false;
            for (int j=0; j<ngids && !isgroup; ++j)
                if (s.st_gid == gids[j])
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

            fl.access[i] = (canaccess ? 1 : 0);
        }

        if(allowRemoval)
        {
            if(origType == GetFileListRPC::UNCHECKED_REMOVE_IF_NOT_DIR)
            {
                if(fl.types[i] == GetFileListRPC::DIR)
                {
                    fl2.names.push_back(fl.names[i]);
                    fl2.types.push_back(fl.types[i]);
                    fl2.sizes.push_back(fl.sizes[i]);
                    fl2.access.push_back(fl.access[i]);
                }
            }
            else
            {
                fl2.names.push_back(fl.names[i]);
                fl2.types.push_back(fl.types[i]);
                fl2.sizes.push_back(fl.sizes[i]);
                fl2.access.push_back(fl.access[i]);
            }
        }
    }

    // Replace the input file list with one that has had UNCHECKED elements
    // that were not directories removed.
    if(allowRemoval)
        fl = fl2;

    // Keep track of the time needed to stat the files in the directory.
    char s[1024];
    SNPRINTF(s, 1024, "Stat %d files in %s", nStat,
             currentWorkingDirectory.c_str());
    visitTimer->StopTimer(total, s);
#endif
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
//   Brad Whitlock, Wed Dec 14 17:05:10 PST 2005
//   I made it read the file attributes if necessary.
//
// ****************************************************************************

GetFileListRPC::FileList *
MDServerConnection::GetCurrentFileList()
{
    // Reads the file attributes if necessary.
    ReadFileListAttributes(currentFileList, false);

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
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 15:19:12 PST 2000
//
// Modifications:
//   Jeremy Meredith, Fri Mar 19 14:46:24 PST 2004
//   I made it use WildcardStringMatch from Utility.h.
//
//   Brad Whitlock, Fri Feb 4 15:08:59 PST 2005
//   I removed the filterList argument.
//
// ****************************************************************************

bool
MDServerConnection::FileMatchesFilterList(const std::string &fileName) const
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
//   file        : The filename.
//   pattern     : The pattern string to be returned.
//   digitLength : The length of the digit string.
//
// Returns:    True if a pattern was detected.
//
// Note:       Based on Eric's pattern matching code in MeshTV.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 10:57:06 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 26 13:29:38 PST 2005
//   I made it acceptable to replace digits before an underscore with the
//   wildcard so files with names like: foo_00000_root.ext work. I also made
//   Flash file dumps ending in Z be considered for automatic grouping. Finally,
//   I made it return the length of the digit string.
//
// ****************************************************************************

bool
MDServerConnection::GetPattern(const std::string &file, std::string &p,
    int &digitLength) const
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
    digitLength = 0;
    for (i = isave; file[i] >= '0' && file[i] <= '9'; i++)
        ++digitLength;
    char charAfterDigit = file[i];

    // If we're doing extra smart file grouping then be a little more lenient
    // when considering patterns.
    bool specialMatch = false;
    if(extraSmartFileGrouping)
    {
        specialMatch = (charAfterDigit == '_') ||
                       (charAfterDigit == 'z' && (i == file.size()-1));
    }

    /* We have a match on *[0-9]?.  Now let's determine the full pattern.  */
    if (file[isave] >= '0' && file[isave] <= '9' &&
        (charAfterDigit == '\0' || charAfterDigit == '.'  || specialMatch)
       )
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
// Method: MDServerConnection::SetFileGroupingOptions
//
// Purpose: 
//   Sets the file filter and file grouping options that should be used
//   when filtering the file list.
//
// Arguments:
//   filter                 : A space separated list of filters.
//   extraSmartFileGrouping : Whether extra smart file grouping should be used.
// Returns:    
//
// Note:       This used to be in GetFilteredFileList but I moved it here so
//             implicit definition of virtual databases would at least use the
//             last filters that we defined instead of "*".
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 4 15:09:34 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
MDServerConnection::SetFileGroupingOptions(const std::string &filter,
    bool extraSmartGrouping)
{
    //
    // Parse the filter string into a list of filter strings.
    //

    // Parse the filter string and store the result in filterList.
    char *temp  = new char[filter.size() + 1];
    char *ptr, *temp2 = temp;
    memcpy((void *)temp, (void *)filter.c_str(), filter.size() + 1);
    filterList.clear();
    while((ptr = strtok(temp2, " ")) != NULL)
    {
        filterList.push_back(std::string(ptr));
        temp2 = NULL;
    }
    delete[] temp;
    if(filterList.size() == 0)
        filterList.push_back("*");

    extraSmartFileGrouping = extraSmartGrouping;
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
//   Brad Whitlock, Fri Aug 27 17:39:32 PST 2004
//   I fixed a bug that caused multiple virtual databases to get the
//   definitions confused if they had similar names.
//
//   Brad Whitlock, Fri Feb 4 15:12:40 PST 2005
//   I made the filter list and extraSmartFileGrouping flag be set elsewhere.
//
//   Brad Whitlock, Tue Apr 26 13:58:11 PST 2005
//   Added code to group "abort" files into their related database.
//
//   Brad Whitlock, Mon Jun 27 14:22:59 PST 2005
//   I made the code to consolidate virtual databases be off unless the 
//   VISIT_CONSOLIDATE_VIRTUAL_DATABASES environment variable is set to "on".
//   This prevents users from always running into it where it might cause
//   problems for a small subset of users. Those who want the feature can
//   set the environment variable.
//
// ****************************************************************************

void
MDServerConnection::GetFilteredFileList(GetFileListRPC::FileList &files)
{
    int total = visitTimer->StartTimer();

    //
    // If we have not yet read a file list, read it now.
    //
    if(!validFileList)
        ReadFileList();

    //
    // Compare each name in the current file list against the list of
    // filters to see if it is an acceptable filename.
    //
    int stage1 = visitTimer->StartTimer();
    int i;
    const stringVector &names = currentFileList.names;
    std::string pattern;
    VirtualFileInformationMap newVirtualFiles;
    VirtualFileInformationMap::iterator pos;

    for(i = 0; i < names.size(); ++i)
    {
        if(currentFileList.types[i] == GetFileListRPC::REG ||
           currentFileList.types[i] == GetFileListRPC::UNCHECKED)
        {
            //
            // See if the file matches any of the filters.
            //
            if(FileMatchesFilterList(names[i]))
            {
                //
                // See if the filename matches a pattern for related files.
                //
                int digitLength = 0;
                bool matchesPattern = GetPattern(names[i], pattern, digitLength);

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
                    // Try some more advanced file grouping rules.
                    if(extraSmartFileGrouping)
                    {
                        //
                        // Try to not group certain ale3d files. Those files
                        // are of the form: X, X.1, X.2, X.3, ... We only want
                        // to group the X files and leave the X.1, X.2, X.3
                        // files unrelated.
                        //
                        if(FileLooksLikePartFile(newVirtualFiles, pattern))
                        {
                            // Add the filename to the list of
                            // filtered files.
                            files.names.push_back(names[i]);
                            files.types.push_back(currentFileList.types[i]);
                            files.sizes.push_back(currentFileList.sizes[i]);
                            files.access.push_back(currentFileList.access[i]);
                            continue;
                        }

                        //
                        // If the pattern contains "_abort_" then it might be
                        // an ale3d abort file. Take out the "_abort_" string 
                        // and replace it with "_". If there's a pattern that
                        // matches the new pattern without the "_abort_" string
                        // then assume we have an abort file and the file should
                        // be grouped with the pattern that we found.
                        // 
                        int apos;
                        if((apos = pattern.find("_abort_")) != -1)
                        {
                            std::string pattern2(pattern);
                            pattern2.replace(apos, 7, "_");
                            pos = newVirtualFiles.find(pattern2);
                            if(pos != newVirtualFiles.end())
                            {
                                std::string altName(names[i]);
                                apos = altName.find("_abort_");
                                altName.replace(apos, 7, "_");
                                // Since the abort filename is most likely to 
                                // come after the regularly numbered files, we
                                // have to insert the abort file name into the
                                // virtual database definition to prevent the
                                // name from just getting tacked on to the end.
                                stringVector::iterator it = pos->second.files.begin();
                                for( ; it != pos->second.files.end(); ++it)
                                {
                                    std::string listName(*it);
                                    apos = listName.find("_abort_");
                                    if(apos != -1)
                                        listName.replace(apos, 7, "_");

                                    if(listName >= altName)
                                        break;
                                }
                                pos->second.files.insert(it, names[i]);
                                debug4 << "Grouping " << names[i].c_str()
                                       << " into " << pattern2.c_str() << endl;
                                continue;
                            }
                            else if(FileLooksLikePartFile(newVirtualFiles, pattern2))
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
                        newVirtualFiles[pattern].digitLength = digitLength;
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
            else if(currentFileList.types[i] == GetFileListRPC::UNCHECKED)
            {
                //
                // The files did not match anything in the filter so let's
                // add it to the list of files so we can stat it but give it
                // a type so that we'll remove it from the list of files if
                // the file is not a directory.
                //
                // If we wanted to make both files and directories subject
                // to the filter string code then we can omit this code here
                // but we'd have to add "." and ".." directories to the list
                // of files to pass on.
                //
                files.names.push_back(names[i]);
                files.types.push_back(GetFileListRPC::UNCHECKED_REMOVE_IF_NOT_DIR);
                files.sizes.push_back(0);
                files.access.push_back(1);
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
    visitTimer->StopTimer(stage1, "stage1: Assembling files list");

    //
    // Now that we've assembled a list of filenames, go back and fix the
    // ones that had a pattern but had no successors and thus were only
    // one file long. Note that we're traversing the files.names vector
    // instead of the keys in the newVirtualFiles map so we are guaranteed
    // to get the same filename ordering, which is important for the way
    // we store the virtual file information in the files object.
    //
    if(newVirtualFiles.size() > 0)
    {
        // Try and group virtual databases that have similar names into a 
        // larger virtual database if the names look like what we'd get
        // with Flash files.
        int stage2 = visitTimer->StartTimer();
        if(extraSmartFileGrouping)
        {
            const char *cvdbs = getenv("VISIT_CONSOLIDATE_VIRTUAL_DATABASES");
            bool allowConsolidate = (cvdbs != 0 && strcmp(cvdbs,"on") == 0);
            if(allowConsolidate)
                ConsolidateVirtualDatabases(newVirtualFiles, files);
        }
        visitTimer->StopTimer(stage2, "stage2: consolidate virtual databases");

        //
        // Create virtual databases using the information stored in the
        // newVirtualFiles map.
        //
        int fileIndex, stage3 = visitTimer->StartTimer();
        GetFileListRPC::FileList virtualFilesToCheck;
        for(fileIndex = 0; fileIndex < files.names.size(); ++fileIndex)
        {
            // Look for the current filename in the new virtual files map. If the
            // name is not in the list then it's not a virtual file.
            pos = newVirtualFiles.find(files.names[fileIndex]);
            if(pos == newVirtualFiles.end())
                continue;
        
            if(pos->second.files.size() == 1)
            {
                // Change the name in the files list back to the original file
                // name, replacing the pattern string that we had inserted.
                files.names[fileIndex] = pos->second.files[0];

                // Erase the entry from newVirtualFiles because it will not be
                // considered to be a virtual database.
                newVirtualFiles.erase(pos);
            }
            else
            {
                virtualFilesToCheck.names.push_back(pos->second.files[0]);
                virtualFilesToCheck.types.push_back(files.types[fileIndex]);
                virtualFilesToCheck.sizes.push_back(files.sizes[fileIndex]);
                virtualFilesToCheck.access.push_back(files.access[fileIndex]);

                // Mark the file as virtual in the file list so we won't get the
                // file type if we needed it.
                files.types[fileIndex] = GetFileListRPC::VIRTUAL;
            }
        }
        visitTimer->StopTimer(stage3, "stage3: populating virtualFilesToCheck");

        int stage4 = visitTimer->StartTimer();
        ReadFileListAttributes(files, true);
        ReadFileListAttributes(virtualFilesToCheck, false);
        visitTimer->StopTimer(stage4, "stage4: getting file attributes");

        // Now that we have the types for the files and for the first file in
        // each virtual database, remove any virtual databases that are not
        // composed of files.
        int stage5 = visitTimer->StartTimer();
        int vfIndex = 0;
        bool needToSortFileList = false;
        for(fileIndex = 0; fileIndex < files.names.size(); ++fileIndex)
        {
            pos = newVirtualFiles.find(files.names[fileIndex]);
            if(pos == newVirtualFiles.end())
                continue;

            if(virtualFilesToCheck.types[vfIndex] != GetFileListRPC::REG)
            {
                debug5 << "File " << files.names[fileIndex].c_str()
                       << " is not a file so we're adding its components back "
                          "to the files list." << endl;

                // Restore the name and file type for the first element in the
                // virtual database.
                files.names[fileIndex] = pos->second.files[0];
                files.types[fileIndex] = virtualFilesToCheck.types[vfIndex];
                debug5 << "\t" << pos->second.files[0].c_str() << endl;

                // The virtual database is not of files so it cannot be a virtual
                // database. Put the rest of its files back into the files list.
                for(int i = 1; i < pos->second.files.size(); ++i)
                {
                    debug5 << "\t" << pos->second.files[i].c_str() << endl;

                    files.names.push_back(pos->second.files[i]);
                    files.types.push_back(virtualFilesToCheck.types[vfIndex]);
                    files.sizes.push_back(virtualFilesToCheck.sizes[vfIndex]);
                    files.access.push_back(virtualFilesToCheck.access[vfIndex]);
                }

                newVirtualFiles.erase(pos);
                needToSortFileList = true;
            }
            ++vfIndex;
        }
        visitTimer->StopTimer(stage5, "stage5: removing invalid virtual databases");

        int stage6 = visitTimer->StartTimer();
        if(needToSortFileList)
            files.Sort();
        visitTimer->StopTimer(stage6, "stage6: sorting file list");

        // These remaining virtual databases should be files.
        int stage7 = visitTimer->StartTimer();
        if(newVirtualFiles.size() > 0)
        {
            for(int fileIndex = 0; fileIndex < files.names.size(); ++fileIndex)
            {
                pos = newVirtualFiles.find(files.names[fileIndex]);
                if(pos == newVirtualFiles.end())
                    continue;

                // Determine a good root name for the database.
                std::string rootName(pos->first.name + " database");

                // Change the name in the files list from the pattern name
                // to the new root name. Also change the file type to VIRTUAL.
                files.names[fileIndex] = rootName;
                files.types[fileIndex] = GetFileListRPC::VIRTUAL;

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
                virtualFiles[key].filterList = filterList;
                virtualFiles[key].files.swap(pos->second.files);
            }
        }
        visitTimer->StopTimer(stage6, "stage7: adding final virtual databases");
    }
    else
        ReadFileListAttributes(files, true);

    // We have gotten the file information for all of the files in the files
    // list. Should we put that information back into the currentFileList?
    visitTimer->StopTimer(total, std::string("GetFilteredFileList: ") +
                          currentWorkingDirectory);
}

// ****************************************************************************
// Method: MDServerConnection::FileLooksLikePartFile
//
// Purpose: 
//   Returns whether or not the pattern that was passed in corresponds to
//   an ale3d part file (the sub-domain files that end in .1, .2, .3, ...).
//
// Arguments:
//   newVirtualFiles : The virtual file definitions.
//   pattern         : The pattern we're checking.
//                     
// Returns:    True if the file is looks like an ale3d part file; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 26 14:30:03 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
MDServerConnection::FileLooksLikePartFile(const VirtualFileInformationMap &
    newVirtualFiles, const std::string &pattern) const
{
    bool retval = false;
    if(pattern.size() >= 2 &&
       pattern.substr(pattern.size() - 2) == ".*")
    {
        std::string pattern2;
        int digitLength;
        bool found2ndPattern = GetPattern(pattern, pattern2, digitLength);
        if(found2ndPattern)
        {
            std::string basepattern(pattern2.substr(0,pattern2.size() - 2));
            VirtualFileInformationMap::const_iterator pos =
                newVirtualFiles.find(basepattern);
            if(pos != newVirtualFiles.end())
            {
                retval = true;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: MDServerConnection::ConsolidateVirtualDatabases
//
// Purpose: 
//   Consolidates related virtual databases into a single virtual database.
//
// Arguments:
//   newVirtualFiles : The map that contains the virtual database definitions.
//   files           : The object that contains the file information.
//
// Note:       This method is executed when we have smart file grouping on
//             and it should only affect Flash virtual databases.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 26 17:59:03 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
MDServerConnection::ConsolidateVirtualDatabases(
    VirtualFileInformationMap &newVirtualFiles,
    GetFileListRPC::FileList &files)
{
    //
    // Make lists of all of the virtual databases that have their "*"
    // character in the last or second to last position and that have
    // the character immediately before the "*" character in ['a','z'].
    //
    VirtualFileInformationMap flashVirtualFiles;
    VirtualFileInformationMap::iterator it;
    for(it = newVirtualFiles.begin(); it != newVirtualFiles.end(); ++it)
    {
        std::string key(it->first.name);
        int starpos = key.find("*");

        bool lastCharIsStar = starpos == key.size()-1;
        bool secondLastCharIsStarWithZ = starpos == key.size()-2 &&
                                         key[key.size()-1] == 'z';
        int beforeStar = starpos - 1;
        bool beforeStarInAZ = false;
        if(beforeStar >= 0)
            beforeStarInAZ = (key[beforeStar] >= 'a') && (key[beforeStar] <= 'z');

        if((lastCharIsStar || secondLastCharIsStarWithZ) && beforeStarInAZ)
        {
            // Replace the character before the string with a 2 character
            // hex number that represents the length of the digit string
            // in the original file's pattern string. We will not allow
            // databases with different digit lengths to be merged.
            char digitLenString[3];
            int digitLen = (it->second.digitLength < 255) ?
                it->second.digitLength : 255;
            SNPRINTF(digitLenString, 3, "%02x", digitLen);
            key.replace(beforeStar, 1, digitLenString);

            // Add the virtual database under the altered key, containing the
            // digit string.
            VirtualFileInformationMap::iterator f =
                flashVirtualFiles.find(key);
            if(f == flashVirtualFiles.end())
            {
                stringVector sv; sv.push_back(it->first.name);
                flashVirtualFiles[key].files = sv;
            }
            else
                flashVirtualFiles[key].files.push_back(it->first.name);
        }
    }

    //
    // If we were able to group any Flash file virtual databases,
    // consolidate those virtual databases now.
    //
    bool consolidatedDBs = false;
    for(it = flashVirtualFiles.begin();
        it != flashVirtualFiles.end(); ++it)
    {
        if(it->second.files.size() > 1)
        {
            VirtualFileInformation consolidatedInfo;
            std::string            virtualDBNames, suffixes("{");

            int vdb_access, vdb_types;
            long vdb_sizes;
            bool vdb_info_set = false;

            for(int s = 0; s < it->second.files.size(); ++s)
            {
                VirtualFileInformationMap::iterator ci = 
                    newVirtualFiles.find(it->second.files[s]);
                if(ci != newVirtualFiles.end() &&
                   ci->second.files.size() > 1)
                {
                    int charpos = ci->first.name.find("*")-1;
                    if(charpos >= 0)
                    {
                        std::string suffixChar(ci->first.name.substr(
                                               charpos, 1));
                        if(suffixes == "{")
                            suffixes += suffixChar;
                        else
                        {
                            suffixes += ",";
                            suffixes += suffixChar;
                        }
                    }
                    virtualDBNames += (ci->first.name + " ");
    
                    consolidatedInfo.path = ci->second.path;
                    for(int index = 0; index < ci->second.files.size(); ++index)
                        consolidatedInfo.files.push_back(ci->second.files[index]);
    
                    // Get the file access information.
                    stringVector::iterator vdb_names_it = files.names.begin();
                    intVector::iterator vdb_access_it   = files.access.begin();
                    intVector::iterator vdb_types_it    = files.types.begin();
                    longVector::iterator vdb_sizes_it   = files.sizes.begin();
                    for(int fileIndex = 0; fileIndex < files.names.size(); ++fileIndex)
                    {
                        if(files.names[fileIndex] == ci->first.name)
                        {
                            if(vdb_info_set)
                            {
                                vdb_access = files.access[fileIndex];
                                vdb_sizes = files.sizes[fileIndex];
                                vdb_types = files.types[fileIndex];
                                vdb_info_set = true;
                            }

                            // We found the file that we're looking for so we
                            // should remove it from the list of files since we're
                            // going to consolidate it anyway.
                            files.names.erase(vdb_names_it);
                            files.access.erase(vdb_access_it);
                            files.types.erase(vdb_types_it);
                            files.sizes.erase(vdb_sizes_it);
                            break;
                        }
                        else
                        {
                            ++vdb_names_it;
                            ++vdb_access_it;
                            ++vdb_types_it;
                            ++vdb_sizes_it;
                        }
                    }

                    newVirtualFiles.erase(ci);
                }
            }

            if(consolidatedInfo.files.size() > 0)
            {
                // Erase the digit length from the name of the new database.
                std::string key(it->first.name);
                int starpos = key.find("*");
                int digitPos = starpos - 2;
                suffixes += "}";
                key.replace(digitPos, 2, suffixes);

                consolidatedDBs = true;
                newVirtualFiles[key] = consolidatedInfo;

                // Add the filename to the list of filtered files.
                files.names.push_back(key);
                files.types.push_back(vdb_types);
                files.sizes.push_back(vdb_sizes);
                files.access.push_back(vdb_access);

                debug4 << "Consolidated virtual databases ("
                       << virtualDBNames.c_str() << ") into "
                       << key.c_str() << endl;
            }
        }
    }

    // If we consolidated any databases then we should resort the list of files
    // since we appended the new virtual database to the end of the file list.
    if(consolidatedDBs)
        files.Sort();
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
//   Brad Whitlock, Fri Feb 4 15:14:15 PST 2005
//   Changed the call to GetFilteredFileList so it uses the last filter
//   and file grouping settings that were in place the last time we read
//   the directory.
//
//   Brad Whitlock, Tue Jan 17 16:35:40 PST 2006
//   I added another case that allows us to update a virtual database that
//   we've seen before.
//
// ****************************************************************************

const MDServerConnection::VirtualFileInformationMap::iterator
MDServerConnection::GetVirtualFileDefinition(const std::string &file)
{
    const char *mName = "MDServerConnection::GetVirtualFileDefinition: ";

    // Look for the file in the virtual files map.
    VirtualFileInformationMap::iterator virtualFile = virtualFiles.find(file);

    bool fileContainsStar = (file.find("*") != std::string::npos);
    std::string::size_type index = file.rfind(SLASH_STRING);

    if (virtualFile == virtualFiles.end())
    {
        //
        // If the file was not found in the virtual file map, but it contains
        // a wildcard character, which means that it is a virtual file, read
        // the file list to try and create a definition for the virtual file
        // so we can open it even if we've never seen it before.
        //
        if(fileContainsStar && index != std::string::npos)
        {
            debug2 << mName << "A virtual database (" << file.c_str()
                   << ") was requested but we've never seen "
                      "it before. Try and get a definition for it." << endl;

            // Remember the old path.
            std::string oldPath(currentWorkingDirectory);

            // We found the last separator so we can change to that path
            // to ensure that the file list is read.
            std::string path(file.substr(0, index));
            ChangeDirectory(path);

            // Get the filtered file list. This creates virtual files.
            GetFileListRPC::FileList f;
            GetFilteredFileList(f);

            // Change the path back to the old path.
            ChangeDirectory(oldPath);

            // Look for the file again in the virtual file map.
            virtualFile = virtualFiles.find(file);
        }
    }
    else if(virtualFile->second.files.size() == 0 &&
            fileContainsStar && index != std::string::npos)
    {
        debug2 << mName << "A virtual database (" << file.c_str()
               << ") was requested and we've seen "
                  "it before but it was closed and its definition "
                  "(list of files) was erased. Try and re-read its "
                  "definition using the filter that we've saved for it (";
        for(int i = 0; i < virtualFile->second.filterList.size(); ++i)
        {
            debug2 << virtualFile->second.filterList[i].c_str();
            if(i < virtualFile->second.filterList.size()-1)
                debug2 << ", ";
        }
        debug2 << ")." << endl;

        // Remember the old path.
        std::string oldPath(currentWorkingDirectory);

        // We found the last separator so we can change to that path
        // to ensure that the file list is read.
        std::string path(file.substr(0, index));
        ChangeDirectory(path);

        // Get the filtered file list. This creates virtual files.
        GetFileListRPC::FileList f;
        stringVector oldFilterList(filterList);
        filterList = virtualFile->second.filterList;
        GetFilteredFileList(f);
        filterList = oldFilterList;

        // Change the path back to the old path.
        ChangeDirectory(oldPath);

        // Look for the file again in the virtual file map.
        virtualFile = virtualFiles.find(file);
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
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg forceReadAllCyclesAndTimes
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//    Also, make sure we successfully opened the file the last time
//    before we skip the part where we re-try to open it again.
//
//    Hank Childs, Thu Jan 11 16:14:25 PST 2007
//    Capture the list of plugins attempted in opening the file.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
// ****************************************************************************

avtDatabase *
MDServerConnection::GetDatabase(string file, int timeState,
                                bool forceReadAllCyclesAndTimes,
                                std::vector<std::string> &plugins,
                                string forcedFileType,
                                bool treatAllDBsAsTimeVarying)
{
    //
    // Make sure that the plugins are loaded.
    //
    LoadPlugins();

    //
    // Expand the filename so it has the whole path.
    //
    file = ExpandPath(file);

    if (currentDatabase == NULL ||
        file != currentDatabaseName ||
        treatAllDBsAsTimeVarying ||
        (timeState != currentDatabaseTimeState && currentDatabaseHasInvariantMD))
    {
        string timerMessage(string("Time to open ") + file);
        int    timeid = visitTimer->StartTimer();
        debug2 << "MDServerConnection::GetDatabase: Need to get a new database"
               << ". file=" << file.c_str()
               << ", timeState=" << timeState
               << ", forceReadAllCyclesAndTimes=" << forceReadAllCyclesAndTimes
               << ", forcedFileType=" << forcedFileType
               << ", treatAllDBsAsTimeVarying = " << treatAllDBsAsTimeVarying
               << endl;

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
                    fileNames.size(), timeState, plugins,
                    forcedFileType=="" ? NULL : forcedFileType.c_str(),
                    forceReadAllCyclesAndTimes,
                    treatAllDBsAsTimeVarying);

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
            currentDatabase =
                avtDatabaseFactory::VisitFile(fn, timeState, plugins,
                                              forcedFileType=="" ? NULL : forcedFileType.c_str(),
                                              forceReadAllCyclesAndTimes,
                                              treatAllDBsAsTimeVarying);
        }
        else
        {
            currentDatabase =
                avtDatabaseFactory::FileList(&fn, 1, timeState, plugins,
                                             forcedFileType=="" ? NULL : forcedFileType.c_str(),
                                             forceReadAllCyclesAndTimes,
                                             treatAllDBsAsTimeVarying);
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
//   Brad Whitlock, Fri Feb 4 08:34:50 PDT 2005
//   I added a db argument so this rpc can be used to remove a virtual file
//   definition without necessarily having to close the database.
//
//   Brad Whitlock, Tue Jan 17 16:37:18 PST 2006
//   Changed the method so virtual databases are "deleted" from the map by
//   deleting their list of files. This way, other information such as the
//   filter is preserved so we can reliably recreate the list of files later,
//   if needed.
//
// ****************************************************************************

void
MDServerConnection::CloseDatabase(const std::string &db)
{
    const char *mName = "MDServerConnection::CloseDatabase: ";
    std::string dbToClose(db);
    bool closeCurrentDB = (dbToClose == currentDatabaseName);
    if(dbToClose == "")
    {
        dbToClose = currentDatabaseName;
        closeCurrentDB = true;
    }

    VirtualFileInformationMap::iterator virtualFile = virtualFiles.find(dbToClose);

    if(virtualFile != virtualFiles.end())
    {
        debug1 << mName << "Clearing out " << dbToClose.c_str()
               << "'s file list from the virtual file map so we can "
                  "repopulate it later." << endl;
        virtualFile->second.files.clear();
    }

    if (closeCurrentDB && currentDatabase != NULL)
    {
        debug1 << mName << "Closing database: " << currentDatabaseName.c_str() << endl;
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
//   Brad Whitlock, Wed Apr 27 11:11:36 PDT 2005
//   Added digitLength member.
//
// ****************************************************************************

MDServerConnection::VirtualFileInformation::VirtualFileInformation() : path(),
    files()
{
    digitLength = 0;
}

MDServerConnection::VirtualFileInformation::VirtualFileInformation(
   const MDServerConnection::VirtualFileInformation &obj) : path(obj.path),
   files(obj.files)
{
    digitLength = obj.digitLength;
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
    digitLength = obj.digitLength;
}
