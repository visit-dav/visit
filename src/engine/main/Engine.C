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

#include <Engine.h>
#include <Executors.h>

#include <cerrno>
#include <cstdlib>
#include <cctype>
#include <climits>
#include <signal.h>
#if !defined(_WIN32)
#include <sys/wait.h>
#include <strings.h>
#include <sys/types.h>   // for getpid()
#include <unistd.h>      // for alarm()
#else
#include <process.h>     // for _getpid()
#include <winsock2.h>     // for gethostname()
#endif
#include <new>

#include <visitstream.h>
#include <visit-config.h>
#include <snprintf.h>

#include <BufferConnection.h>
#include <CouldNotConnectException.h>
#include <DefineVirtualDatabaseRPC.h>
#include <ExpressionList.h>
#include <ExprParser.h>
#ifdef PARALLEL
#   include <cognomen.h>
#   ifdef HAVE_ICET
#      include <IceTNetworkManager.h>
#   endif
#endif
#include <IncompatibleVersionException.h>
#include <VisItInit.h>
#include <InstallationFunctions.h>
#include <InitVTK.h>
#include <LoadBalancer.h>
#include <LostConnectionException.h>
#include <Netnodes.h>
#include <ParentProcess.h>
#include <ParsingExprList.h>
#include <QueryAttributes.h>
#include <RemoteProcess.h>
#include <SILAttributes.h>
#include <SimulationCommand.h>
#include <SocketConnection.h>
#include <StringHelpers.h>
#include <StackTimer.h>
#include <VisItDisplay.h>
#include <vtkDebugStream.h>

#include <avtDatabaseMetaData.h>
#include <avtDataObjectReader.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtDebugDumpOptions.h>
#include <avtDataset.h>
#include <avtExprNodeFactory.h>
#include <avtFilter.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <avtTypes.h>
#include <vtkDataSetWriter.h>
#include <avtDataObjectToDatasetFilter.h>
#include <avtVariableCache.h>

#include <string>
using std::string;

// We do this so that the strings command on the .o file
// can tell us whether or not DEBUG_MEMORY_LEAKS was turned on
#ifdef DEBUG_MEMORY_LEAKS
const char *dummy_string1 = "DEBUG_MEMORY_LEAKS";
#endif

#ifdef PARALLEL
#include <parallel.h>
#else
#include <Xfer.h>
#endif

// Static data
Engine *Engine::instance = NULL;

// Static methods
static void WriteByteStreamToSocket(NonBlockingRPC *, Connection *,
                                    avtDataObjectString &);
static void ResetEngineTimeout(void *p, int secs);

// message tag for interrupt messages used in static abort callback function
#ifdef PARALLEL
const int INTERRUPT_MESSAGE_TAG = GetUniqueStaticMessageTag();
static void SumWithINT_MAX_Func(void *ibuf, void *iobuf, int *, MPI_Datatype *);
static MPI_Op SumWithINT_MAX_Op = MPI_OP_NULL;
#endif

// ****************************************************************************
// Class: ViewerRemoteProcess
//
// Purpose:
//   This class is used when the engine needs to reverse launch the viewer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr  9 11:51:35 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class ViewerRemoteProcess : public RemoteProcess
{
public:
    ViewerRemoteProcess(const std::string &p) : RemoteProcess(p)
    {
    }

    virtual ~ViewerRemoteProcess()
    {
    }

protected:
    virtual void Launch(const std::string &rHost, bool createAsThoughLocal,
                        const stringVector &commandLine)
    {
        const char *mName = "ViewerRemoteProcess::Launch: ";

        // Convert the remote process arguments into arguments that the viewer
        // will recognize as directives to reverse connect to the engine.
        stringVector viewerArgs;
        std::map<std::string, std::string> launchArgs;
        for(size_t i = 0; i < commandLine.size(); ++i)
        {
            if(commandLine[i] == "-host" ||
               commandLine[i] == "-port" || 
               commandLine[i] == "-key")
            {
                launchArgs[commandLine[i]] = commandLine[i+1];
                ++i;
            }
            else
                viewerArgs.push_back(commandLine[i]);
        }

        // Convert to -host=val,-port=val,-key=val
        std::string arg;
        std::map<std::string, std::string>::const_iterator it;
        for(it = launchArgs.begin(); it != launchArgs.end(); ++it)
        {
            arg += it->first;
            if(it->second.size() > 0)
            {
                arg += "=";
                arg += it->second;
            }
            arg += ",";
        }
        arg = arg.substr(0, arg.size()-1);
        viewerArgs.push_back("-connectengine");
        viewerArgs.push_back(arg);

        debug5 << mName << "viewer args(";
        for(size_t i = 0; i < viewerArgs.size(); ++i)
            debug5 << viewerArgs[i] << ", ";
        debug5 << ")" << endl;

        RemoteProcess::Launch(rHost, createAsThoughLocal, viewerArgs);
    }
};

// ****************************************************************************
//  Constructor:  Engine::Engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue Mar  8 17:59:40 PST 2005
//    Added procAtts
//
//    Hank Childs, Mon Mar 21 11:24:06 PST 2005
//    Initialize all of the data members.
//
//    Jeremy Meredith, Mon Apr  4 15:58:23 PDT 2005
//    Added simulationCommandCallback.
//
//    Mark C. Miller Thu Apr 21 09:37:41 PDT 2005
//    Set simulationCommandRPC to NULL
//
//    Hank Childs, Thu Jan  5 14:24:06 PST 2006
//    Initialize shouldDoDashDump.
//
//    Brad Whitlock, Thu Jan 25 13:56:24 PST 2007
//    Added commandFromSim.
//
//    Sean Ahern, Wed Dec 12 12:01:13 EST 2007
//    Added a distinction between the execution timeout and the idle timeout.
//
//    Sean Ahern, Wed Dec 12 16:24:46 EST 2007
//    Moved the execution timeout back to 30 minutes.
//
//    Jeremy Meredith, Wed Jan 23 16:50:36 EST 2008
//    Added setEFileOpenOptionsRPC.
//
//    Cyrus Harrison, Tue Feb 19 08:42:51 PST 2008
//    Removed shouldDoDashDump (flag now contained in avtDebugDumpOptions)
//
//    Brad Whitlock, Tue Jun 24 16:07:10 PDT 2008
//    Added pluginDir.
//
//    Tom Fogal, Fri Jul 11 13:53:23 EDT 2008
//    Default IceT to false.
//
//    Tom Fogal, Mon Aug 11 11:40:16 EDT 2008
//    Initialize the number of displays.
//
//    Tom Fogal, Mon Sep  1 12:48:36 EDT 2008
//    Initialize the display for rendering.
//
//    Brad Whitlock, Fri Mar 27 11:33:52 PDT 2009
//    I initialized simulationCommandCallbackData.
//
//    Brad Whitlock, Thu Apr  9 11:57:44 PDT 2009
//    Initialize viewer, viewerP, reverseLaunch.
//
//    Brad Whitlock, Thu Apr 23 12:05:16 PDT 2009
//    Disable simulation plugins by default.
//
// ****************************************************************************

Engine::Engine() : viewerArgs()
{
    viewer = NULL;
    viewerP = NULL;
    reverseLaunch = false;

    vtkConnection = 0;
    noFatalExceptions = true;
    idleTimeoutMins = 480;
    executionTimeoutMins = 30;
    idleTimeoutEnabled = false;
    overrideTimeoutMins = 0;
    overrideTimeoutEnabled = false;
    netmgr = NULL;
    lb = NULL;
    procAtts = NULL;
    simulationCommandCallback = NULL;
    simulationCommandCallbackData = NULL;
    metaData = NULL;
    silAtts = NULL;
    commandFromSim = NULL;
    pluginDir = "";
    simulationPluginsEnabled = false;

    quitRPC = NULL;
    keepAliveRPC = NULL;
    readRPC = NULL;
    applyOperatorRPC = NULL;
    makePlotRPC = NULL;
    useNetworkRPC = NULL;
    updatePlotAttsRPC = NULL;
    pickRPC = NULL;
    startPickRPC = NULL;
    startQueryRPC = NULL;
    executeRPC = NULL;
    clearCacheRPC = NULL;
    queryRPC = NULL;
    releaseDataRPC = NULL;
    openDatabaseRPC = NULL;
    defineVirtualDatabaseRPC = NULL;
    renderRPC = NULL;
    setWinAnnotAttsRPC = NULL;
    cloneNetworkRPC = NULL;
    procInfoRPC = NULL;
    simulationCommandRPC = NULL;
    setEFileOpenOptionsRPC = NULL;

    useIceT = false;
    nDisplays = 0;
    renderingDisplay = NULL;
}

// ****************************************************************************
//  Destructor:  Engine::~Engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar 21 11:21:42 PST 2005
//    Delete RPCs.
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    deleted simulationCommandRPC
//
//    Brad Whitlock, Thu Jan 25 13:56:24 PST 2007
//    Added commandFromSim.
//
//    Jeremy Meredith, Wed Jan 23 16:50:36 EST 2008
//    Added setEFileOpenOptionsRPC.
//
//    Tom Fogal, Mon Sep  1 13:04:51 EDT 2008
//    Add renderingDisplay.
//
//    Brad Whitlock, Mon Dec  1 10:22:05 PST 2008
//    Delete network manager last to delay when plugins are unloaded.
//
//    Brad Whitlock, Thu Apr  9 11:57:07 PDT 2009
//    Delete viewer and viewerP.
//
// ****************************************************************************

Engine::~Engine()
{
    delete xfer;
    delete lb;
    delete silAtts;
    delete metaData;
    delete commandFromSim;

    for (size_t i=0; i<rpcExecutors.size(); i++)
        delete rpcExecutors[i];

    delete quitRPC;
    delete keepAliveRPC;
    delete readRPC;
    delete applyOperatorRPC;
    delete makePlotRPC;
    delete useNetworkRPC;
    delete updatePlotAttsRPC;
    delete pickRPC;
    delete startPickRPC;
    delete startQueryRPC;
    delete executeRPC;
    delete clearCacheRPC;
    delete queryRPC;
    delete releaseDataRPC;
    delete openDatabaseRPC;
    delete defineVirtualDatabaseRPC;
    delete renderRPC;
    delete setWinAnnotAttsRPC;
    delete cloneNetworkRPC;
    delete procInfoRPC;
    delete simulationCommandRPC;
    delete setEFileOpenOptionsRPC;

    delete viewer;
    delete viewerP;

    delete renderingDisplay;

    // Delete the network manager last since it deletes plugin managers
    // and our RPC's may need to call plugin AttributeSubject destructors.
    // We can't seem to do that reliably on Linux once plugins have been
    // unloaded.
    delete netmgr;
}

// ****************************************************************************
//  Method:  Engine::Instance
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
//  Modfications:
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added timer
// ****************************************************************************
Engine *Engine::Instance()
{
    int instanceTimer = visitTimer->StartTimer();
    if (!instance)
        instance = new Engine;
    visitTimer->StopTimer(instanceTimer, "Instancing the engine");
    return instance;
}

// ****************************************************************************
//  Method:  Engine::Initialize
//
//  Purpose:
//    Do all the initialization needed first.
//
//  Arguments:
//    argc
//    argv
//
//  Note: Broken off from old main().  See main.C for comment history.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
//  Modifications:
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added code to override the new handler for the engine
//
//    Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//    Wrapped call to set_new_handler with WIN32 conditional compilation
//
//    Eric Brugger, Tue Aug 31 10:45:57 PDT 2004
//    Added a call to PAR_CreateTypes since it is no longer called from
//    PAR_Init.
//
//    Jeremy Meredith, Mon Nov  1 13:26:23 PST 2004
//    Use a buffer connection that is visible at class scope for parallel
//    communication.  Before, it was an automatic variable in PAR_EventLoop
//    but I needed a separate PAR_ProcessInput method that used the same
//    buffer connection.
//
//    Jeremy Meredith, Tue May 17 11:20:51 PDT 2005
//    Allow disabling of signal handlers.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added timer
//
//    Cyrus Harrison, Wed Jan 23 09:21:18 PST 2008
//    Changed set_new_handler to std::set_new_handler b/c of change from 
//    deprecated <new.h> to <new>
//  
//    Cyrus Harrison, Thu Jan 31 14:48:18 PST 2008
//    Removed a lingering cerr message showing the MPI rank of the engine 
//    process.
//  
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
//
//    Tom Fogal, Tue Jul 15 10:02:27 EDT 2008
//    Include the # of processors we're running on in the timing message.
//
//    Brad Whitlock, Thu Apr  9 13:40:32 PDT 2009
//    I moved the xfer set input connection code for non-UI procs to 
//    ConnectToViewer where the UI proc sets its xfer input connection.
//
// ****************************************************************************

void
Engine::Initialize(int *argc, char **argv[], bool sigs)
{
    int initTimer = visitTimer->StartTimer();

    // Get arguments that the viewer can use.
    ExtractViewerArguments(argc, argv);

#ifdef PARALLEL
    xfer = new MPIXfer;
    //
    // Initialize for MPI and get the process rank & size.
    //
    PAR_Init(*argc, *argv);

    //
    // Create the derived types and operators for sending messages
    // and collective operations.
    //
    PAR_CreateTypes();

    //
    // Create MPI operator used for reducing cell counts in presence
    // if INT_MAX values.
    //
    MPI_Op_create(SumWithINT_MAX_Func, 1, &SumWithINT_MAX_Op);

    //
    // Initialize error logging
    //
    VisItInit::SetComponentName("engine");
    VisItInit::Initialize(*argc, *argv, PAR_Rank(), PAR_Size(), true, sigs);
#else
    xfer = new Xfer;
    VisItInit::SetComponentName("engine");
    VisItInit::Initialize(*argc, *argv, 0,1, true, sigs);
#endif

    //
    // Set a different new handler for the engine
    //
#if !defined(_WIN32)
    std::set_new_handler(Engine::NewHandler);
#endif

    debug1 << "ENGINE started\n";
#ifdef PARALLEL
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Initializing a %d processor engine "
                 "(including MPI_Init())", PAR_Size());
        visitTimer->StopTimer(initTimer, msg);
    }
