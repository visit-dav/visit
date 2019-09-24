// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_H
#define ENGINE_H
#include <EngineBase.h>

#include <EnginePropertiesRPC.h>

// RPCs
class ApplyOperatorRPC;
class ClearCacheRPC;
class CloneNetworkRPC;
class ConstructDataBinningRPC;
class DefineVirtualDatabaseRPC;
class ExecuteRPC;
class ExportDatabaseRPC;
class KeepAliveRPC;
class MakePlotRPC;
class NamedSelectionRPC;
class OpenDatabaseRPC;
class PickRPC;
class QueryRPC;
class DBQueryOverTimeRPC;
class QueryParametersRPC;
class QuitRPC;
class ReadRPC;
class ReleaseDataRPC;
class RenderRPC;
class SetWinAnnotAttsRPC;
class SimulationCommand;
class StartPickRPC;
class StartQueryRPC;
class UpdatePlotAttsRPC;
class UseNetworkRPC;
class ProcInfoRPC;
class SimulationCommandRPC;
class SetEFileOpenOptionsRPC;
class SetPrecisionTypeRPC;
class SetBackendTypeRPC;
class SetRemoveDuplicateNodesRPC;
class LaunchRPC;
class EngineState;

// Other classes
class avtDatabaseMetaData;
class Connection;
class LoadBalancer;
class MPIXfer;
class NetworkManager;
class NonBlockingRPC;
class Observer;
class ParentProcess;
class ParsingExprList;
class ProcessAttributes;
class ViewerRemoteProcess;
class VisItDisplay;
class Xfer;

