#include <Engine.h>
#include <Executors.h>

#if !defined(_WIN32)
#include <strings.h>
#include <unistd.h>      // for alarm()
#endif
#include <new.h>

#include <visitstream.h>
#include <snprintf.h>

#include <BufferConnection.h>
#include <CouldNotConnectException.h>
#include <DefineVirtualDatabaseRPC.h>
#include <IncompatibleVersionException.h>
#include <ExpressionList.h>
#include <ParserInterface.h>
#include <ParsingExprList.h>
#include <EngineExprNodeFactory.h>
#include <Init.h>
#include <InitVTK.h>
#include <LoadBalancer.h>
#include <LostConnectionException.h>
#include <Netnodes.h>
#include <ParentProcess.h>
#include <QueryAttributes.h>
#include <SILAttributes.h>
#include <SocketConnection.h>

#include <avtDatabaseMetaData.h>
#include <avtDataObjectReader.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtDataset.h>
#include <avtFilter.h>
#include <avtOriginatingSink.h>
#include <avtStreamer.h>
#include <avtTerminatingSource.h>
#include <avtVariableMapper.h>
#include <vtkDataSetWriter.h>

#include <string>
using std::string;

#ifdef PARALLEL
#include <parallel.h>
#include <avtParallel.h>
#else
#include <Xfer.h>
#endif

// Static data
Engine *Engine::instance = NULL;

// Static method
static void WriteByteStreamToSocket(NonBlockingRPC *, Connection *,
                                    avtDataObjectString &);

#if defined(_WIN32)
// Get around a macro problem
#define GetMessageA GetMessage
#endif

// Initial connection timeout of 5 minutes (300 seconds)
#define INITIAL_CONNECTION_TIMEOUT 60

// message tag for interrupt messages used in static abort callback function
#ifdef PARALLEL
const int INTERRUPT_MESSAGE_TAG = GetUniqueMessageTag();
#endif

// ****************************************************************************
//  Constructor:  Engine::Engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
// ****************************************************************************
Engine::Engine()
{
    vtkConnection = 0;
    noFatalExceptions = true;
    timeout = 0;
    netmgr = NULL;
    lb = NULL;
}

// ****************************************************************************
//  Destructor:  Engine::~Engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
// ****************************************************************************
Engine::~Engine()
{
    delete netmgr;
    delete xfer;
    delete lb;
    for (int i=0; i<rpcExecutors.size(); i++)
        delete rpcExecutors[i];
}