#else
    visitTimer->StopTimer(initTimer, "Initializing the engine");
#endif
}

// ****************************************************************************
//  Method:  Engine::Finalize
//
//  Purpose:
//      Do the finalization for the engine.
//
//  Programmer:  Hank Childs
//  Creation:    June 1, 2004
//
//  Modifications:
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added timer
//
//    Mark C. Miller, Thu Aug  3 13:33:20 PDT 2006
//    Eliminated out call to StopTimer. That call cannot be made after
//    Finalize has been called. However, TimingsManager can still log the
//    timer as "unknown" when writing timings to files
//
//    Tom Fogal, Fri Jul 18 15:09:51 EDT 2008
//    Remove the variable which captures the return value of StartTimer.  It
//    can't be used, and the compiler complains about it.
//
//    Tom Fogal, Tue Jul 29 10:56:51 EDT 2008
//    Kill the X server if one was started.
//
//    Tom Fogal, Mon Sep  1 13:03:09 EDT 2008
//    Remove the X server stuff, but explicitly delete the rendering display.
//    It must be done, otherwise in the X case we'll leave stale displays
//    around.
//
//    Hank Childs, Tue Dec  2 10:04:37 PST 2008
//    Remove unmatched StartTimer call.  (The resulting timing can't be 
//    dumped anyways.)
//
// ****************************************************************************

void
Engine::Finalize(void)
{
    delete this->renderingDisplay;
    // Now null it out; in case the destructor actually *does* get called.
    this->renderingDisplay = NULL;

#ifdef PARALLEL
    MPI_Op_free(&SumWithINT_MAX_Op);
#endif

    VisItInit::Finalize();
}


// ****************************************************************************
// Method: Engine::EnableSimulationPlugins
//
// Purpose: 
//   Allow the engine to use simulation plugins in addition to engine plugins.
//
// Note:       This method needs to be called before SetUpViewerInterface.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 23 12:08:27 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
Engine::EnableSimulationPlugins()
{
    simulationPluginsEnabled = true;
}

#ifdef PARALLEL
#include <PluginBroadcaster.h>

// ****************************************************************************
// Class: PAR_PluginBroadcaster
//
// Purpose: 
//   This object helps the plugin managers broadcast information about libI
//   plugins to other processors. This can save 1000's of processors reading
//   ~200 libI files at the same time.
//
// Arguments:
//   ids      : The plugin ids.
//   names    : The plugin names.
//   versions : The plugin versions.
//   libfiles : The names of the plugin files.
//   enabled  : Whether the plugins are enabled
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 17 13:16:20 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class PAR_PluginBroadcaster : public PluginBroadcaster
{
public:
    PAR_PluginBroadcaster() : PluginBroadcaster()
    {
    }

    virtual ~PAR_PluginBroadcaster()
    {
    }

    virtual void BroadcastStringVector(stringVector &v)
    {
        ::BroadcastStringVector(v, PAR_Rank());
    }

    virtual void BroadcastBoolVector(boolVector &v)
    {
        ::BroadcastBoolVector(v, PAR_Rank());
    }

    virtual void BroadcastStringVectorVector(std::vector<std::vector<std::string> > &v)
    {
        ::BroadcastStringVectorVector(v, PAR_Rank());
    }
};
#endif

// ****************************************************************************
//  Method:  Engine::SetUpViewerInterface
//
//  Purpose:
//    Do all the initialization needed after we connect to the viewer.
//
//  Arguments:
//    argc
//    argv
//
//  Note: Broken off from old main().  See main.C for comment history.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
//  Modifications:
//    Sean Ahern, Fri Nov 22 16:09:26 PST 2002
//    Removed ApplyNamedFunction.
//
//    Jeremy Meredith, Mon Sep 15 17:14:07 PDT 2003
//    Removed SetFinalVariableName.
//
//    Hank Childs, Fri Mar  5 11:46:09 PST 2004
//    Made the database plugins be loaded on demand.
//
//    Brad Whitlock, Fri Mar 12 11:20:58 PDT 2004
//    I added keepAliveRPC.
//
//    Kathleen Bonnell, Wed Mar 31 16:53:03 PST 2004 
//    Added cloneNetworkRPC.
//
//    Jeremy Meredith, Wed Aug 25 10:09:14 PDT 2004
//    Added ability to send metadata and SIL atts back to the viewer.
//
//    Jeremy Meredith, Wed Nov 24 11:42:40 PST 2004
//    Renamed EngineExprNode to avtExprNode.
//
//    Hank Childs, Mon Feb 28 17:03:06 PST 2005
//    Added StartQueryRPC.
//
//    Mark C. Miller, Tue Mar  8 17:59:40 PST 2005
//    Added procInfoRPC 
//
//    Jeremy Meredith, Mon Apr  4 15:58:48 PDT 2005
//    Added simulationCommandRPC.
//
//    Hank Childs, Sat Dec  3 20:32:37 PST 2005
//    Add support for hardware acceleration.
//
//    Hank Childs, Thu Jan  5 14:24:06 PST 2006
//    Fix issue with -dump since code that instantiates network manager has
//    moved.
//
//    Hank Childs, Mon Feb 13 22:25:04 PST 2006
//    Added constructDDFRPC.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added timer
//
//    Hank Childs, Tue Sep  5 10:45:13 PDT 2006
//    Add a callback to avtCallback to reset the timer.  This is for
//    functionality that takes a long time: queries over time and line scan
//    queries.
//
//    Brad Whitlock, Thu Jan 25 14:02:10 PST 2007
//    Added commandFromSim.
//
//    Dave Pugmire, Mon Dec 10 15:57:32 EST 2007
//    Moved the plugin init and load after ProcessCommandLine call.
//    ProcessCommandLine now takes an optional plugindir argument which
//    must be set before the plugins are activated.
//
//    Jeremy Meredith, Wed Jan 23 16:50:36 EST 2008
//    Added setEFileOpenOptionsRPC.
//
//    Cyrus Harrison, Tue Feb 19 08:42:51 PST 2008
//    Removed shouldDoDashDump (flag now contained in avtDebugDumpOptions)
//
//    Brad Whitlock, Tue Jun 24 15:18:44 PDT 2008
//    Changed how plugin managers are called.
//
//    Tom Fogal, Sun Jul  6 16:55:05 EDT 2008
//    Use the IceT manager if enabled at compile time.
//
//    Tom Fogal, Thu Jul 10 10:09:50 EDT 2008
//    Use a static variable for putenv's argument.  According to the
//    documentation, the argument is used directly -- so if an automatic
//    variable is used in putenv, Bad Things can happen if the method returns
//    and we try to use that memory address again.
//
//    Tom Fogal, Fri Jul 11 12:01:34 EDT 2008
//    Use the IceT flag to figure out which NetworkManager to instantiate.
//
//    Tom Fogal, Sun Jul 27 17:44:32 EDT 2008
//    Use a new method, SetupDisplay, for initializing the connection to the X
//    server.
//
//    Tom Fogal, Mon Aug 25 10:04:30 EDT 2008
//    Make sure the number of GPUs ends up in the timing output.
//
//    Tom Fogal, Mon Sep  1 14:19:45 EDT 2008
//    Removed an assert.
//
//    Hank Childs, Thu Jan 29 11:15:16 PST 2009
//    Add NamedSelectionRPC.
//
//    Brad Whitlock, Thu Apr 23 12:11:35 PDT 2009
//    Differentiate between simulation and engine plugins.
//
//    Brad Whitlock, Wed Jun 17 13:23:58 PDT 2009
//    I passed an instance of PAR_PluginBroadcaster to the plugin managers 
//    so they could skip reading libI plugins for most processors in parallel.
//    Rank 0 reads the libI plugins and then shares their information with 
//    other processors over MPI. I also added timing information for loading
//    plugins in parallel.
//
// ****************************************************************************