#include <vector>
#include <avtDataObjectWriter.h>
#include <BufferConnection.h>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class:  Engine
//
//  Purpose:
//    VisIt Computation Engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Jan 14 10:38:48 PST 2004
//    Made some changes so that the engine can be used from within an
//    external event loop.
//
//    Brad Whitlock, Fri Mar 12 11:08:21 PDT 2004
//    Added KeepAliveRPC and SendKeepAliveReply.
//
//    Kathleen Bonnell, Wed Mar 31 16:53:03 PST 2004
//    Added CloneNetworkRPC.
//
//    Mark C. Miller, Mon May 24 18:36:13 PDT 2004
//    Added args to WriteData method to support checking if SR threshold is
//    exceeded
//
//    Hank Childs, Tue Jun  1 14:02:44 PDT 2004
//    Added Finalize method.
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added NewHandler
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added argument for cellCountMultiplier to WriteData
//
//    Jeremy Meredith, Wed Aug 25 11:58:44 PDT 2004
//    Added methods needed for Simulation support.
//
//    Jeremy Meredith, Mon Nov  1 17:17:59 PST 2004
//    Added a parallel ProcessInput for parallel Simulation support.  Made
//    some other small changes necessary for that to happen.
//
//    Hank Childs, Mon Feb 28 17:03:06 PST 2005
//    Added StartQueryRPC.
//
//    Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//    Added GetProcessAttributes
//
//    Hank Childs, Tue Mar 22 09:37:58 PST 2005
//    Remove unused data members that date back to old expression calls.
//
//    Jeremy Meredith, Mon Apr  4 15:59:49 PDT 2005
//    Added methods to control a simulation from VisIt.
//
//    Jeremy Meredith, Wed May 18 16:50:59 PDT 2005
//    Allow disabling of signal handlers.
//
//    Hank Childs, Thu May 26 11:50:45 PDT 2005
//    Add ExportDatabase.
//
//    Mark C. Miller, Wed Dec 14 17:19:38 PST 2005
//    Added compression bool to WriteData
//
//    Hank Childs, Thu Jan  5 14:24:06 PST 2006
//    Added a Boolean for whether or not we are using "-dump".
//
//    Hank Childs, Mon Feb 13 22:25:04 PST 2006
//    Added ConstructDDF.
//
//    Hank Childs, Tue Sep  5 10:45:13 PDT 2006
//    Make ResetTimeout be a public function, since query over time and
//    some of the line scan algorithms may take more than ten minutes.
//
//    Brad Whitlock, Thu Jan 25 15:26:45 PST 2007
//    Added SimulationInitiateCommand.
//
//    Jeremy Meredith, Wed Jan 23 16:50:36 EST 2008
//    Added setEFileOpenOptionsRPC.
//
//    Cyrus Harrison, Tue Feb 19 08:42:51 PST 2008
//    Removed shouldDoDashDump (flag now contained in avtDebugDumpOptions)
//
//    Brad Whitlock, Tue Jun 24 15:20:05 PDT 2008
//    Made plugin managers be members of the class instead of singletons.
//
//    Tom Fogal, Fri Jul 11 12:00:29 EDT 2008
//    Added a flag to determine whether or not we should use the IceT network
//    manager.  This should be set if we're given the `-icet' command line
//    parameter.
//
//    Tom Fogal, Tue Aug  5 14:26:03 EDT 2008
//    Added a string to store arguments to the X server.
//
//    Tom Fogal, Mon Aug 11 11:39:07 EDT 2008
//    Instance variable to store how many X servers we should start up.
//
//    Hank Childs, Thu Jan 29 11:12:04 PST 2009
//    Add data member namedSelectionRPC.
//
//    Tom Fogal, Mon Sep  1 12:47:57 EDT 2008
//    Instance variable to store what kind of display we'll render with, and
//    made SetupDisplay a method.
//
//    Brad Whitlock, Thu Feb 26 14:03:25 PST 2009
//    I added Message and Error so we can send messages to the simulation user.
//
//    Brad Whitlock, Mon Mar  2 16:11:36 PST 2009
//    I added a SaveWindow method.
//
//    Brad Whitlock, Fri Mar 27 11:32:32 PDT 2009
//    I changed the command callback mechanism so it only accepts string
//    arguments and it also accepts user-provided callback data.
//
//    Brad Whitlock, Thu Apr  9 11:55:50 PDT 2009
//    I made it possible to reverse launch the viewer.
//
//    Brad Whitlock, Thu Apr 23 12:08:17 PDT 2009
//    I added simulationPluginsEnabled.
//
//    Tom Fogal, Tue Jun 30 21:00:37 MDT 2009
//    Adapted for symbol visibility.
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
//    Tom Fogal, Wed May 26 09:22:43 MDT 2010
//    Add state for launching X servers.
//
//    Kathleen Biagas, Fri Jul 15 11:08:14 PDT 2011
//    Added QueryPrametersRPC.
//
//    Brad Whitlock, Mon Nov 28 16:27:21 PST 2011
//    I added LaunchRPC.
//
//    Dave Pugmire, Wed Apr 18 09:05:40 EDT 2012
//    Add alarmEnabled flag. Setting alarm(0) is not disabling the alarm.
//
//    Brad Whitlock, Fri Sep 28 09:15:46 PDT 2012
//    I added InitializeCompute.
//
//    Brad Whitlock, Wed Sep 17 18:21:40 PDT 2014
//    Lots of changes to support in situ.
//
// ****************************************************************************

class ENGINE_MAIN_API Engine : public EngineBase
{
  public:
    Engine();
    virtual ~Engine();

    // Initialization routines
    void            Initialize(int *argc, char **argv[], bool sigs);
    bool            ConnectViewer(int *argc, char **argv[]);
    void            SetUpViewerInterface(int *argc, char **argv[]);
    void            InitializeCompute();
    void            Finalize(void);

    // Needed for simulations
    void            EnableSimulationPlugins();
    void            PopulateSimulationMetaData(const std::string &db,
                                               const std::string &fmt);
    virtual void    SimulationTimeStepChanged();
    virtual void    SimulationInitiateCommand(const std::string &);
    void            SetSimulationCommandCallback(void(*)(const char*, const char*, void*), void*);
    virtual void    ExecuteSimulationCommand(const std::string&,
                                             const std::string&);
    static void     DisconnectSimulation();
    void            Message(const std::string &msg);
    void            Error(const std::string &msg);

    // Two event loops
    bool            EventLoop();
    void            PAR_EventLoop();

    // Get the network manager
    NetworkManager *GetNetMgr() { return netmgr; }

    // Methods needed for an external event loop
    int             GetInputSocket();
    void            ProcessInput();
    void            PAR_ProcessInput();

