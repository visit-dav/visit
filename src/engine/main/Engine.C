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

#include <errno.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <strings.h>
#include <sys/types.h>   // for getpid()
#include <unistd.h>      // for alarm()
#else
#include <process.h>     // for _getpid()
#include <winsock.h>     // for gethostname()
#endif
#include <new>

#include <visitstream.h>
#include <snprintf.h>

#include <BufferConnection.h>
#include <CouldNotConnectException.h>
#include <DefineVirtualDatabaseRPC.h>
#include <IncompatibleVersionException.h>
#include <ExpressionList.h>
#include <ExprParser.h>
#include <ParsingExprList.h>
#include <avtExprNodeFactory.h>
#include <Init.h>
#include <InitVTK.h>
#include <LoadBalancer.h>
#include <LostConnectionException.h>
#include <Netnodes.h>
#include <ParentProcess.h>
#include <QueryAttributes.h>
#include <SILAttributes.h>
#include <SimulationCommand.h>
#include <SocketConnection.h>
#include <TimingsManager.h>
#include <vtkDebugStream.h>

#include <avtDatabaseMetaData.h>
#include <avtDataObjectReader.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtDataset.h>
#include <avtFilter.h>
#include <avtTerminatingSink.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <avtTypes.h>
#include <avtVariableMapper.h>
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
#endif

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
// ****************************************************************************