void
Engine::SetUpViewerInterface(int *argc, char **argv[])
{
    int setupTimer = visitTimer->StartTimer();

    if(reverseLaunch)
        vtkConnection = viewer->GetWriteConnection(1);
    else
        vtkConnection = viewerP->GetReadConnection(1);

    // Parse the command line.
    ProcessCommandLine(*argc, *argv);

    InitVTK::Initialize();
    if (avtCallback::GetSoftwareRendering())
    {
        InitVTK::ForceMesa();
    }
    else
    {
        std::ostringstream s;
        s << "Setting up " << this->nDisplays << " GPUs for HW rendering";
        debug3 << "Setting up X displays for " << this->nDisplays << " GPUs."
               << "  Using X arguments: '" << this->X_Args << "'" << std::endl;
        TimedCodeBlock(s.str(), this->SetupDisplay());
    }
    avtCallback::SetNowinMode(true);

    //
    // Create the network manager.  Note that this must be done *after* the
    // code to set the display and decide if we are using Mesa.
    //
#if defined(PARALLEL) && defined(HAVE_ICET)
    if(this->useIceT)
    {
        debug2 << "Using IceT network manager." << std::endl;
        netmgr = new IceTNetworkManager;
    }
    else
    {
        debug2 << "Using standard network manager." << std::endl;
        netmgr = new NetworkManager;
    }
#else
    if(this->useIceT)
    {
        debug1 << "Error; IceT not enabled at compile time. "
               << "Ignoring ..." << std::endl;
    }
    netmgr = new NetworkManager;
#endif

    //
    // Initialize the plugin managers.
    //
    int pluginsTotal = visitTimer->StartTimer();
    if(pluginDir.size() > 0)
    {
        netmgr->GetPlotPluginManager()->SetPluginDir(pluginDir.c_str());
        netmgr->GetOperatorPluginManager()->SetPluginDir(pluginDir.c_str());
        netmgr->GetDatabasePluginManager()->SetPluginDir(pluginDir.c_str());
    }
    PluginManager::PluginCategory pCat = simulationPluginsEnabled ? 
        PluginManager::Simulation : PluginManager::Engine;
#ifdef PARALLEL
    bool readInfo = PAR_UIProcess();
    PAR_PluginBroadcaster broadcaster;
    int pluginInit = visitTimer->StartTimer();
    netmgr->GetPlotPluginManager()->Initialize(pCat, true, NULL, 
                                               readInfo, 
                                               &broadcaster);
    visitTimer->StopTimer(pluginInit, "Loading plot plugin info");

    pluginInit = visitTimer->StartTimer();
    netmgr->GetOperatorPluginManager()->Initialize(pCat, true, NULL, 
                                               readInfo, 
                                               &broadcaster);
    visitTimer->StopTimer(pluginInit, "Loading operator plugin info");

    pluginInit = visitTimer->StartTimer();
    netmgr->GetDatabasePluginManager()->Initialize(pCat, true, NULL, 
                                               readInfo, 
                                               &broadcaster);
    visitTimer->StopTimer(pluginInit, "Loading database plugin info");
#else
    netmgr->GetPlotPluginManager()->Initialize(pCat, false);
    netmgr->GetOperatorPluginManager()->Initialize(pCat, false);
    netmgr->GetDatabasePluginManager()->Initialize(pCat, false);
#endif    
    //
    // Load plugins
    //
    netmgr->GetPlotPluginManager()->LoadPluginsOnDemand();
    netmgr->GetOperatorPluginManager()->LoadPluginsOnDemand();
    netmgr->GetDatabasePluginManager()->LoadPluginsOnDemand();
    visitTimer->StopTimer(pluginsTotal, "Setting up plugins.");

#if !defined(_WIN32)
    // Set up the alarm signal handler.
    signal(SIGALRM, Engine::AlarmHandler);
#endif

    avtCallback::RegisterResetTimeoutCallback(ResetEngineTimeout, this);

    // Create some RPC objects and make Xfer observe them.
    quitRPC                         = new QuitRPC;
    keepAliveRPC                    = new KeepAliveRPC;
    readRPC                         = new ReadRPC;
    applyOperatorRPC                = new ApplyOperatorRPC;
    makePlotRPC                     = new MakePlotRPC;
    useNetworkRPC                   = new UseNetworkRPC;
    updatePlotAttsRPC               = new UpdatePlotAttsRPC;
    pickRPC                         = new PickRPC;
    startPickRPC                    = new StartPickRPC;
    startQueryRPC                   = new StartQueryRPC;
    executeRPC                      = new ExecuteRPC;
    clearCacheRPC                   = new ClearCacheRPC;
    queryRPC                        = new QueryRPC;
    releaseDataRPC                  = new ReleaseDataRPC;
    openDatabaseRPC                 = new OpenDatabaseRPC;
    defineVirtualDatabaseRPC        = new DefineVirtualDatabaseRPC;
    renderRPC                       = new RenderRPC;
    setWinAnnotAttsRPC              = new SetWinAnnotAttsRPC;
    cloneNetworkRPC                 = new CloneNetworkRPC;
    procInfoRPC                     = new ProcInfoRPC;
    simulationCommandRPC            = new SimulationCommandRPC;
    exportDatabaseRPC               = new ExportDatabaseRPC;
    constructDDFRPC                 = new ConstructDDFRPC;
    namedSelectionRPC               = new NamedSelectionRPC;
    setEFileOpenOptionsRPC          = new SetEFileOpenOptionsRPC;

    xfer->Add(quitRPC);
    xfer->Add(keepAliveRPC);
    xfer->Add(readRPC);
    xfer->Add(applyOperatorRPC);
    xfer->Add(makePlotRPC);
    xfer->Add(useNetworkRPC);
    xfer->Add(updatePlotAttsRPC);
    xfer->Add(pickRPC);
    xfer->Add(startPickRPC);
    xfer->Add(startQueryRPC);
    xfer->Add(executeRPC);
    xfer->Add(clearCacheRPC);
    xfer->Add(queryRPC);
    xfer->Add(releaseDataRPC);
    xfer->Add(openDatabaseRPC);
    xfer->Add(defineVirtualDatabaseRPC);
    xfer->Add(renderRPC);
    xfer->Add(setWinAnnotAttsRPC);
    xfer->Add(cloneNetworkRPC);
    xfer->Add(procInfoRPC);
    xfer->Add(simulationCommandRPC);
    xfer->Add(exportDatabaseRPC);
    xfer->Add(constructDDFRPC);
    xfer->Add(namedSelectionRPC);
    xfer->Add(setEFileOpenOptionsRPC);

    // Create an object to implement the RPCs
    rpcExecutors.push_back(new RPCExecutor<QuitRPC>(quitRPC));
    rpcExecutors.push_back(new RPCExecutor<KeepAliveRPC>(keepAliveRPC));
    rpcExecutors.push_back(new RPCExecutor<ReadRPC>(readRPC));
    rpcExecutors.push_back(new RPCExecutor<ApplyOperatorRPC>(applyOperatorRPC));
    rpcExecutors.push_back(new RPCExecutor<PrepareOperatorRPC>(&applyOperatorRPC->GetPrepareOperatorRPC()));
    rpcExecutors.push_back(new RPCExecutor<MakePlotRPC>(makePlotRPC));
    rpcExecutors.push_back(new RPCExecutor<PreparePlotRPC>(&makePlotRPC->GetPreparePlotRPC()));
    rpcExecutors.push_back(new RPCExecutor<UseNetworkRPC>(useNetworkRPC));
    rpcExecutors.push_back(new RPCExecutor<UpdatePlotAttsRPC>(updatePlotAttsRPC));
    rpcExecutors.push_back(new RPCExecutor<PrepareUpdatePlotAttsRPC>(&updatePlotAttsRPC->GetPrepareUpdatePlotAttsRPC()));
    rpcExecutors.push_back(new RPCExecutor<PickRPC>(pickRPC));
    rpcExecutors.push_back(new RPCExecutor<StartPickRPC>(startPickRPC));
    rpcExecutors.push_back(new RPCExecutor<StartQueryRPC>(startQueryRPC));
    rpcExecutors.push_back(new RPCExecutor<ExecuteRPC>(executeRPC));
    rpcExecutors.push_back(new RPCExecutor<ClearCacheRPC>(clearCacheRPC));
    rpcExecutors.push_back(new RPCExecutor<QueryRPC>(queryRPC));
    rpcExecutors.push_back(new RPCExecutor<ReleaseDataRPC>(releaseDataRPC));
    rpcExecutors.push_back(new RPCExecutor<OpenDatabaseRPC>(openDatabaseRPC));
    rpcExecutors.push_back(new RPCExecutor<DefineVirtualDatabaseRPC>(defineVirtualDatabaseRPC));
    rpcExecutors.push_back(new RPCExecutor<RenderRPC>(renderRPC));
    rpcExecutors.push_back(new RPCExecutor<SetWinAnnotAttsRPC>(setWinAnnotAttsRPC));
    rpcExecutors.push_back(new RPCExecutor<CloneNetworkRPC>(cloneNetworkRPC));
    rpcExecutors.push_back(new RPCExecutor<ProcInfoRPC>(procInfoRPC));
    rpcExecutors.push_back(new RPCExecutor<SimulationCommandRPC>(simulationCommandRPC));
    rpcExecutors.push_back(new RPCExecutor<ExportDatabaseRPC>(exportDatabaseRPC));
    rpcExecutors.push_back(new RPCExecutor<ConstructDDFRPC>(constructDDFRPC));
    rpcExecutors.push_back(new RPCExecutor<NamedSelectionRPC>(namedSelectionRPC));
    rpcExecutors.push_back(new RPCExecutor<SetEFileOpenOptionsRPC>(setEFileOpenOptionsRPC));

    // Hook up the expression list as an observed object.
    Parser *p = new ExprParser(new avtExprNodeFactory());
    ParsingExprList *l = new ParsingExprList(p);
    xfer->Add(l->GetList());

    // Hook up metadata and SIL to be send back to the viewer.
    // This is intended to only be used for simulations.
    metaData = new avtDatabaseMetaData;
    silAtts = new SILAttributes;
    commandFromSim = new SimulationCommand;
    xfer->Add(metaData);
    xfer->Add(silAtts);
    xfer->Add(commandFromSim);

    //
    // Hook up the viewer connections to Xfer
    //
#ifdef PARALLEL
    if (PAR_UIProcess())
    {
        if(reverseLaunch)
            xfer->SetInputConnection(viewer->GetWriteConnection());
        else
            xfer->SetInputConnection(viewerP->GetWriteConnection());
    }
    else
    {
        // Set the xfer object's input connection to be the buffer connection
        // of the object itself
        xfer->SetInputConnection(&par_conn);
    }
#else
    if(reverseLaunch)
        xfer->SetInputConnection(viewer->GetWriteConnection());
    else
        xfer->SetInputConnection(viewerP->GetWriteConnection());
#endif
    if(reverseLaunch)
        xfer->SetOutputConnection(viewer->GetReadConnection());
    else
        xfer->SetOutputConnection(viewerP->GetReadConnection());

    //
    // Set the global destination format. This only happens on the UI-Process
    // when running in parallel since non-UI processes have no SocketConnections.
    //
    if(reverseLaunch)
    {
        if (viewer->GetReadConnection() != 0)
            destinationFormat = viewer->GetReadConnection()->GetDestinationFormat();
    }
    else
    {
        if (viewerP->GetReadConnection() != 0)
            destinationFormat = viewerP->GetReadConnection()->GetDestinationFormat();
    }

    //
    // Create the network manager and the load balancer.
    //
#ifdef PARALLEL
    lb = new LoadBalancer(PAR_Size(), PAR_Rank());
#else
    lb = new LoadBalancer(1, 0);
#endif
    netmgr->SetLoadBalancer(lb);

    //
    // Initialize some callback functions.
    //
    avtDataObjectSource::RegisterAbortCallback(Engine::EngineAbortCallback, xfer);
    avtDataObjectSource::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                                  NULL);
    LoadBalancer::RegisterAbortCallback(Engine::EngineAbortCallbackParallel, xfer);
    LoadBalancer::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                           NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                                       Engine::EngineInitializeProgressCallback, NULL);

    visitTimer->StopTimer(setupTimer, "Setting up viewer interface");
}

// ****************************************************************************
//  Method:  Engine::GetInputSocket
//
//  Purpose:
//    Return the file descriptor for the input socket.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 12, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Apr  9 11:39:57 PDT 2009
//    I added code to support reverse launches.
//
// ****************************************************************************

int
Engine::GetInputSocket()
{
    int s;
    if(reverseLaunch)
        s = viewer->GetWriteConnection()->GetDescriptor();
    else
        s = viewerP->GetWriteConnection()->GetDescriptor();

    return s;
}

// ****************************************************************************
// Method: Engine::ExtractViewerArguments
//
// Purpose: 
//   This method takes a first peek at the engine's command line and extracts
//   any useful arguments into viewerArgs. If the arguments are meant only
//   for the viewer then they get removed from the command line before the
//   engine gets them. The viewerArgs are used when reverse launching the
//   viewer from the engine.
//
// Arguments:
//   argc : Pointer to the number of command line arguments.
//   argv : Pointer to the command line arguments array.
// 
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr  9 14:09:24 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
Engine::ExtractViewerArguments(int *argc, char **argv[])
{
    char **ARGV = *argv;
    for(int i = 1; i < *argc; ++i)
    {
        int deleteCount = 0;

        if(strcmp(ARGV[i], "-reverse_launch") == 0)
        {
            reverseLaunch = true;
            deleteCount = 1;
        }
        else if(strcmp(ARGV[i], "-nowin") == 0)
        {
            viewerArgs.push_back("-nowin");
            deleteCount = 1;
        }
        else if(strcmp(ARGV[i], "-s") == 0)
        {
            viewerArgs.push_back("-s");
            viewerArgs.push_back(ARGV[i+1]);
            deleteCount = 2;
        }
        else if(strcmp(ARGV[i], "-o") == 0)
        {
            viewerArgs.push_back("-o");
            viewerArgs.push_back(ARGV[i+1]);
            deleteCount = 2;
        }
        else if(strcmp(ARGV[i], "-debug") == 0)
        {
            viewerArgs.push_back(ARGV[i]);
            viewerArgs.push_back(ARGV[i+1]);
            ++i;
        }
        else
            viewerArgs.push_back(ARGV[i]);
        // some other args we want to handle.

        if(deleteCount > 0)
        {
            for(int j = i; j < *argc; ++j)
            {
                ARGV[j] = (j + deleteCount < *argc) ?
                    ARGV[j + deleteCount] : NULL;
            }
            (*argc) -= deleteCount;
            --i;
        }
    }
}

// ****************************************************************************
// Method: Engine::ReverseLaunchViewer
//
// Purpose: 
//   This function scans the command line arguments for -reverse_launch and if
//   it is found then the engine launches the viewer.
//
// Arguments:
//   argc : Pointer to the number of command line arguments.
//   argv : Pointer to the command line arguments array.
//
// Returns:    True if we reverse launched the viewer; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr  9 11:59:57 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
Engine::ReverseLaunchViewer(int *argc, char **argv[])
{
    // If we're reverse launching and we're the UI process then we can 
    // launch the viewer.
    viewer = new ViewerRemoteProcess(GetVisItLauncher());
#ifdef PARALLEL
    if(reverseLaunch && PAR_UIProcess())
#else
    if(reverseLaunch)
#endif
    {
        TRY
        {
            viewer->AddArgument("-viewer");
            for(size_t j = 0; j < viewerArgs.size(); ++j)
                viewer->AddArgument(viewerArgs[j]);
            viewer->Open("localhost",              // host
                         HostProfile::MachineName, // client host determination
                         "",                       // client host name
                         false,                    // manual SSH port
                         0,                        // ssh port
                         false,                    // ssh tunnelling
                         1,                        // num read sockets
                         2);                       // num write sockets
        }
        CATCH(VisItException)
        {
            reverseLaunch = false;
        }
        ENDTRY
    }
    int reverse = reverseLaunch ? 1 : 0;
#ifdef PARALLEL
    // Make sure that all processors have the same value as the UI process.
    MPI_Bcast((void *)&reverse, 1, MPI_INT, 0, VISIT_MPI_COMM);
#endif
    if(reverse == 0 && viewer != NULL)
    {
        delete viewer;
        viewer = NULL;
        reverseLaunch = false;
    }

    return reverseLaunch;
}

// ****************************************************************************
// Function: ConnectViewer
//
// Purpose: 
//   Connects to the viewer.
//
// Arguments:
//   argc : Pointer to the number of command line arguments.
//   argv : Pointer to the command line arguments array.
//
// Returns:    True if we connect to the viewer, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 08:47:46 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Brad Whitlock, Tue Jul 29 11:21:22 PDT 2003
//    I updated the interface to ParentProcess::Connect.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added timer
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Brad Whitlock, Thu Apr  9 12:07:22 PDT 2009
//    I added support for reverse launching the viewer.
//
// ****************************************************************************

bool
Engine::ConnectViewer(int *argc, char **argv[])
{
    int connectTimer = visitTimer->StartTimer();

    // Connect to the viewer.
    TRY
    {
        if(ReverseLaunchViewer(argc, argv))
        {
            // The engine launched the viewer.
            reverseLaunch = true;
        }
        else
        {
            // The viewer launched the engine
            viewerP = new ParentProcess;
#ifdef PARALLEL
            viewerP->Connect(1, 2, argc, argv, PAR_UIProcess());
#else
            viewerP->Connect(1, 2, argc, argv, true);
#endif
        }
    }
    CATCH(IncompatibleVersionException)
    {
        debug1 << "The engine has a different version than its client." << endl;
        noFatalExceptions = false;
    }
    CATCH(CouldNotConnectException)
    {
        noFatalExceptions = false;
    }
    ENDTRY

#ifdef PARALLEL
    //
    // Tell the other processes if they should exit if we had an error when
    // connecting to the viewer.
    //
    int shouldExit = noFatalExceptions ? 0 : 1;
    MPI_Bcast((void *)&shouldExit, 1, MPI_INT, 0, VISIT_MPI_COMM);
    noFatalExceptions = (shouldExit == 0);
#endif

    visitTimer->StopTimer(connectTimer, "Connecting to viewer");
    return noFatalExceptions;
}