    // Method to write data back to the viewer
    bool            GatherData(avtDataObjectWriter_p &writer,
                               bool  useCompression,
                               bool  respondWithNull, 
                               int   scalableThreshold, 
                               int   currentTotalGlobalCellCount, 
                               float cellCountMultiplier,
                               void (*statusCB)(int,const char*,void*),
                               void  *statusDBData,
                               // outputs
                               void (*writeCB)(avtDataObjectString &,void*),
                               void *writeCBData,  
                               std::string         &errMessage,
                               bool               *scalableThresholdExceeded,
                               int                *currentNetworkGlobalCellCount);
    void            WriteByteStreamToSocket(avtDataObjectString &do_str);
    void            SendKeepAliveReply();

    // Tell the engine whether or not fatal exceptions have occurred
    void            SetNoFatalExceptions(bool nf) {noFatalExceptions = nf;}

    // Method to get unix process information
    ProcessAttributes *GetProcessAttributes();

    // Method to get engine properties.
    EngineProperties GetEngineProperties();

    // Method to launch other programs.
    void LaunchProcess(const stringVector &args);

    // Various callbacks
    static bool     EngineAbortCallback(void *);
    static bool     EngineAbortCallbackParallel(void*, bool);
    static void     EngineInitializeProgressCallback(void *, int);
    static void     EngineUpdateProgressCallback(void*, const char*,
                                                 const char*, int,int);
    static void     EngineWarningCallback(void *, const char *);
    void            ResetTimeout(int timeout);
    void            SetOverrideTimeout(int mins)
                    {
                        overrideTimeoutEnabled = true;
                        overrideTimeoutMins = mins; 
                    }
    bool            IsIdleTimeoutEnabled(void) { return idleTimeoutEnabled; }

    // Internal methods
  protected:
    void            ProcessCommandLine(int argc, char *argv[]);
    bool            ReverseLaunchViewer(int *argc, char **argv[]);
    void            ExtractViewerArguments(int *argc, char **argv[]);

    virtual void    CreatePluginManagers();

    static void     AlarmHandler(int signal);
    static void     NewHandler(void);

  private:
    void            SetupDisplay();

  protected:
    // The Viewer
    ParentProcess       *viewerP;       // Used when the viewer launches engine
    ViewerRemoteProcess *viewer;        // Used when engine launches viewer
    stringVector         viewerArgs;    // "
    bool                 reverseLaunch;

    // The destination machine's type representation.
    TypeRepresentation destinationFormat;

    // Here's the network manager!
    NetworkManager    *netmgr;
    std::string        pluginDir;
    bool               simulationPluginsEnabled;

    // The connection
    Connection        *vtkConnection;

    // Flag for whether there were any fatal exceptions
    bool               noFatalExceptions;

    // Current timeouts in minutes.
    int                idleTimeoutMins;
    int                executionTimeoutMins;
    bool               idleTimeoutEnabled;
    int                overrideTimeoutMins;
    bool               overrideTimeoutEnabled;

    // The load balancer
    LoadBalancer       *lb;


    // The Xfer object
#ifdef PARALLEL
    MPIXfer            *xfer;
    BufferConnection    par_conn;
#else
    Xfer               *xfer;
#endif

    // The list of all RPC executors (needed to clean up memory)
    std::vector<Observer*> rpcExecutors;

    // The RPCs
    QuitRPC                  *quitRPC;
    KeepAliveRPC             *keepAliveRPC;
    EngineState              *enginestate;

#ifdef DEBUG_MEMORY_LEAKS
    ParsingExprList          *parsingExprList;
#endif

    // The metadata, filename, format, control data for a simulation
    std::string               filename;
    std::string               format;
    Xfer                     *simxfer;
    Connection               *simConnection;
    avtDatabaseMetaData      *metaData;
    SILAttributes            *silAtts;
    SimulationCommand        *commandFromSim;
    void                    (*simulationCommandCallback)(const char*,const char*,void*);
    void                     *simulationCommandCallbackData;

    // unix process attributes
    ProcessAttributes        *procAtts;

 private:
    bool                      useIceT;
    std::string               X_Args;
    std::string               X_Display;
    size_t                    nDisplays;
    VisItDisplay             *renderingDisplay;
    bool                      launchXServers;
    bool                      alarmEnabled;
};

#endif