Engine::Engine()
{
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
    metaData = NULL;
    silAtts = NULL;
    commandFromSim = NULL;
    shouldDoDashDump = false;
    
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
// ****************************************************************************

Engine::~Engine()
{
    delete netmgr;
    delete xfer;
    delete lb;
    delete silAtts;
    delete metaData;
    delete commandFromSim;

    for (int i=0; i<rpcExecutors.size(); i++)
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
// ****************************************************************************
void
Engine::Initialize(int *argc, char **argv[], bool sigs)
{
    int initTimer = visitTimer->StartTimer();
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
    // Initialize error logging
    //
    Init::Initialize(*argc, *argv, PAR_Rank(), PAR_Size(), true, sigs);
#else
    xfer = new Xfer;
    Init::Initialize(*argc, *argv, 0,1, true, sigs);
#endif
    Init::SetComponentName("engine");

    //
    // Set a different new handler for the engine
    //
#if !defined(_WIN32)
    std::set_new_handler(Engine::NewHandler);
#endif

#ifdef PARALLEL
    if (!PAR_UIProcess())
    {
        // Set the xfer object's input connection to be the buffer connection
        // of the object itself
        xfer->SetInputConnection(&par_conn);
    }
#endif

    debug1 << "ENGINE started\n";
#ifdef PARALLEL
    visitTimer->StopTimer(initTimer, "Initializing the engine (including MPI_Init())");
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
// ****************************************************************************
void
Engine::Finalize(void)
{
    int finalizeTimer = visitTimer->StartTimer();
    Init::Finalize();
}

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
// ****************************************************************************

void
Engine::SetUpViewerInterface(int *argc, char **argv[])
{
    int setupTimer = visitTimer->StartTimer();

    vtkConnection = theViewer.GetReadConnection(1);

    // Parse the command line.
    ProcessCommandLine(*argc, *argv);

    //
    // Initialize the plugin managers.
    //
#ifdef PARALLEL
    PlotPluginManager::Initialize(PlotPluginManager::Engine, true);
    OperatorPluginManager::Initialize(OperatorPluginManager::Engine, true);
    DatabasePluginManager::Initialize(DatabasePluginManager::Engine, true);
#else
    PlotPluginManager::Initialize(PlotPluginManager::Engine, false);
    OperatorPluginManager::Initialize(OperatorPluginManager::Engine, false);
    DatabasePluginManager::Initialize(DatabasePluginManager::Engine, false);
#endif    
    //
    // Load plugins
    //
    PlotPluginManager::Instance()->LoadPluginsOnDemand();
    OperatorPluginManager::Instance()->LoadPluginsOnDemand();
    DatabasePluginManager::Instance()->LoadPluginsOnDemand();
    

    InitVTK::Initialize();
    if (avtCallback::GetSoftwareRendering())
        InitVTK::ForceMesa();
    else
        putenv("DISPLAY=:0");
    avtCallback::SetNowinMode(true);

    //
    // Create the network manager.  Note that this must be done *after* the
    // code to set the display and decide if we are using Mesa.
    //
    netmgr = new NetworkManager;
    if (shouldDoDashDump)
        netmgr->DumpRenders();

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
        xfer->SetInputConnection(theViewer.GetWriteConnection());
#else
    xfer->SetInputConnection(theViewer.GetWriteConnection());
#endif
    xfer->SetOutputConnection(theViewer.GetReadConnection());

    //
    // Set the global destination format. This only happens on the UI-Process
    // when running in parallel since non-UI processes have no SocketConnections.
    //
    if (theViewer.GetReadConnection() != 0)
        destinationFormat = theViewer.GetReadConnection()->GetDestinationFormat();

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
// ****************************************************************************

int
Engine::GetInputSocket()
{
    return theViewer.GetWriteConnection()->GetDescriptor();
}

// ****************************************************************************
// Function: ConnectViewer
//
// Purpose: 
//   Connects to the viewer.
//
// Arguments:
//   viewer : The viewer object that we want to connect.
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
// ****************************************************************************

bool
Engine::ConnectViewer(int *argc, char **argv[])
{
    int connectTimer = visitTimer->StartTimer();

    // Connect to the viewer.
    TRY
    {
#ifdef PARALLEL
        theViewer.Connect(1, 2, argc, argv, PAR_UIProcess());
#else
        theViewer.Connect(1, 2, argc, argv, true);
#endif
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
            quitRPC->Write(par_conn);
            xfer->SetInputConnection(&par_conn);
            xfer->SetEnableReadHeader(false);
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

            // Get state information from the UI process.
            MPIXfer::VisIt_MPI_Bcast((void *)&par_buf, 1,
                PAR_STATEBUFFER, 0, VISIT_MPI_COMM);

            // We have work to do, so reset the alarm.
            idleTimeoutEnabled = false;
            debug5 << "Resetting execution timeout to " << executionTimeoutMins << " minutes." << endl;
            ResetTimeout(executionTimeoutMins * 60);

            // Add the state information to the connection.
            par_conn.Append((unsigned char *)par_buf.buffer, par_buf.nbytes);

            // Process the state information.
            xfer->Process();

            idleTimeoutEnabled = true;
            debug5 << "Resetting idle timeout to " << idleTimeoutMins << " minutes." << endl;
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
        if (xfer->GetInputConnection()->NeedsRead(true))
        {
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
        }
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
                    int nanoSecsOfSleeps = (int) tval;
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
            avtFilter::DebugDump(true);
            avtTerminatingSink::DebugDump(true);
            shouldDoDashDump = true;
        }
        else if (strcmp(argv[i], "-info-dump") == 0)
        {
            avtFilter::DebugDump(true);
            avtTerminatingSink::DebugDump(true);
            avtDataRepresentation::DatasetDump(false);
            shouldDoDashDump = true;
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
            PlotPluginManager::Instance()->SetPluginDir(argv[i+1]);
            OperatorPluginManager::Instance()->SetPluginDir(argv[i+1]);
            DatabasePluginManager::Instance()->SetPluginDir(argv[i+1]);
            ++i;
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
//    Made it PAR_Exit() in parallel and call Init::Finalize()
//
// ****************************************************************************

void
Engine::AlarmHandler(int signal)
{
    Engine *e = Engine::Instance();
    if (e->overrideTimeoutEnabled == true)
    {
        debug1 << "ENGINE exited due to an inactivity timeout of "
            << e->overrideTimeoutMins << " minutes.  Timeout was set through a callback. (Alarm received)" << endl;
    } else
    {
        if (e->idleTimeoutEnabled == true)
        {
            debug1 << "ENGINE exited due to an idle inactivity timeout of "
                << e->idleTimeoutMins << " minutes. (Alarm received)" << endl;
        } else
        {
            debug1 << "ENGINE exited due to an execution timeout of "
                << e->executionTimeoutMins << " minutes. (Alarm received)" << endl;
        }
    }

    Init::Finalize();
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

    // set up MPI message tags
    int mpiCellCountTag   = GetUniqueMessageTag();
    int mpiSendDataTag    = GetUniqueMessageTag();
    int mpiDataObjSizeTag = GetUniqueMessageTag();
    int mpiDataObjDataTag = GetUniqueMessageTag();

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

            for (int i=1; i<PAR_Size(); i++)
            {
                MPI_Status stat;
                int size, proc_i_localCellCount;

                int shouldGetData = 1;
                int mpiSource = MPI_ANY_SOURCE; 

                // recv the "num cells I have" message from any proc
                MPI_Recv(&proc_i_localCellCount, 1, MPI_INT, MPI_ANY_SOURCE,
                    mpiCellCountTag, VISIT_MPI_COMM, &stat);

                mpiSource = stat.MPI_SOURCE;

                debug5 << "received the \"num cells I have\" (=" << proc_i_localCellCount
                       << ") message from processor " << mpiSource << endl;

                // accumulate this processors cell count in the total for this network
                if (currentCellCount != INT_MAX)
                    currentCellCount += proc_i_localCellCount;

                // test if we've exceeded the scalable threshold
                if (currentTotalGlobalCellCount == INT_MAX ||
                    currentCellCount == INT_MAX ||
                    (currentTotalGlobalCellCount + currentCellCount 
                          > scalableThreshold))
                {
                    if (!thresholdExceeded)
                        debug5 << "exceeded scalable threshold of " << scalableThreshold << endl;
                    shouldGetData = sendDataAnyway;
                    thresholdExceeded = true; 
                }

                // tell source processor whether or not to send data with
                // the "should send data" message
                MPI_Send(&shouldGetData, 1, MPI_INT, mpiSource, 
                    mpiSendDataTag, VISIT_MPI_COMM);
                debug5 << "told processor " << mpiSource << (shouldGetData==1?" to":" NOT to")
                       << " send data" << endl;

                if (shouldGetData)
                {
                    MPI_Recv(&size, 1, MPI_INT, mpiSource, 
                             mpiDataObjSizeTag, VISIT_MPI_COMM, &stat);
                    debug5 << "receiving size=" << size << endl;

                    debug5 << "receiving " << size << " bytes from MPI_SOURCE "
                           << mpiSource << endl;

                    char *str = new char[size];
                    MPI_Recv(str, size, MPI_CHAR, mpiSource, 
                             mpiDataObjDataTag, VISIT_MPI_COMM, &stat);
                    debug5 << "receiving data" << endl;
    
                    // The data object reader will delete the string.
                    avtDataObjectReader *avtreader = new avtDataObjectReader;
                    avtreader->Read(size, str);
                    avtDataObject_p proc_i_dob = avtreader->GetOutput();

                    // We can't tell the reader to read (Update) unless we tell it
                    // what we want it to read.  Fortunately, we can just ask it
                    // for a general specification.
                    avtOriginatingSource *src = proc_i_dob->GetOriginatingSource();
                    avtContract_p spec
                        = src->GetGeneralContract();
                    proc_i_dob->Update(spec);

                    ui_dob->Merge(*proc_i_dob);
                    delete avtreader;
                }

                rpc->SendStatus(100. * float(i)/float(PAR_Size()),
                                rpc->GetCurStageNum(),
                                "Synchronizing",
                                rpc->GetMaxStageNum());
            }
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

        char *descStr = "Collecting data and writing it to viewer";
        visitTimer->StopTimer(collectAndWriteData, descStr);
    }
    else // non-UI processes
    {
        if (writer->MustMergeParallelStreams())
        {
            char *str;
            int   size;
            avtDataObjectString do_str;
            writer->Write(do_str);
            do_str.GetWholeString(str, size);

            int shouldSendData = 1;
            MPI_Status stat;

            // send the "num cells I have" message to proc 0
            int numCells;
            if (cellCountMultiplier > INT_MAX/2.)
                numCells = INT_MAX;
            else
                numCells = (int) 
                             (writer->GetInput()->GetNumberOfCells(polysOnly) *
                                                      cellCountMultiplier);
            debug5 << "sending \"num cells I have\" message (=" << numCells << ")" << endl;
            MPI_Send(&numCells, 1, MPI_INT, 0, mpiCellCountTag, VISIT_MPI_COMM);

            // recv the "should send data" message from proc 0
            MPI_Recv(&shouldSendData, 1, MPI_INT, 0, mpiSendDataTag, VISIT_MPI_COMM, &stat);

            if (shouldSendData)
            {
               debug5 << "sending size=" << size << endl; 
               MPI_Send(&size, 1, MPI_INT, 0, mpiDataObjSizeTag, VISIT_MPI_COMM);
               debug5 << "sending " << size << " bytes to proc 0" << endl;
               debug5 << "sending data" << endl; 
               MPI_Send(str, size, MPI_CHAR, 0, mpiDataObjDataTag, VISIT_MPI_COMM);
            }
            else
            {
                debug5 << "not sending data to proc 0 because the scalable"
                       << "threshold has been exceeded." << endl;
            }
        }
        else
        {
            debug5 << "not sending data to proc 0 because the data object "
                   << "does not require parallel streams." << endl;
        }
    }

    //
    // all processors need to know the network's cell count and whether
    // scalable threshold was exceeded
    //
    int tmp[2] = {currentCellCount, thresholdExceeded?1:0};
    MPI_Bcast(tmp, 2, MPI_INT, 0, VISIT_MPI_COMM);
    currentCellCount  = tmp[0];
    thresholdExceeded = tmp[1]==1;

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
// ****************************************************************************

bool
Engine::EngineAbortCallbackParallel(void *data, bool informSlaves)
{

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
// ****************************************************************************

void
Engine::EngineUpdateProgressCallback(void *data, const char *type, const char *desc,
                             int cur, int total)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineUpdateProgressCallback called with no RPC set.");

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

        int cur1 = rpc->GetCurStageNum();
        int tot1 = rpc->GetMaxStageNum();
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
// ****************************************************************************

void
Engine::EngineInitializeProgressCallback(void *data, int nStages)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineInitializeProgressCallback called with no RPC set.");

    if (nStages > 0)
        rpc->SendStatus(0, 1, "Starting execution", nStages+1);
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
//   
// ****************************************************************************

void
Engine::SimulationInitiateCommand(const char *command)
{
    if(!quitRPC->GetQuit())
    {
        commandFromSim->SetCommand(command);
        commandFromSim->Notify();
    }
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
// ****************************************************************************
void
Engine::SetSimulationCommandCallback(void (*scc)(const char*,
                                                 int,float,const char*))
{
    simulationCommandCallback = scc;
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
// ****************************************************************************
void
Engine::ExecuteSimulationCommand(const std::string &command,
                                 int int_data,
                                 float float_data,
                                 const std::string &string_data)
{
    if (!simulationCommandCallback)
        return;

    simulationCommandCallback(command.c_str(),
                              int_data,float_data,string_data.c_str());
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