#ifdef PARALLEL
// ****************************************************************************
// Function: PAR_EventLoop
//
// Purpose:
//   This is the main event loop for the engine. The master process executes
//   the serial event loop and its xfer object broadcasts the viewer's control
//   data to the xfer objects on the other processors.
//
// Notes:      
//   The MPI_BCast call for the UI process happens in the MPIXfer
//   object's process method. This was done so broadcasts are done
//   only when there are complete messages. This avoids LOTS of
//   extra broadcasts when the socket read tends to happen in small
//   increments.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 15:58:20 PST 2000
//
// Modifications:
//    Jeremy Meredith, Thu Sep 21 22:42:19 PDT 2000
//    Added an extern for the appropriate socket to read from.
//    Note that this is a small hack.   
//
//    Brad Whitlock, Thu Mar 15 14:32:33 PST 2001
//    Rewrote it so the master process uses the new & improved serial
//    event loop. It is also set up to tell the other processes to die if
//    we lost the connection to the viewer.
//
//    Sean Ahern, Thu Feb 21 16:18:03 PST 2002
//    Added timeout support.
//
//    Brad Whitlock, Tue Apr 9 13:43:12 PST 2002
//    Ported to Windows.
//
//    Hank Childs, Tue Jun 24 18:02:01 PDT 2003
//    Allow for timeouts during network executions.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Jeremy Meredith, Mon Nov  1 13:26:23 PST 2004
//    Use a buffer connection that is visible at class scope for parallel
//    communication.  Before, it was an automatic variable in this method
//    but I needed a separate PAR_ProcessInput method that used the same
//    buffer connection.
//
//    Kathleen Bonnell, Mon May  9 13:27:18 PDT 200 
//    Changed the timeout to 10 minutes. 
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Mark C. Miller, Tue Feb 13 16:24:58 PST 2007
//    Replaced MPI_Bcast with MPIXfer::VisIt_MPI_Bcast
//
//    Sean Ahern, Wed Dec 12 12:02:21 EST 2007
//    Made a distinction between the execution and the idle timeouts.
//
//    Hank Childs, Mon Sep  8 16:39:03 PDT 2008
//    Make sure quitRPC is properly communicated to all processors; prevents
//    runaway engines.
//
//    Tom Fogal, Wed May 27 14:07:19 MDT 2009
//    Removed a duplicate debug statement.
//
//    Brad Whitlock, Fri Jun 19 09:05:31 PDT 2009
//    I rewrote the code for receiving the command to other processors.
//    Instead of receiving 1..N 1K size messages per command, with N being 
//    more common, we now receive 1 size message and then we receive the 
//    entire command at once.
//
// ****************************************************************************

void
Engine::PAR_EventLoop()
{
    if (PAR_UIProcess())
    {
        // The master process executes the serial event loop since it
        // communicates with the viewer.
        bool errFlag = EventLoop();

        // If the errFlag is set, we exited the event loop because we lost
        // the connection to the viewer. We need to send a quit signal
        // to all other processes.
        if (errFlag || !noFatalExceptions)
        {
            Connection *conn = xfer->GetBufferedInputConnection();
            quitRPC->SetQuit(true);
            conn->Flush();
            int hardcodedOpcodeForQuitRPC = 0;
            conn->WriteInt(hardcodedOpcodeForQuitRPC);
            int msg_size = quitRPC->CalculateMessageSize(*conn);
            conn->WriteInt(msg_size);
            quitRPC->Write(*conn);
            xfer->Process();
        }
    }
    else
    {
        // Non-UI Process
        while(!quitRPC->GetQuit() && noFatalExceptions)
        {
            // Reset the alarm
            overrideTimeoutEnabled = false;

            idleTimeoutEnabled = true;
            debug5 << "Resetting idle timeout to " << idleTimeoutMins << " minutes." << endl;
            ResetTimeout(idleTimeoutMins * 60);

            // Get the next message length.
            int msgLength = 0;
            MPIXfer::VisIt_MPI_Bcast((void *)&msgLength, 1, MPI_INT,
                                     0, VISIT_MPI_COMM);

            // Read the incoming message. Use regular MPI_Bcast since this
            // message is guaranteed to come right after the other one.
#ifdef VISIT_BLUE_GENE_P
            // Make the buffer be 32-byte aligned
            unsigned char *buf = 0;
            posix_memalign((void **)&buf, 32, msgLength);
#else
            unsigned char *buf = (unsigned char*)malloc(msgLength * sizeof(unsigned char));
#endif
            MPI_Bcast((void *)buf, msgLength, MPI_UNSIGNED_CHAR, 0, VISIT_MPI_COMM);
            par_conn.Append(buf, msgLength);
            free(buf);

            // We have work to do, so reset the alarm.
            idleTimeoutEnabled = false;
            debug5 << "Resetting execution timeout to " << executionTimeoutMins << " minutes." << endl;
            ResetTimeout(executionTimeoutMins * 60);

            // Process the state information.
            xfer->Process();

            idleTimeoutEnabled = true;
            ResetTimeout(idleTimeoutMins * 60);
        }
    }
}

// ****************************************************************************
//  Method:  Engine::PAR_ProcessInput
//
//  Purpose:
//    Same purpose as ProcessInput, but parallel-aware.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Mark C. Miller, Tue Feb 13 16:24:58 PST 2007
//    Replaced MPI_Bcast with MPIXfer::VisIt_MPI_Bcast
// ****************************************************************************
void
Engine::PAR_ProcessInput()
{    
    if (PAR_UIProcess())
    {
        ProcessInput();
    }
    else
    {
        MPIXfer::VisIt_MPI_Bcast((void *)&par_buf, 1,
            PAR_STATEBUFFER, 0, VISIT_MPI_COMM);
        par_conn.Append((unsigned char *)par_buf.buffer, par_buf.nbytes);
        xfer->Process();
    }
}
#endif

// ****************************************************************************
// Function: EventLoop
//
// Purpose:
//   This is the serial event loop for the engine. It reads from the
//   socket attached to the viewer and processes the state information
//   that it reads.
//
// Notes:      Executes the event loop until the quit State object
//             says to quit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:08:27 PST 2000
//
// Modifications:
//    Jeremy Meredith, Wed Aug  9 14:42:44 PDT 2000
//    Changed the read socket to a real variable.
//
//    Brad Whitlock, Thu Mar 15 13:44:41 PST 2001
//    I rewrote the event loop so it uses a call to select. This allowed me
//    to add code that tests to see if we've lost a connection. If we lost
//    the connection, quit the program.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Sean Ahern, Thu Feb 21 16:14:52 PST 2002
//    Added timeout support.
//
//    Brad Whitlock, Mon Mar 25 16:03:54 PST 2002
//    Made the connection and timeout code more general.
//
//    Hank Childs, Tue Jun 24 18:02:01 PDT 2003
//    Allow for timeouts during network executions.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Kathleen Bonnell, Mon May  9 13:27:18 PDT 200 
//    Changed the timeout to 10 minutes. 
//
//    Sean Ahern, Wed Dec 12 12:04:15 EST 2007
//    Made a distinction between the execution and the idle timeouts.
//
//    Brad Whitlock, Thu Jun 11 15:12:36 PST 2009
//    I disabled the call to NeedsRead when we don't have select().
//
// ****************************************************************************

bool
Engine::EventLoop()
{
    bool errFlag = false;

    // The application's main loop
    while(!quitRPC->GetQuit() && noFatalExceptions)
    {
        // Reset the timeout alarm
        overrideTimeoutEnabled = false;

        idleTimeoutEnabled = true;
        debug5 << "Resetting idle timeout to " << idleTimeoutMins << " minutes." << endl;
        ResetTimeout(idleTimeoutMins * 60);

        //
        // Block until the connection needs to be read. Then process its
        // new input.
        //
#ifdef HAVE_SELECT
        if (xfer->GetInputConnection()->NeedsRead(true))
        {
#endif
            TRY
            {
                // We've got some work to do.  Reset the alarm.
                idleTimeoutEnabled = false;
                debug5 << "Resetting execution timeout to " << executionTimeoutMins << " minutes." << endl;
                ResetTimeout(executionTimeoutMins * 60);

                // Process input.
                ProcessInput();

                idleTimeoutEnabled = true;
                debug5 << "Resetting idle timeout to " << idleTimeoutMins << " minutes." << endl;
                ResetTimeout(idleTimeoutMins * 60);
            }
            CATCH(LostConnectionException)
            {
                // Indicate that we want to quit the application.
                quitRPC->SetQuit(true);
                errFlag = true;
            }
            ENDTRY
#ifdef HAVE_SELECT
        }
#endif
    }

    return errFlag;
}

// ****************************************************************************
//  Function: ProcessInput
//
//  Purpose:
//    Reads socket input from the viewer and adds it to the xfer object's
//    input. After doing that, the xfer object is called upon to process its
//    input.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 15 14:08:30 PST 2001
//
//  Modifications:
//    Brad Whitlock, Wed Mar 20 17:53:20 PST 2002
//    I abstracted the read code.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
// ****************************************************************************

void
Engine::ProcessInput()
{    
    // Try reading from the viewer.  
    int amountRead = xfer->GetInputConnection()->Fill();

    // If we got input, process it. Otherwise, start counting errors.
    if (amountRead > 0)
    {
        // Process the new information.
        xfer->Process();
    }
}

// ****************************************************************************
//  Function: ProcessCommandLine
//
//  Purpose:
//    Reads the command line arguments for the engine.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 21, 2001
//
//  Modifications:
//    Eric Brugger, Wed Nov  7 12:40:56 PST 2001
//    I added the command line argument -timing.
//
//    Sean Ahern, Thu Feb 21 16:12:43 PST 2002
//    Added timeout support.
//
//    Sean Ahern, Tue Dec  3 09:58:28 PST 2002
//    Added -dump support for streamer debugging.
//
//    Hank Childs, Mon May 12 19:44:50 PDT 2003
//    Add support for -lb-block, -lb-stride, and -lb-random.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
// 
//    Mark C. Miller, Thu Mar  4 12:07:04 PST 2004
//    Told NetworkManager to dump rendered images
//
//    Hank Childs, Sun Mar  6 08:42:50 PST 2005
//    Renamed -forcedynamic to -allowdynamic.  Removed -forcestatic argument.
//
//    Hank Childs, Sun Mar 27 13:29:13 PST 2005
//    Added more timing arguments.
//
//    Mark C. Miller, Thu Sep 15 11:30:18 PDT 2005
//    Added lb-absolute option
//
//    Hank Childs, Sat Dec  3 20:27:16 PST 2005
//    Add support for -hw-accel.
//
//    Hank Childs, Thu Jan  5 14:24:06 PST 2006
//    Don't reference NetworkManager, because it has not been instantiated yet.
//
//    Mark C. Miller, Thu Jan 19 18:12:46 PST 2006
//    Made more fault tolerant to errors in specifying arg to -timeout option
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Allowed '-timings' with an 's' too 
//
//    Mark C. Miller, Wed Aug  9 19:40:30 PDT 2006
//    Added "-stereo" option to support stereo in SR mode
//
//    Hank Childs, Thu Dec 21 09:19:37 PST 2006
//    Change DebugDump method from avtDataTreeIterator to avtFilter, since it is
//    now supported at a higher level.
//
//    Mark C. Miller, Wed Feb 14 15:22:33 PST 2007
//    Added -ui-bcast-thresholds CL option
//
//    Cyrus Harrison, Fri Mar 16 09:38:10 PDT 2007
//    Added -info-dump option
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added CL argument to specify cycle number regular expression 
//
//    Cyrus Harrison, Sat Aug 11 19:58:55 PDT 2007
//    Added -vtk-debug option
//
//    Dave Pugmire, Mon Dec 10 15:57:32 EST 2007
//    Added -plugindir option
//
//    Kathleen Bonnell, Wed Jan  2 08:15:07 PST 2008 
//    Fix -plugindir for Windows platform. 
//
//    Eric Brugger, Mon Jan  7 11:51:12 PST 2008
//    Made to use putenv instead of setenv if on Windows or HAVE_SETENV is
//    not defined.
//
//    Sean Ahern, Thu Jan 17 16:34:14 EST 2008
//    Got rid of the setenv thing entirely and instead called SetPluginDir on
//    the individual plugin managers.
//
//    Cyrus Harrison, Wed Feb 13 11:06:03 PST 2008
//    Change -dump and -info dump to set new avtDebugDumpOptions object. 
//
//    Hank Childs, Thu Feb 14 15:27:13 PST 2008
//    Added -lb-stream.
//
//    Cyrus Harrison, Tue Feb 19 08:42:51 PST 2008
//    Removed shouldDoDashDump (flag now contained in avtDebugDumpOptions)
//
//    Brad Whitlock, Tue Jun 24 15:27:45 PDT 2008
//    Changed how the plugin managers are accessed.
//
//    Tom Fogal, Fri Jul 11 11:55:43 EDT 2008
//    Added `icet' command line parameter.
//
//    Tom Fogal, Tue Aug  5 14:21:56 EDT 2008
//    Add `x-args' command line parameter.
//
//    Jeremy Meredith, Thu Aug  7 16:23:22 EDT 2008
//    Wrap parallel-only vars with appropriate ifdef.
//
//    Tom Fogal, Mon Aug 11 11:40:57 EDT 2008
//    Add `n-gpus-per-node' command line parameter.
//
//    Hank Childs, Sat Apr 11 23:41:27 CDT 2009
//    Added -never-output-timings flag.
//
// ****************************************************************************