// ****************************************************************************
//  Method:  Engine::Instance
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
// ****************************************************************************
Engine *Engine::Instance()
{
    if (!instance)
        instance = new Engine;
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
// ****************************************************************************
void
Engine::Initialize(int *argc, char **argv[])
{
#ifdef PARALLEL

    xfer = new MPIXfer;
    // Initialize for MPI and get the process rank & size.
    //
    PAR_Init(*argc, *argv);

    //
    // Initialize error logging
    //
    Init::Initialize(*argc, *argv, PAR_Rank(), PAR_Size());
#else
    xfer = new Xfer;
    Init::Initialize(*argc, *argv);
#endif
    Init::SetComponentName("engine");

    //
    // Set a different new handler for the engine
    //
#if !defined(_WIN32)
    set_new_handler(Engine::NewHandler);
#endif

    debug1 << "ENGINE started\n";
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
// ****************************************************************************
void
Engine::Finalize(void)
{
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
// ****************************************************************************

void
Engine::SetUpViewerInterface(int *argc, char **argv[])
{
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

    InitVTK::Initialize();
    InitVTK::ForceMesa();
    avtCallback::SetNowinMode(true);

    //
    // Load plugins
    //
    PlotPluginManager::Instance()->LoadPluginsOnDemand();
    OperatorPluginManager::Instance()->LoadPluginsOnDemand();
    DatabasePluginManager::Instance()->LoadPluginsOnDemand();

    vtkConnection = theViewer.GetReadConnection(1);

    //
    // Create the network manager.
    //
    netmgr = new NetworkManager;

    // Parse the command line.
    ProcessCommandLine(*argc, *argv);

#if !defined(_WIN32)
    // Set up the alarm signal handler.
    signal(SIGALRM, Engine::AlarmHandler);
#endif

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
    executeRPC                      = new ExecuteRPC;
    clearCacheRPC                   = new ClearCacheRPC;
    queryRPC                        = new QueryRPC;
    releaseDataRPC                  = new ReleaseDataRPC;
    openDatabaseRPC                 = new OpenDatabaseRPC;
    defineVirtualDatabaseRPC        = new DefineVirtualDatabaseRPC;
    renderRPC                       = new RenderRPC;
    setWinAnnotAttsRPC              = new SetWinAnnotAttsRPC;
    cloneNetworkRPC                 = new CloneNetworkRPC;

    xfer->Add(quitRPC);
    xfer->Add(keepAliveRPC);
    xfer->Add(readRPC);
    xfer->Add(applyOperatorRPC);
    xfer->Add(makePlotRPC);
    xfer->Add(useNetworkRPC);
    xfer->Add(updatePlotAttsRPC);
    xfer->Add(pickRPC);
    xfer->Add(startPickRPC);
    xfer->Add(executeRPC);
    xfer->Add(clearCacheRPC);
    xfer->Add(queryRPC);
    xfer->Add(releaseDataRPC);
    xfer->Add(openDatabaseRPC);
    xfer->Add(defineVirtualDatabaseRPC);
    xfer->Add(renderRPC);
    xfer->Add(setWinAnnotAttsRPC);
    xfer->Add(cloneNetworkRPC);

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
    rpcExecutors.push_back(new RPCExecutor<ExecuteRPC>(executeRPC));
    rpcExecutors.push_back(new RPCExecutor<ClearCacheRPC>(clearCacheRPC));
    rpcExecutors.push_back(new RPCExecutor<QueryRPC>(queryRPC));
    rpcExecutors.push_back(new RPCExecutor<ReleaseDataRPC>(releaseDataRPC));
    rpcExecutors.push_back(new RPCExecutor<OpenDatabaseRPC>(openDatabaseRPC));
    rpcExecutors.push_back(new RPCExecutor<DefineVirtualDatabaseRPC>(defineVirtualDatabaseRPC));
    rpcExecutors.push_back(new RPCExecutor<RenderRPC>(renderRPC));
    rpcExecutors.push_back(new RPCExecutor<SetWinAnnotAttsRPC>(setWinAnnotAttsRPC));
    rpcExecutors.push_back(new RPCExecutor<CloneNetworkRPC>(cloneNetworkRPC));

    // Hook up the expression list as an observed object.
    ParserInterface *p = ParserInterface::MakeParser(new EngineExprNodeFactory());
    ParsingExprList *l = new ParsingExprList(p);
    xfer->Add(l->GetList());

    // Hook up metadata and SIL to be send back to the viewer.
    // This is intended to only be used for simulations.
    metaData = new avtDatabaseMetaData;
    silAtts = new SILAttributes;
    xfer->Add(metaData);
    xfer->Add(silAtts);

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
    avtTerminatingSource::RegisterInitializeProgressCallback(
                                       Engine::EngineInitializeProgressCallback, NULL);
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
// ****************************************************************************

bool
Engine::ConnectViewer(int *argc, char **argv[])
{
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
    MPI_Bcast((void *)&shouldExit, 1, MPI_INT, 0, MPI_COMM_WORLD);
    noFatalExceptions = (shouldExit == 0);
#endif

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
// ****************************************************************************

void
Engine::PAR_EventLoop()
{
    PAR_StateBuffer  buf;
    BufferConnection conn;

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
            quitRPC->Write(conn);
            xfer->SetInputConnection(&conn);
            xfer->SetEnableReadHeader(false);
            xfer->Process();
        }
    }
    else
    {
        // Set the xfer object's input connection to be the buffer connection
        // that was declared at the top of this routine.
        xfer->SetInputConnection(&conn);

        // Non-UI Process
        while(!quitRPC->GetQuit() && noFatalExceptions)
        {
            // Reset the alarm
            ResetTimeout(timeout * 60);

            // Get state information from the UI process.
            MPI_Bcast((void *)&buf, 1, PAR_STATEBUFFER, 0, MPI_COMM_WORLD);

            // We have work to do, so cancel the alarm.
            int num_seconds_in_half_hour = 30*60;
            ResetTimeout(num_seconds_in_half_hour);

            // Add the state information to the connection.
            conn.Append((unsigned char *)buf.buffer, buf.nbytes);

            // Process the state information.
            xfer->Process();

            ResetTimeout(timeout * 60);
        }
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
// ****************************************************************************

bool
Engine::EventLoop()
{
    bool errFlag = false;

    // The application's main loop
    while(!quitRPC->GetQuit() && noFatalExceptions)
    {
        // Reset the timeout alarm
        ResetTimeout(timeout * 60);

        //
        // Block until the connection needs to be read. Then process its
        // new input.
        //
        if (xfer->GetInputConnection()->NeedsRead(true))
        {
            TRY
            {
                // We've got some work to do.  Disable the alarm.
                const int num_seconds_in_half_hour = 30*60;
                ResetTimeout(num_seconds_in_half_hour);

                // Process input.
                ProcessInput();

                ResetTimeout(timeout * 60);
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
// ****************************************************************************
void
Engine::ProcessCommandLine(int argc, char **argv)
{
    // process arguments.
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-forcestatic") == 0)
            LoadBalancer::ForceStatic();
        else if (strcmp(argv[i], "-forcedynamic") == 0)
            LoadBalancer::ForceDynamic();
        else if (strcmp(argv[i], "-timing") == 0)
            visitTimer->Enable();
        else if (strcmp(argv[i], "-timeout") == 0)
        {
            timeout = atol(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "-dump") == 0)
        {
            avtStreamer::DebugDump(true);
            netmgr->DumpRenders();
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
    }
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
    debug1 << "ENGINE exited due to an inactivity timeout of "
           << Engine::Instance()->timeout << " minutes." << endl;
#ifdef PARALLEL
    PAR_Exit();
#else
    exit(0);
#endif
    Init::Finalize();
}

// ****************************************************************************
//  Function: NewHandler
//
//  Purpose: Issue warning message when memory has run out
//
//  Programmer: Mark C. Miller 
//  Creation:   Tue Jun 29 17:34:19 PDT 2004
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
    MPI_Abort(MPI_COMM_WORLD, 18);
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
// ****************************************************************************
void
Engine::WriteData(NonBlockingRPC *rpc, avtDataObjectWriter_p &writer,
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
    // to recieve the dummied-up data tree from each processor, regardless of
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

        currentCellCount = (int) (ui_dob->GetNumberOfCells(polysOnly) * cellCountMultiplier);

        // test if we've exceeded the scalable threshold already with proc 0's output
        if (currentTotalGlobalCellCount +
            currentCellCount > scalableThreshold)
        {
            debug5 << "exceeded scalable threshold of " << scalableThreshold << endl;
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
                    mpiCellCountTag, MPI_COMM_WORLD, &stat);

                mpiSource = stat.MPI_SOURCE;

                debug5 << "recievied the \"num cells I have\" (=" << proc_i_localCellCount
                       << ") message from processor " << mpiSource << endl;

                // accumulate this processors cell count in the total for this network
                currentCellCount += proc_i_localCellCount;

                // test if we've exceeded the scalable threshold
                if (currentTotalGlobalCellCount +
                    currentCellCount > scalableThreshold)
                {
                    if (!thresholdExceeded)
                        debug5 << "exceeded scalable threshold of " << scalableThreshold << endl;
                    shouldGetData = sendDataAnyway;
                    thresholdExceeded = true; 
                }

                // tell source processor whether or not to send data with
                // the "should send data" message
                MPI_Send(&shouldGetData, 1, MPI_INT, mpiSource, 
                    mpiSendDataTag, MPI_COMM_WORLD);
                debug5 << "told processor " << mpiSource << (shouldGetData==1?" to":" NOT to")
                       << " send data" << endl;

                if (shouldGetData)
                {
                    MPI_Recv(&size, 1, MPI_INT, mpiSource, 
                             mpiDataObjSizeTag, MPI_COMM_WORLD, &stat);
                    debug5 << "recieving size=" << size << endl;

                    debug5 << "receiving " << size << " bytes from MPI_SOURCE "
                           << mpiSource << endl;

                    char *str = new char[size];
                    MPI_Recv(str, size, MPI_CHAR, mpiSource, 
                             mpiDataObjDataTag, MPI_COMM_WORLD, &stat);
                    debug5 << "recieving data" << endl;
    
                    // The data object reader will delete the string.
                    avtDataObjectReader *avtreader = new avtDataObjectReader;
                    avtreader->Read(size, str);
                    avtDataObject_p proc_i_dob = avtreader->GetOutput();

                    // We can't tell the reader to read (Update) unless we tell it
                    // what we want it to read.  Fortunately, we can just ask it
                    // for a general specification.
                    avtTerminatingSource *src = proc_i_dob->GetTerminatingSource();
                    avtPipelineSpecification_p spec
                        = src->GetGeneralPipelineSpecification();
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
            int numCells = (int) (writer->GetInput()->GetNumberOfCells(polysOnly) *
                                                      cellCountMultiplier);
            debug5 << "sending \"num cells I have\" message (=" << numCells << ")" << endl;
            MPI_Send(&numCells, 1, MPI_INT, 0, mpiCellCountTag, MPI_COMM_WORLD);

            // recv the "should send data" message from proc 0
            MPI_Recv(&shouldSendData, 1, MPI_INT, 0, mpiSendDataTag, MPI_COMM_WORLD, &stat);

            if (shouldSendData)
            {
               debug5 << "sending size=" << size << endl; 
               MPI_Send(&size, 1, MPI_INT, 0, mpiDataObjSizeTag, MPI_COMM_WORLD);
               debug5 << "sending " << size << " bytes to proc 0" << endl;
               debug5 << "sending data" << endl; 
               MPI_Send(str, size, MPI_CHAR, 0, mpiDataObjDataTag, MPI_COMM_WORLD);
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
    MPI_Bcast(tmp, 2, MPI_INT, 0, MPI_COMM_WORLD);
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
        // Send a second stage for the RPC.
        rpc->SendStatus(0,
                        rpc->GetCurStageNum(),
                        "Transferring Data Set",
                        rpc->GetMaxStageNum());

        writer->SetDestinationFormat(destinationFormat);
        avtDataObjectString  do_str;
        writer->Write(do_str);

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
        MPI_Iprobe(0, INTERRUPT_MESSAGE_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag)
        {
            char buf[1];
            MPI_Recv(buf, 1, MPI_CHAR, 0, INTERRUPT_MESSAGE_TAG, MPI_COMM_WORLD, &status);
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
// ****************************************************************************

void
Engine::EngineUpdateProgressCallback(void *data, const char *type, const char *desc,
                             int cur, int total)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineUpdateProgressCallback called with no RPC set.");

    if (total == 0)
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
        rpc->SendStatus(100. * float(cur)/float(total),
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
// ****************************************************************************

void
Engine::EngineWarningCallback(void *data, const char *msg)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineInitializeProgressCallback called with no RPC set.");

    rpc->SendWarning(msg);
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
    metaData->Notify();
    silAtts->SelectAll();
    silAtts->Notify();
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
// ****************************************************************************
void
Engine::SimulationTimeStepChanged()
{
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
Engine::Disconnect()
{
    delete instance;
    instance = NULL;
}