void
Engine::ProcessCommandLine(int argc, char **argv)
{
    // process arguments.
    bool timingsAllowed = true;
    bool haveHWAccel = false;
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-allowdynamic") == 0)
            LoadBalancer::AllowDynamic();
        else if (strcmp(argv[i], "-stereo") == 0)
            NetworkManager::SetStereoEnabled();
        else if (strcmp(argv[i], "-hw-accel") == 0)
            haveHWAccel = true;
        else if (strcmp(argv[i], "-x-args") == 0 && i+1 < argc)
        {
            this->X_Args = std::string(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "-n-gpus-per-node") == 0 && i+1 < argc)
        {
            if(!StringHelpers::str_to_u_numeric<size_t>(argv[i+1],
                                                        &this->nDisplays))
            {
                debug1 << "Could not parse '-n-gpus-per-node' argument "
                       << "'" << argv[i+1] << "'. "
                       << "Disabling hardware acceleration!" << std::endl;
                this->nDisplays = 0;
                haveHWAccel = false;
            }
            i++;
        }
        else if ((strcmp(argv[i], "-timing") == 0 ||
                  strcmp(argv[i], "-timings") == 0) && timingsAllowed)
            visitTimer->Enable();
        else if (strcmp(argv[i], "-timing-processor-stride") == 0)
        {
            int stride = 1;
            if (i+1 < argc)
            {
                i++;
                stride = atoi(argv[i]);
            }
            if (stride <= 0)
            {
                if (PAR_Rank() != 0)
                {
                    visitTimer->Disable();
                    timingsAllowed = false;
                }
            }
            else
            {
                int modulo = (PAR_Rank() % stride);
                if (modulo != 0)
                {
                    visitTimer->Disable();
                    timingsAllowed = false;
                }
            }
 
        }
        else if (strcmp(argv[i], "-withhold-timing-output") == 0)
            visitTimer->WithholdOutput(true);
        else if (strcmp(argv[i], "-never-output-timings") == 0)
            visitTimer->NeverOutput(true);
        else if (strcmp(argv[i], "-timeout") == 0)
        {
            if (i+1 < argc)
            {
                char *endptr = 0;
                errno = 0;
                long int to = strtol(argv[i+1], &endptr, 10);
                if (*(argv[i+1]) != '\0' && *endptr == '\0' && errno == 0)
                {
                    idleTimeoutMins = (int) to;
                }
                else
                {
                    cerr << "-timeout option ignored due to bad argument." << endl;
                    debug1 << "-timeout option ignored due to bad argument." << endl;
                }
                i++;
            }
            else
            {
                cerr << "-timeout option ignored due to missing argument." << endl;
                debug1 << "-timeout option ignored due to missing argument." << endl;
            }
        }
        else if (strcmp(argv[i], "-ui-bcast-thresholds") == 0)
        {
            if (i+1 < argc)
            {
                char *endptr = 0;
                errno = 0;
                long int tval = strtol(argv[i+1], &endptr, 10);
                if (*(argv[i+1]) != '\0' && *endptr == '\0' && errno == 0)
                {
#ifdef PARALLEL
                    int nanoSecsOfSleeps = (int) tval;
#endif
                    int secsOfSpinBeforeSleeps = -1;

                    if (i+2 < argc)
                    {
                        endptr = 0;
                        errno = 0;
                        tval = strtol(argv[i+2], &endptr, 10);
                        if (*(argv[i+2]) != '\0' && *endptr == '\0' && errno == 0)
                        {
                            secsOfSpinBeforeSleeps = (int) tval;
                            i++;
                        }
                    }
#ifdef PARALLEL
                    MPIXfer::SetUIBcastThresholds(nanoSecsOfSleeps, secsOfSpinBeforeSleeps);
#endif
                }
                else
                {
                    cerr << "-ui-bcast-thresholds option ignored due to bad argument." << endl;
                    debug1 << "-ui-bcast-thresholds option ignored due to bad argument." << endl;
                }
                i++;
            }
            else
            {
                cerr << "-ui-bcast-thresholds option ignored due to missing argument." << endl;
                debug1 << "-ui-bcast-thresholds option ignored due to missing argument." << endl;
            }
        }
        else if (strcmp(argv[i], "-cycleregex") == 0)
        {
            avtDatabaseMetaData::SetCycleFromFilenameRegex(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "-dump") == 0)
        {
            avtDebugDumpOptions::EnableDump();
            
            // check for optional -dump output directory
            if( i+1 < argc && argv[i+1][0] !='-')
            {
                avtDebugDumpOptions::SetDumpDirectory(argv[i+1]);
                ++i;
            }
        }
        else if (strcmp(argv[i], "-info-dump") == 0)
        {
            avtDebugDumpOptions::EnableDump();
            avtDebugDumpOptions::DisableDatasetDump();
            
            // check for optional -dump output directory
            if( i+1 < argc && argv[i+1][0] !='-')
            {
                avtDebugDumpOptions::SetDumpDirectory(argv[i+1]);
                ++i;
            }
        }
        else if (strcmp(argv[i], "-vtk-debug") == 0)
        {
            avtDataObjectToDatasetFilter::SetVTKDebugMode(true);
            avtVariableCache::SetVTKDebugMode(true);
            vtkDebugStream::FrequentEventsFilter(true);
        }
        else if (strcmp(argv[i], "-lb-block") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER);
        }
        else if (strcmp(argv[i], "-lb-stride") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_STRIDE_ACROSS_BLOCKS);
        }
        else if (strcmp(argv[i], "-lb-stream") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_STREAM);
        }
        else if (strcmp(argv[i], "-lb-random") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_RANDOM_ASSIGNMENT);
        }
        else if (strcmp(argv[i], "-lb-absolute") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_ABSOLUTE);
        }
        else if (strcmp(argv[i], "-plugindir") == 0  && (i+1) < argc )
        {
            pluginDir = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-icet") == 0)
        {
            this->useIceT = true;
        }
    }
    avtCallback::SetSoftwareRendering(!haveHWAccel);
}

// ****************************************************************************
//  Function: AlarmHandler
//
//  Purpose:
//    Gracefully exits the engine if an SIGALRM signal was received.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Feb 21 16:13:43 PST 2002
//
//  Modifications:
//    Brad Whitlock, Tue Apr 9 13:46:32 PST 2002
//    Disabled on Windows.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Made it PAR_Exit() in parallel and call VisItInit::Finalize()
//
//    Hank Childs, Fri Apr 24 07:30:48 CDT 2009
//    Also print out timeout statement to cerr if in parallel.
//
// ****************************************************************************

void
Engine::AlarmHandler(int signal)
{
    Engine *e = Engine::Instance();
    if (e->overrideTimeoutEnabled == true)
    {
        if (PAR_Size() > 1)
        {
            cerr << PAR_Rank() << ": ENGINE exited due to an inactivity timeout of "
                << e->overrideTimeoutMins << " minutes.  Timeout was set through a callback. (Alarm received)" << endl;
        }
        debug1 << "ENGINE exited due to an inactivity timeout of "
            << e->overrideTimeoutMins << " minutes.  Timeout was set through a callback. (Alarm received)" << endl;
    } else
    {
        if (e->idleTimeoutEnabled == true)
        {
            if (PAR_Size() > 1)
            {
                cerr << PAR_Rank() << ": ENGINE exited due to an idle inactivity timeout of "
                    << e->idleTimeoutMins << " minutes. (Alarm received)" << endl;
            }
            debug1 << "ENGINE exited due to an idle inactivity timeout of "
                << e->idleTimeoutMins << " minutes. (Alarm received)" << endl;
        } else
        {
            if (PAR_Size() > 1)
            {
                cerr << PAR_Rank() << ": ENGINE exited due to an execution timeout of "
                    << e->executionTimeoutMins << " minutes. (Alarm received)" << endl;
            }
            debug1 << "ENGINE exited due to an execution timeout of "
                << e->executionTimeoutMins << " minutes. (Alarm received)" << endl;
        }
    }

    VisItInit::Finalize();
#ifdef PARALLEL
    PAR_Exit();
#endif
    exit(0);
}

// ****************************************************************************
//  Function: NewHandler
//
//  Purpose: Issue warning message when memory has run out
//
//  Programmer: Mark C. Miller 
//  Creation:   Tue Jun 29 17:34:19 PDT 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
Engine::NewHandler(void)
{
#ifdef PARALLEL
    const char *msg = "VisIt: engine out of memory, try more processors";
#else
    const char *msg = "VisIt: engine out of memory";
#endif

    debug1 << msg << endl;
    //cerr << msg << endl;

#ifdef PARALLEL
    MPI_Abort(VISIT_MPI_COMM, 18);
#else
    abort();
#endif

}

// ****************************************************************************
//  Method: WriteByteStreamToSocket
//
//  Purpose:
//      Writes a byte stream to a socket.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

static void
WriteByteStreamToSocket(NonBlockingRPC *rpc, Connection *vtkConnection,
                        avtDataObjectString &do_str)
{
    int totalSize = do_str.GetTotalLength();
    rpc->SendReply(totalSize);
    int writeData = visitTimer->StartTimer();
    int nStrings = do_str.GetNStrings();
    debug5 << "sending " << totalSize << " bytes to the viewer " << nStrings
           << " from strings." << endl;

    const int buff_size = 4096;
    unsigned char buffer[buff_size];
    int buff_cur = 0;
    int strings_written = 0;
    for (int i = 0 ; i < nStrings ; i++)
    {
        int size;
        char *str;
        do_str.GetString(i, str, size);

        if ((buff_cur + size) < buff_size)
        {
            // Put this message into the buffer.
            memcpy(buffer + buff_cur, str, size*sizeof(char));
            buff_cur += size;
        }
        else
        {
            // We can't put this message into "buffer", because
            // that would exceed buffer's size.  Write "buffer"
            // first, or else we would be sending messages out of
            // order.
            vtkConnection->DirectWrite(buffer, long(buff_cur));
            strings_written++;

            buff_cur = 0;
            if (size > buff_size)
            {
                // It's big. Just write this string directly.
                vtkConnection->DirectWrite((const unsigned char *)str,
                                           long(size));
                strings_written++;
            }
            else
            {
                memcpy(buffer + buff_cur, str, size*sizeof(char));
                buff_cur += size;
            }
        }
        
        // We have no more strings, so just write what we have.
        if ((i == (nStrings-1)) && (buff_cur > 0))
        {
            vtkConnection->DirectWrite(buffer, long(buff_cur));
            strings_written++;
            buff_cur = 0;
        }
    }

    debug5 << "Number of actual direct writes = " << strings_written << endl;

    char info[124];
    SNPRINTF(info, 124, "Writing %d bytes to socket", totalSize);     
    visitTimer->StopTimer(writeData, info);
}

#ifdef PARALLEL
// ****************************************************************************
//  Function SwapAndMergeWriterOutputs
//
//  Purpose:
//      Swaps avtDataObjects between this processor and another and
//      Merges it into this processor's data object.
//
//  Programmer: Mark C. Miller (borrowed from avtDataObjectInformation)
//  Creation:   June 25, 2009 
//
// ****************************************************************************
static void
SwapAndMergeClonedWriterOutputs(avtDataObject_p dob,
   int swapWithProc, int mpiSwapLenTag, int mpiSwapStrTag)
{

   MPI_Status mpiStatus;
   char *srcStr, *dstStr;
   int   srcLen,  dstLen;

   // serialize the data object information into a string
   avtDataObjectString srcDobStr;
   avtDataObjectWriter *dobwrtr = dob->InstantiateWriter();
   dobwrtr->SetInput(dob);
   dobwrtr->Write(srcDobStr);
   srcDobStr.GetWholeString(srcStr, srcLen);

   // swap string lengths
   MPI_Sendrecv(&srcLen, 1, MPI_INT, swapWithProc, mpiSwapLenTag,
                &dstLen, 1, MPI_INT, swapWithProc, mpiSwapLenTag,
                VISIT_MPI_COMM, &mpiStatus);

   dstStr = new char [dstLen];

   // swap strings
   MPI_Sendrecv(srcStr, srcLen, MPI_CHAR, swapWithProc, mpiSwapStrTag,
                dstStr, dstLen, MPI_CHAR, swapWithProc, mpiSwapStrTag,
                VISIT_MPI_COMM, &mpiStatus);

   avtDataObjectReader *avtreader = new avtDataObjectReader;
   avtreader->Read(dstLen, dstStr);
   avtDataObject_p destdob = avtreader->GetOutput();

   // We can't tell the reader to read (Update) unless we tell it
   // what we want it to read.  Fortunately, we can just ask it
   // for a general specification.
   avtOriginatingSource *src = destdob->GetOriginatingSource();
   avtContract_p spec = src->GetGeneralContract();
   destdob->Update(spec);

   // Do what we came here for.
   dob->Merge(*destdob);

   // The data object reader will delete the dstStr string we allocated above.
   delete avtreader;
}

// ****************************************************************************
//  Function ParallelMergeWriterOutputs
//
//  Purpose:
//      Performs a tree-like sequence of swap and merge operations. At each
//      iteration through the loop, the entire communicator of processors is
//      divided into groups which are known to have merged results. One processor
//      in each even numbered group is paired with one processor in each odd
//      numbered group and vice versa. As long as the processor identified to
//      swap with is in the range of the communicator, the avtDataObject
//      is swapped and merged between these pairs of processors. The group
//      size is doubled and the process of pairing and swapping is repeated.
//      This continues until the group size equals or exceeds the communicator
//      size. At this point, one or two processors has merged results that include
//      the influence of every other processor even if they did not explicitly
//      communicate with each other. In all cases, one of those includes
//      processor 0.
//
//  Programmer: Mark C. Miller (copied from avtDataObjectInformation)
//  Creation:   June 25, 2009
//
//  Modifications:
//
//    Mark C. Miller, Sat Jun 27 07:45:17 PDT 2009
//    Added simple status reporting
// ****************************************************************************

static void
ParallelMergeClonedWriterOutputs(avtDataObject_p dob, int lenTag, int strTag,
    NonBlockingRPC *rpc)
{
    int groupSize = 1;
    int myRank, commSize;

    MPI_Comm_size(VISIT_MPI_COMM, &commSize);
    MPI_Comm_rank(VISIT_MPI_COMM, &myRank);

    //
    // Determine number of times we'll iterate up the tree
    //
    int ntree = 0;
    while (groupSize < commSize)
    {
        groupSize <<= 1;
        ntree++;
    }
    
    // walk up the communication tree, swapping and merging infos
    int n = 1;
    groupSize = 1;
    while (groupSize < commSize)
    {
        int swapWithProc = -1;
        int myGroupNum = myRank / groupSize;
        int myGroupIdx = myRank % groupSize;

        // determine processor to swap with
        if (myGroupNum % 2)   // myGroupNum is odd
            swapWithProc = (myGroupNum - 1) * groupSize + myGroupIdx;
        else                  // myGroupNum is even
            swapWithProc = (myGroupNum + 1) * groupSize + myGroupIdx;

        // only do the swap between 0th processors in each group AND only
        // if the processor to swap with is in range of communicator
        if ((myGroupIdx == 0) && (0 <= swapWithProc) && (swapWithProc < commSize))
            SwapAndMergeClonedWriterOutputs(dob, swapWithProc, lenTag, strTag);
        
        //
        // Only the root will have non-zero rpc.
        //
        if (rpc)
        {
            rpc->SendStatus((int) (100. * float(n)/float(ntree)),
                rpc->GetCurStageNum(),
                "Synchronizing",
                rpc->GetMaxStageNum());
        }
 
        n++;
        groupSize <<= 1;
    }
}

// ****************************************************************************
// Function: SumWithINT_MAX_Func
//
// Purpose:  User defined MPI reduction operator to perform an MPI_SUM
//           operation of a single integer value (cell count) but taking into
//           account the possibility of INT_MAX being one of the summands and
//           causing overflow.
//
// Programmer: Mark C. Miller
// Created:    Thu Jun 25 10:27:24 PDT 2009
// ****************************************************************************
static void SumWithINT_MAX_Func(void *ibuf, void *iobuf, int *, MPI_Datatype *)
{
    int *ival = (int *) ibuf;
    int *ioval = (int *) iobuf;

    // If iobuf is already at INT_MAX, it is the 'special' case I think for
    // volume rendering. Leave it alone.
    if (*ioval == INT_MAX)
        return;

    // If iobuf is at INT_MAX-1, it is an 'ordinary' overflow condition.
    // Again, we leave it alone.
    if (*ioval == INT_MAX-1)
        return;

    // If ibuf is at INT_MAX, set INT_MAX in iobuf and we're done.
    if (*ival == INT_MAX)
    {
        *ioval = INT_MAX;
        return;
    }

    // If ibuf is at INT_MAX-1, it is an 'ordinary' overflow condition.
    // Set it in iobuf and we're done.
    if (*ival == INT_MAX-1)
    {
        *ioval = INT_MAX-1;
        return;
    }

    //
    // If we get here, we have real summation work to do.
    //
    
    // First check for possible 'ordinary' overflow in that both inputs could
    // be valid (e.g. non-INT_MAX) integers but their sum is out of range. If
    // that is the case, we use the value 'INT_MAX-1' to represent it because
    // the value INT_MAX is used for some special purpose for volume rendering.
    double tmp = (double) *ioval + (double) *ival;
    if (tmp >= INT_MAX)
    {
        *ioval = INT_MAX-1;
        return;
    }

    *ioval = *ioval + *ival;
}
#endif

// ****************************************************************************
// Function: WriteData
//
// Purpose:
//   Writes a vtkDataSet object back to the viewer
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September 5, 2000
//
// Modifications:
//    Jeremy Meredith, Thu Sep 21 22:17:42 PDT 2000
//    I made this append the output from all processors in parallel
//    before sending it back to the viewer.
//
//    Jeremy Meredith, Tue Sep 26 16:41:09 PDT 2000
//    Made this use an avtDataSetWriter in serial.  Parallel is
//    temporarily broken.
//
//    Jeremy Meredith, Thu Sep 28 12:44:22 PDT 2000
//    Fixed for parallel.
//
//    Hank Childs, Tue Oct 17 09:38:12 PDT 2000
//    Made argument be a reference counted avtDataset instead of a normal
//    avtDataset.
//
//    Brad Whitlock, Wed Oct 18 14:48:29 PST 2000
//    I fixed a bug with using the ref_ptr. I also added code to set
//    the destination format in the avtDataSetWriter used by the UI-process
//    so the avtDataSet it sends back is in the right format. Note that
//    I didn't set the destination format in the non-UI processes because
//    the string created by non-UI processes is sent to the UI process
//    where it is read and resent to the viewer.
//
//    Hank Childs, Wed Oct 18 16:57:03 PDT 2000
//    Cleaned up memory leak.
//
//    Hank Childs, Wed Jan 17 11:37:36 PST 2001
//    Made input be a data object writer rather than data.
//
//    Hank Childs, Mon Feb 12 07:55:47 PST 2001
//    Fix logic for parallel.
//
//    Brad Whitlock, Tue May 1 13:45:31 PST 2001
//    Added code to send back status.
//
//    Hank Childs, Sat May 26 10:31:14 PDT 2001
//    Made use of avtDataObjectString to prevent bottlenecking at proc 0.
//
//    Jeremy Meredith, Fri Jun 29 14:50:18 PDT 2001
//    Added progress reporting, even in parallel.
//
//    Jeremy Meredith, Tue Aug 14 14:45:12 PDT 2001
//    Made the final progress update occur before the SendReply because
//    SendStatus is for reporting *incomplete* progress.  If it got sent
//    too soon, it overwrote the complete status in the viewer and 
//    the viewer never realized it was done.  Thus, it hung.
//
//    Hank Childs, Sun Sep 16 14:55:48 PDT 2001
//    Add timing information.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Mon Sep 17 11:20:10 PDT 2001
//    Have the data object reader handle deleting data strings.
//
//    Hank Childs, Mon Oct  1 11:31:27 PDT 2001
//    Stop assumptions about data objects.
//
//    Eric Brugger, Mon Nov  5 13:50:49 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Mon Jan  7 19:39:32 PST 2002
//    Fix memory leak.
//
//    Brad Whitlock, Tue Mar 26 10:56:38 PDT 2002
//    Changed the communication code so it uses connection objects.
//
//    Sean Ahern, Thu Jun 13 10:51:17 PDT 2002
//    Removed broken, unused code.
//
//    Hank Childs, Sun Aug 18 21:41:29 PDT 2002
//    Removed progress message that said "Transferring Data Set" as we were
//    really waiting for the processors to synchronize.
//
//    Hank Childs (Mark Miller), Mon May 12 18:04:51 PDT 2003
//    Corrected slightly misleading debug statement.
//
//    Hank Childs, Fri May 16 16:26:03 PDT 2003
//    Detect if there was a failure in the pipeline and send a message to
//    the viewer if so.
//
//    Kathleen Bonnell, Thu Jun 12 10:57:11 PDT 2003 
//    Split timing code to time Serialization separately from write. 
//    
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Mark C. Miller, Wed Feb  4 19:45:35 PST 2004
//    Made the ui_dob a 'clone' of the writer's input
//
//    Hank Childs, Wed Feb 25 11:40:21 PST 2004
//    Fix mis-spelling of cumulative (the function we were calling changed
//    names).
//
//    Hank Childs, Fri Mar 19 21:20:12 PST 2004
//    Use a helper routine (that's more efficient) to write to a socket.
//
//    Mark C. Miller, Mon May 24 18:36:13 PDT 2004
//    Added arguments to support checking of scalable threshold is exceeded
//    Modified communication algorithm to have each processor handshake with
//    UI proc and check if ok before actually sending data. When scalable
//    threshold is exceeded, UI proc tells everyone to stop sending data.
//
//    Mark C. Miller, Thu Jun 10 09:08:18 PDT 2004
//    Modified to use unique MPI message tags
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added explicit const bool polysOnly to document the fact that we
//    are counting polygons only during this phase. Also added code to
//    set processor 0's cell count and test if its count alone causes the
//    scalable threshold to be exceeded.
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added argument for cellCountMultiplier. Used cellCountMultiplier
//    to adjust cell counting for SR threshold
//
//    Hank Childs, Wed Dec  1 14:57:22 PST 2004
//    Automatically transition to SR mode with image based plots.
//
//    Mark C. Miller, Wed Dec 14 17:19:38 PST 2005
//    Added compression bool arg to method and appropriate calls to writers
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Mark C. Miller, Tue Jun 10 15:57:15 PDT 2008
//    Cast first arg of SendStatus to int explicitly
//
//    Mark C. Miller, Thu Jun 25 18:31:23 PDT 2009
//    Adjusted communication algorithm to eliminate point-to-point approach.
//    Replaced loop of p2p interactions with two collective operations; an
//    all reduce sum operation on the cell count being careful to take into
//    account possible INT_MAX values in the sumands; a tree-based swap
//    and merge algorithm to merge the data objects. An alternative to the
//    tree-based approach is a gatherv to the root. However, a problem with
//    the gatherv approach is that it cannot concurrently perform merges 
//    with communication. So, after the gatherv completes, the root proc
//    winds up with COMM_SIZE strings that it still needs to a) convert
//    back to data objects and b) merge together. Care was taken to ensure
//    all other logic with respect to SR mode thresholds and INT_MAX special
//    case for volume rendering was upheld.
//
//    Mark C. Miller, Fri Jun 26 18:52:34 PDT 2009
//    Replaced '+=' assignment to currentCellCount from reducedCurrentCellCount
//    to just '=' assignment. This is correct because the reduced value has
//    already been summed.
//
//    Mark C. Miller, Fri Jun 26 18:59:00 PDT 2009
//    Removed extraneous debug statements
//
//    Mark C. Miller, Sat Jun 27 07:56:29 PDT 2009
//    Passed rpc to ParallelMerge function so it could report a simple
//    status.
// ****************************************************************************
void
Engine::WriteData(NonBlockingRPC *rpc, avtDataObjectWriter_p &writer,
    bool useCompression,
    bool respondWithNull, int scalableThreshold, bool* scalableThresholdExceeded,
    int currentTotalGlobalCellCount, float cellCountMultiplier,
    int* currentNetworkGlobalCellCount)
{

#ifdef PARALLEL

    static const bool polysOnly = true;
    int mpiSwapLenTag   = GetUniqueMessageTag();
    int mpiSwapStrTag   = GetUniqueMessageTag();

    //
    // When respond with null is true, this routine still has an obligation
    // to receive the dummied-up data tree from each processor, regardless of
    // whether or not the scalable threshold has been exceeded. So, we capture
    // that fact in the 'sendDataAnyway' bool. Likewise, when scalableThreshold
    // is -1, it means also to send the data anyway. 
    //
    bool sendDataAnyway = respondWithNull || scalableThreshold==-1;
    bool thresholdExceeded = false;
    int  currentCellCount = 0;

    if (PAR_UIProcess())
    {
        int collectAndWriteData = visitTimer->StartTimer();
        int collectData = visitTimer->StartTimer();

        // Send a second stage for the RPC.
        rpc->SendStatus(0,
                        rpc->GetCurStageNum(),
                        "Synchronizing",
                        rpc->GetMaxStageNum());

        avtDataObject_p ui_dob = writer->GetInput();

        if (cellCountMultiplier > INT_MAX/2.)
            currentCellCount = INT_MAX;
        else
            currentCellCount = (int) 
                   (ui_dob->GetNumberOfCells(polysOnly) * cellCountMultiplier);

        // test if we've exceeded the scalable threshold already with proc 0's
        // output
        if (currentTotalGlobalCellCount == INT_MAX ||
            currentCellCount == INT_MAX ||
            (currentTotalGlobalCellCount + currentCellCount 
                  > scalableThreshold))
        {
            debug5 << "exceeded scalable threshold of " << scalableThreshold 
                   << endl;
            thresholdExceeded = true; 
        }

        if (writer->MustMergeParallelStreams())
        {
            // we clone here to preserve this processor's orig network output
            // while we merge other proc's output into the cloned dob
            ui_dob = ui_dob->Clone();

            // If we're here to respondWithNull, that means we're really only going
            // to communicate the 'metadata' part of the data objects and NOT the
            // raw data. In that case, we do NOT need to do an initial communication
            // to determine how much total geometry (cells) we'll have because the
            // metadata part is 'small' and we need ALL of it to ship back to the
            // viewer anyways.
            int reducedCurrentCellCount = 0;
            if (!sendDataAnyway)
                MPI_Allreduce(&currentCellCount, &reducedCurrentCellCount, 1,
                    MPI_INT, SumWithINT_MAX_Op, VISIT_MPI_COMM);

            if (currentTotalGlobalCellCount == INT_MAX ||
                currentCellCount == INT_MAX ||
                reducedCurrentCellCount == INT_MAX ||
                reducedCurrentCellCount == INT_MAX-1 || // 'ordinary' overflow
                currentTotalGlobalCellCount + reducedCurrentCellCount > scalableThreshold)
            {
                if (!thresholdExceeded)
                {
                    debug5 << "Exceeded scalable threshold of " << scalableThreshold << endl;
                    if (reducedCurrentCellCount == INT_MAX-1)
                        debug5 << "This was due to 'oridinary' overflow in summing cell counts" << endl;
                }
                thresholdExceeded = true;
            }
            else
            {
                currentCellCount = reducedCurrentCellCount;
            }

            if (!thresholdExceeded || sendDataAnyway)
                ParallelMergeClonedWriterOutputs(ui_dob, mpiSwapLenTag, mpiSwapStrTag, rpc);

        }
        visitTimer->StopTimer(collectData, "Collecting data");

        // indicate that cumulative extents in data object now as good as true extents
        ui_dob->GetInfo().GetAttributes().SetCanUseCumulativeAsTrueOrCurrent(true);

        //
        // See if there was an error on another processor.
        //
        avtDataValidity &v = ui_dob->GetInfo().GetValidity();
        if (!v.HasErrorOccurred())
        {
            int serializeData = visitTimer->StartTimer();

            if (thresholdExceeded && !sendDataAnyway)
            {
                // dummy a null data object message to send to viewer
                avtNullData_p nullData = new avtNullData(NULL,AVT_NULL_DATASET_MSG);
                CopyTo(ui_dob, nullData);
            }

            // Create a writer to write across the network.
            avtDataObjectWriter_p networkwriter = ui_dob->InstantiateWriter();
            networkwriter->SetDestinationFormat(destinationFormat);
            networkwriter->SetUseCompression(useCompression);
            networkwriter->SetInput(ui_dob);
    
            avtDataObjectString do_str;
            networkwriter->Write(do_str);
    
            rpc->SendStatus(100,
                            rpc->GetCurStageNum(),
                            "Synchronizing",
                            rpc->GetMaxStageNum());
    
            visitTimer->StopTimer(serializeData, "Serializing data for writer");

            WriteByteStreamToSocket(rpc, vtkConnection, do_str);

        }
        else
        {
            rpc->SendError(v.GetErrorMessage());
        }

        const char *descStr = "Collecting data and writing it to viewer";
        visitTimer->StopTimer(collectAndWriteData, descStr);
    }
    else // non-UI processes
    {
        if (writer->MustMergeParallelStreams())
        {
            // send the "num cells I have" message to proc 0
            int numCells;
            if (cellCountMultiplier > INT_MAX/2.)
                numCells = INT_MAX;
            else
                numCells = (int) 
                             (writer->GetInput()->GetNumberOfCells(polysOnly) *
                                                      cellCountMultiplier);

            int reducedCurrentCellCount = 0;
            if (!sendDataAnyway)
                MPI_Allreduce(&numCells, &reducedCurrentCellCount, 1,
                    MPI_INT, SumWithINT_MAX_Op, VISIT_MPI_COMM);

            if (currentTotalGlobalCellCount == INT_MAX ||
                currentCellCount == INT_MAX ||
                reducedCurrentCellCount == INT_MAX ||
                reducedCurrentCellCount == INT_MAX-1 || // 'ordinary' overflow
                currentTotalGlobalCellCount + reducedCurrentCellCount > scalableThreshold)
            {
                thresholdExceeded = true;
            }
            else
            {
                currentCellCount = reducedCurrentCellCount;
            }

            if (!thresholdExceeded || sendDataAnyway)
            {
                avtDataObject_p dob = writer->GetInput();
                dob = dob->Clone();
                ParallelMergeClonedWriterOutputs(dob, mpiSwapLenTag, mpiSwapStrTag, 0);
            }
        }
        else
        {
            debug5 << "not sending data to proc 0 because the data object "
                   << "does not require parallel streams." << endl;
        }
    }

    // return requested arguments
    if (currentNetworkGlobalCellCount != 0)
        *currentNetworkGlobalCellCount = currentCellCount;
    if (scalableThresholdExceeded != 0)
        *scalableThresholdExceeded = thresholdExceeded;

#else // serial
    avtDataObject_p dob = writer->GetInput();
    dob->GetInfo().GetAttributes().SetCanUseCumulativeAsTrueOrCurrent(true);
    avtDataValidity &v = dob->GetInfo().GetValidity();
    if (!v.HasErrorOccurred())
    {
        avtDataObjectWriter_p writer_to_use = writer;
        if (cellCountMultiplier > INT_MAX/2.) // div2 for float precision
        {
            // dummy a null data object message to send to viewer
            avtNullData_p nullData =new avtNullData(NULL,AVT_NULL_DATASET_MSG);
            nullData->GetInfo().Copy(dob->GetInfo());
            CopyTo(dob, nullData);
            avtDataObjectWriter_p nullwriter = dob->InstantiateWriter();
            nullwriter->SetInput(dob);
            writer_to_use = nullwriter;
            *scalableThresholdExceeded = true;
            *currentNetworkGlobalCellCount = INT_MAX;
        }

        // Send a second stage for the RPC.
        rpc->SendStatus(0,
                        rpc->GetCurStageNum(),
                        "Transferring Data Set",
                        rpc->GetMaxStageNum());

        writer_to_use->SetDestinationFormat(destinationFormat);
        writer_to_use->SetUseCompression(useCompression);
        avtDataObjectString  do_str;
        writer_to_use->Write(do_str);

        rpc->SendStatus(100,
                        rpc->GetCurStageNum(),
                        "Transferring Data Set",
                        rpc->GetMaxStageNum());

        WriteByteStreamToSocket(rpc, vtkConnection, do_str);
    }
    else
    {
        rpc->SendError(v.GetErrorMessage());
    }
#endif
}

// ****************************************************************************
// Method: Engine::SendKeepAliveReply
//
// Purpose: 
//   Sends a small string to the client over the engine's data socket.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:32:25 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
Engine::SendKeepAliveReply()
{
#ifdef PARALLEL
    if(PAR_UIProcess())
    {
#endif
        //
        // Send a reply on the command socket.
        //
        keepAliveRPC->SendReply();

        //
        // Send a little data on the data socket.
        //
        const char *str = "VisIt!!!";
        vtkConnection->DirectWrite((const unsigned char *)str, 10);
#ifdef PARALLEL
    }
#endif
}

// ****************************************************************************
//  Function: EngineAbortCallback
//
//  Purpose:
//      Tells filters whether or not they should abort execution.
//
//  Arguments:
//      data     the xfer object to use
//
//  Programmer:   Jeremy Meredith
//  Creation:     July 3, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 20 18:28:22 PDT 2001
//    Added support for parallel interruption.
//
//    Brad Whitlock, Mon Mar 25 15:51:39 PST 2002
//    Made it more general.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Mark C. Miller, Thu Jun 10 10:05:09 PDT 2004
//    Modified to use a unique message tag for the interrupt message
//
//    Mark C. Miller, Fri Jun 11 09:39:11 PDT 2004
//    Made xfer local variable conditionally defined as MPIXfer or Xfer
//
//    Mark C. Miller, Fri Jun 11 13:21:42 PDT 2004
//    Made it use a static, file-scope const int as the message tag
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Brad Whitlock, Thu Jun 11 15:08:46 PST 2009
//    I disabled this code if we don't have select().
//
// ****************************************************************************

bool
Engine::EngineAbortCallbackParallel(void *data, bool informSlaves)
{
#ifdef HAVE_SELECT
#ifdef PARALLEL
    MPIXfer *xfer = (MPIXfer*)data;
#else
    Xfer *xfer = (Xfer*)data;
#endif

    if (!xfer)
        EXCEPTION1(VisItException,
                   "EngineAbortCallback called with no Xfer set.");

#ifdef PARALLEL

    // non-ui processes must do something entirely different
    if (!PAR_UIProcess())
    {
        int flag;
        MPI_Status status;
        MPI_Iprobe(0, INTERRUPT_MESSAGE_TAG, VISIT_MPI_COMM, &flag, &status);
        if (flag)
        {
            char buf[1];
            MPI_Recv(buf, 1, MPI_CHAR, 0, INTERRUPT_MESSAGE_TAG, VISIT_MPI_COMM, &status);
            return true;
        }
        return false;
    }
#endif

    //
    // Check to see if the connection has any input that should be read.
    //
    if (xfer->GetInputConnection()->NeedsRead())
    {
        xfer->GetInputConnection()->Fill();
    }

    bool abort = xfer->ReadPendingMessages();

#ifdef PARALLEL 
    // If needed, tell the non-ui processes to abort as well
    if (abort && informSlaves)
        xfer->SendInterruption(INTERRUPT_MESSAGE_TAG);
#endif
    return abort;
#else
    return false;
#endif
}

// ****************************************************************************
//  Method:  EngineAbortCallback
//
//  Purpose:
//    This is the normal callback which interfaces to the parallel
//    callback with the default arguments for an unknown code type.
//
//  Arguments:
//    data       the callback data
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
// ****************************************************************************

bool
Engine::EngineAbortCallback(void *data)
{
    return EngineAbortCallbackParallel(data, true);
}

// ****************************************************************************
//  Function: EngineUpdateProgressCallback
//
//  Purpose:
//      Updates the progress of the filter execution.
//
//  Arguments:
//      data       The current rpc (if it exists)
//      type       The filter type.
//      desc       A description from the filter (this may be NULL).
//      cur        The current node that finished execution.
//      total      The total number of nodes to execute on.
//
//  Notes:         When a filter starts or finishes entirely, it call this
//                 routine with total == 0 to indicate that, with cur == 0
//                 for starting and cur == 1 for finishing.
//
//  Programmer:   Hank Childs
//  Creation:     June 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Jun 29 14:44:59 PDT 2001
//    Added real status updates, and added cur==0/1 for starting/finishing.
//
//    Jeremy Meredith, Wed Aug 29 15:30:58 PDT 2001
//    Fixed domain indexing for partial stage completion.
//
//    Jeremy Meredith, Mon Sep 17 21:37:31 PDT 2001
//    Fixed previous fix....
//
//    Jeremy Meredith, Thu Sep 20 00:59:47 PDT 2001
//    Changed the way stage-finish updates are sent to the client.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Mark C. Miller, Fri Nov 11 09:45:42 PST 2005
//    Adding timing stuff to throttle progress messages to once per second
//
//    Hank Childs, Thu Feb  8 10:37:19 PST 2007
//    Add better support for queries over time.  They have a ton of stages,
//    but the current logic makes sure each stage is reported.  If there is
//    a ton of stages, then we should allow for some stages to not be reported.
//
//    Hank Childs, Thu Jan 31 16:58:34 PST 2008
//    Make sure all percentage completes are correct. 
//
//    Hank Childs, Fri Feb  1 09:03:22 PST 2008
//    Avoid a divide by zero.
//
//    Jeremy Meredith, Thu Aug  7 16:23:46 EDT 2008
//    Removed unused vars.
//
//    Brad Whitlock, Tue Mar  3 10:37:02 PST 2009
//    I made it okay to call this function without an RPC since RPC's are no
//    longer the only way to instigate engine functions into being called.
//
// ****************************************************************************

void
Engine::EngineUpdateProgressCallback(void *data, const char *type, const char *desc,
                             int cur, int total)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (rpc == 0)
        return;

    if (total == 0 && rpc->GetMaxStageNum() < 30)
    {
        if (cur == 0)
        {
            // Starting the new stage
            rpc->SendStatus(0,
                            rpc->GetCurStageNum(),
                            desc ? desc : type,  
                            rpc->GetMaxStageNum());
        }
        else
        {
            // Ending this stage...
            rpc->SendStatus(100,
                            rpc->GetCurStageNum(),
                            desc ? desc : type,
                            rpc->GetMaxStageNum());
            // And moving to the next one
            rpc->SendStatus(0,
                            rpc->GetCurStageNum() + 1,
                            desc ? desc : type,
                            rpc->GetMaxStageNum());
        }
    }
    else
    {
        static double timeOfLastProgressCallback = -1.0;
        double timeOfThisProgressCallback = TOA_THIS_LINE;

        double timeBetween = 1.0;
        if (rpc->GetMaxStageNum() >= 30)
            timeBetween = 0.2;
        if (total == 0 && cur != 0)
            rpc->SetCurStageNum(rpc->GetCurStageNum()+1);
        if (timeOfThisProgressCallback < timeOfLastProgressCallback + timeBetween)
            return;

        timeOfLastProgressCallback = timeOfThisProgressCallback;

        int percent = int(100. * float(cur)/(total+0.0001));
        percent = (percent < 0 ? 0 : percent);
        percent = (percent > 100 ? 100 : percent);
        rpc->SendStatus(percent,
                        rpc->GetCurStageNum(),
                        desc ? desc : type,
                        rpc->GetMaxStageNum());
    }
}


// ****************************************************************************
//  Function: EngineInitializeProgressCallback
//
//  Purpose:
//      Informs this module how many total filters will be executing.
//
//  Arguments:
//      data       The current rpc (if it exists)
//      nStages    The total number of stages in the pipeline.
//
//  Programmer:   Hank Childs
//  Creation:     June 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Jun 29 14:44:59 PDT 2001
//    Added real status updates.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Brad Whitlock, Tue Mar  3 10:37:02 PST 2009
//    I made it okay to call this function without an RPC since RPC's are no
//    longer the only way to instigate engine functions into being called.
//
// ****************************************************************************

void
Engine::EngineInitializeProgressCallback(void *data, int nStages)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (nStages > 0)
    {
        if(rpc != 0)
            rpc->SendStatus(0, 1, "Starting execution", nStages+1);
    }
    else
        debug1 << "ERROR: EngineInitializeProgressCallback called "
               << "with nStages == 0" << endl;
}


// ****************************************************************************
//  Function: EngineWarningCallback
//
//  Purpose:
//      Issues a warning through an rpc.
//
//  Arguments:
//      data      The current rpc (if it exists)
//      msg       The warning message.
//
//  Programmer:   Hank Childs
//  Creation:     October 18, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Hank Childs, Fri Mar  3 14:17:40 PST 2006
//    Changed name in exception to be correct.
//
//    Mark C. Miller, Mon Aug 20 18:30:24 PDT 2007
//    Don't throw an exception if rpc is NOT set; just send to debug1 log
//
// ****************************************************************************

void
Engine::EngineWarningCallback(void *data, const char *msg)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
    {
        debug1 << "EngineWarningCallback called with no RPC set. Message was..." << endl;
        debug1 << msg << endl;
    }
    else
    {
        rpc->SendWarning(msg);
    }
}

// ****************************************************************************
// Function: ResetTimeout
//
// Purpose: 
//   Resets the engine timeout.
//
// Arguments:
//   timeout : The number of seconds until we want another alarm.
//
// Programmer: Sean Ahern, Brad Whitlock
// Creation:   Tue Apr 9 13:41:29 PST 2002
//
// Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
// ****************************************************************************

void
Engine::ResetTimeout(int timeout)
{
#if !defined(_WIN32)
    alarm(timeout);
#endif    
}



// ****************************************************************************
//  Method:  Engine::PopulateSimulationMetaData
//
//  Purpose:
//    If this is a simulation acting as an engine, we need to send
//    current metadata (including a SIL) to the viewer.  This method
//    does that.
//
//  Arguments:
//    db,fmt:    filename and format needed to retrieve the database
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
//  Modifications:
//    Brad Whitlock, Fri Mar 27 14:12:16 PDT 2009
//    Allow xfer's updates so we can send the data back to the viewer in
//    case we're already responding to an update from the viewer.
//
// ****************************************************************************
void
Engine::PopulateSimulationMetaData(const std::string &db,
                                   const std::string &fmt)
{
    filename = db;
    format   = fmt;

    // Get the database
    ref_ptr<avtDatabase> database = 
                 netmgr->GetDBFromCache(filename,0,format.c_str())->GetDB();

    // Get the metadata
    *metaData = *database->GetMetaData(0);

    // Abort if this isn't a simulation
    if (!metaData->GetIsSimulation())
        return;

    // Get the SIL
    SILAttributes *tmp = database->GetSIL(0)->MakeSILAttributes();
    *silAtts = *tmp;
    delete tmp;

    // Send the metadata and SIL to the viewer
    if(!quitRPC->GetQuit())
    {
        xfer->SetUpdate(true);
        metaData->Notify();
        silAtts->SelectAll();
        silAtts->Notify();
    }
}

// ****************************************************************************
//  Method:  Engine::SimulationTimeStepChanged
//
//  Purpose:
//    If this is a simulation acting as an engine, call this on a
//    timestep change.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri May 13 17:06:19 PDT 2005
//    Skip this if there's not a filename -- it means we haven't had
//    a chance to open the simulation file yet.
//
// ****************************************************************************
void
Engine::SimulationTimeStepChanged()
{
    // We may have not had a chance to open the file yet
    if (filename == "")
        return;

    // Get the database
    ref_ptr<avtDatabase> database = 
                 netmgr->GetDBFromCache(filename,0,format.c_str())->GetDB();

    // Clear the old metadata and problem-sized data
    database->ClearMetaDataAndSILCache();
    database->FreeUpResources();

    // Send new metadata to the viewer
    PopulateSimulationMetaData(filename, format);
}

// ****************************************************************************
// Method: Engine::SimulationInitiateCommand
//
// Purpose: 
//   Tells the viewer to initiate a command.
//
// Arguments:
//   command : The command to execute.
//
// Note:       "UpdatePlots" is a special command that can be sent to make the
//             viewer update its plots using new data. Alternatively, any
//             command prefixed with "Interpret:" will be sent to the clients
//             where the CLI will interpret the code if the CLI exists.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 25 15:07:29 PST 2007
//
// Modifications:
//   Brad Whitlock, Thu Feb 26 13:59:36 PST 2009
//   I changed the argument to std::string.
//
//   Brad Whitlock, Fri Mar 27 13:55:27 PDT 2009
//   Enable xfer updates.
//
// ****************************************************************************

void
Engine::SimulationInitiateCommand(const std::string &command)
{
    if(!quitRPC->GetQuit())
    {
        // Allow the command to be sent, even if we're in the middle of an
        // Xfer::Process. This fixes a synchronization bug.
        xfer->SetUpdate(true);

        commandFromSim->SetCommand(command);
        commandFromSim->Notify();
    }
}

// ****************************************************************************
// Method: Engine::Message
//
// Purpose: 
//   This method lets the engine send a message back to the viewer.
//
// Arguments:
//   msg : The message to send back to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 13:59:47 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
Engine::Message(const std::string &msg)
{
    SimulationInitiateCommand(std::string("Message:") + msg);
}

// ****************************************************************************
// Method: Engine::Error
//
// Purpose: 
//   This method lets the engine send an error message back to the viewer.
//
// Arguments:
//   msg : The message to send back to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 13:59:47 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
Engine::Error(const std::string &msg)
{
    SimulationInitiateCommand(std::string("Error:") + msg);
}

// ****************************************************************************
//  Method:  Engine::Disconnect
//
//  Purpose:
//    Intended to disconnect the simulation from VisIt.
//
//  Arguments:
//    Close connections and free the engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************
void
Engine::DisconnectSimulation()
{
    delete instance;
    instance = NULL;
}

// ****************************************************************************
//  Method:  Engine::SetSimulationCommandCallback
//
//  Purpose:
//    Set the callback to control a simulation.
//
//  Arguments:
//    sc         the control command callback
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 18, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Mar 27 11:34:54 PDT 2009
//    I changed the function pointer type and added a callback data pointer.
//
// ****************************************************************************
void
Engine::SetSimulationCommandCallback(void (*scc)(const char*,const char*,void*),
    void *sccdata)
{
    simulationCommandCallback = scc;
    simulationCommandCallbackData = sccdata;
}

// ****************************************************************************
//  Method:  Engine::ExecuteSimulationCommand
//
//  Purpose:
//    Set the callback to control a simulation.
//
//  Arguments:
//    sc         the control command callback
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 18, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Mar 27 11:35:59 PDT 2009
//    I added simulationCommandCallbackData.
//
// ****************************************************************************
void
Engine::ExecuteSimulationCommand(const std::string &command,
                                 const std::string &string_data)
{
    if (!simulationCommandCallback)
        return;

    simulationCommandCallback(command.c_str(),
                              string_data.c_str(),
                              simulationCommandCallbackData);
}

// ****************************************************************************
//  Method:  Engine::GetProcessAttributes
//
//  Purpose: Gets unix process attributes
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 8, 2005 
//
//  Modifications:
//    Brad Whitlock, Tue May 10 15:52:16 PST 2005
//    Fixed for win32.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Removed extraneous cerr statement 
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

ProcessAttributes *
Engine::GetProcessAttributes()
{
    if (procAtts == NULL)
    {
        procAtts = new ProcessAttributes;

        doubleVector pids;
        doubleVector ppids;
        stringVector hosts;

#if defined(_WIN32)
        int myPid = _getpid();
        int myPpid = -1;
#else
        int myPid = getpid();
        int myPpid = getppid();
#endif

#ifdef PARALLEL

        char myHost[2*MPI_MAX_PROCESSOR_NAME];
        int strLen;
        MPI_Get_processor_name(myHost, &strLen); 

        bool isParallel = true;

        // collect pids and host names
        int *allPids;
        int *allPpids;
        char *allHosts;
        if (PAR_Rank() == 0)
        {
            allPids = new int[PAR_Size()];
            allPpids = new int[PAR_Size()];
            allHosts = new char[PAR_Size() * sizeof(myHost)];
        }

        MPI_Gather(&myPid, 1, MPI_INT,
                   allPids, 1, MPI_INT, 0, VISIT_MPI_COMM);
        MPI_Gather(&myPpid, 1, MPI_INT,
                   allPpids, 1, MPI_INT, 0, VISIT_MPI_COMM);
        MPI_Gather(&myHost, sizeof(myHost), MPI_CHAR,
                   allHosts, sizeof(myHost), MPI_CHAR, 0, VISIT_MPI_COMM);

        if (PAR_Rank() == 0)
        {
            for (int i = 0; i < PAR_Size(); i++)
            {
                pids.push_back(allPids[i]);
                ppids.push_back(allPpids[i]);
                hosts.push_back(&allHosts[i*sizeof(myHost)]);
            }

            delete [] allPids;
            delete [] allPpids;
            delete [] allHosts;
        }

#else

        pids.push_back(myPid);
        ppids.push_back(myPpid);

        char myHost[256];
        gethostname(myHost, sizeof(myHost));
        hosts.push_back(myHost);

        bool isParallel = false;

#endif

        procAtts->SetPids(pids);
        procAtts->SetPpids(ppids);
        procAtts->SetHosts(hosts);
        procAtts->SetIsParallel(isParallel);

    }

    return procAtts;

}

// ****************************************************************************
//  Method: SetupDisplay
//
//  Purpose:
//    Figure out which processes should start HW displays, and which should
//    start Mesa (SW) displays.  Use VisItDisplay to initialize those displays.
//
//  Programmer: Tom Fogal
//  Creation:   July 27, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue Jul 29 10:28:07 EDT 2008
//    Add a check to make sure we don't start too many X servers.
//    Wait for the X server to start up correctly before connecting.
//
//    Tom Fogal, Mon Aug 11 13:55:38 EDT 2008
//    Cast to avoid a warning.
//    Use cog_set_max instead of calling '_min twice.
//
//    Tom Fogal, Fri Aug 29 10:19:00 EDT 2008
//    Reorganize so we only call InitVTK::*orceMesa once; this seems to work
//    more reliably.
//
//    Brad Whitlock, Fri Aug 29 09:55:09 PDT 2008
//    Added Mac-specific code for unsetenv since it returns void on Mac.
//
//    Tom Fogal, Mon Sep  1 12:54:29 EDT 2008
//    Change to a method from a static function, and delegate to VisItDisplay.
//
//    Tom Fogal, Sun Mar  8 00:25:52 MST 2009
//    Allow a HW context even in serial mode.
//
// ****************************************************************************

void
Engine::SetupDisplay()
{
    int display = -1;  // Display ID to create.
#ifdef PARALLEL
    cog_set lnodes;

    cog_identify();

    cog_set_local(&lnodes, PAR_Rank());
    int rank;
    int min = cog_set_min(&lnodes);
    int max = cog_set_max(&lnodes);

    // Explicitly nullify it.  If it's still null after the loop, we'll know we
    // should setup a SW display.
    this->renderingDisplay = NULL;

    for(rank = min; rank <= max; ++rank)
    {
        if(cog_set_intersect(&lnodes, rank))
        {
            if(PAR_Rank() == rank &&
               static_cast<size_t>(rank-min) < this->nDisplays)
            {
                display = rank-min;
                this->renderingDisplay = Display::Create(Display::D_X);
            }
        }
    }
#else
    if(this->nDisplays > 0)
    {
        this->renderingDisplay = Display::Create(Display::D_X);
        display = 0;
    }
#endif
    if(this->renderingDisplay == NULL)
    {
        this->renderingDisplay = Display::Create(Display::D_MESA);
    }
    if(this->renderingDisplay->Initialize(display,
                               split(this->X_Args, PAR_Rank(), display)))
    {
        this->renderingDisplay->Connect();
    }
    else
    {
        debug1 << "Display initialization failed.  Rendering in this state "
               << "has undefined results ..." << std::endl;
    }
}

// ****************************************************************************
//  Function: ResetEngineTimeout
//
//  Purpose:
//      A static function that calls ResetTimeout.  This is meant to be a 
//      callback for libraries.
//
//  Programmer: Hank Childs
//  Creation:   September 5, 2006
//  
//  Sean Ahern, Wed Dec 12 12:08:38 EST 2007
//  Added a message to say what timeout is being affected.
//
// ****************************************************************************

static void
ResetEngineTimeout(void *p, int secs)
{
    Engine *e = (Engine *) p;
    e->SetOverrideTimeout(secs*60);
    if (e->IsIdleTimeoutEnabled() == false)
    {
        debug5 << "ResetEngineTimeout: Overriding timeout to " << secs << " seconds." << endl;
    } else
    {
        debug5 << "ResetEngineTimeout: We shouldn't get here!  Callbacks shouldn't set the timeout during idle!" << endl;
        debug5 << "ResetEngineTimeout: Overriding timeout to " << secs << " seconds." << endl;
    }
    e->ResetTimeout(secs);
}

// ****************************************************************************
// Method: Engine::SaveWindow
//
// Purpose: 
//   Tells the network manager to render and save a window.
//
// Arguments:
//   filename   : The filename in which to save an image.
//   imageWidth : The width of the image to save.
//   imageHeight: The height of the image to save.
//   fmt        : The file format to use for the saved image.
//
// Returns:    True if the image was saved; false otherwise.
//
// Note:       If imageWidth and imageHeight are <=0 then they are ignored and
//             the previous image resolution is used.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar  2 16:14:07 PST 2009
//
// Modifications:
//   
// ****************************************************************************

bool
Engine::SaveWindow(const std::string &filename, int imageWidth, int imageHeight,
    SaveWindowAttributes::FileFormat fmt)
{
    return netmgr->SaveWindow(filename, imageWidth, imageHeight, fmt);
}
