#ifndef ENGINE_H
#define ENGINE_H

// RPCs
class ApplyNamedFunctionRPC;
class ApplyOperatorRPC;
class ClearCacheRPC;
class DefineVirtualDatabaseRPC;
class ExecuteRPC;
class MakePlotRPC;
class OpenDatabaseRPC;
class PickRPC;
class QueryRPC;
class QuitRPC;
class ReadRPC;
class ReleaseDataRPC;
class RenderRPC;
class SetFinalVariableNameRPC;
class SetWindowAttsRPC;
class StartPickRPC;
class UpdatePlotAttsRPC;
class UseNetworkRPC;

// Other classes
class Connection;
class LoadBalancer;
class MPIXfer;
class NetworkManager;
class NonBlockingRPC;
class Observer;
class Xfer;

#include <vector>
#include <avtDataObjectWriter.h>
#include <ParentProcess.h>

// ****************************************************************************
//  Class:  Engine
//
//  Purpose:
//    VisIt Computation Engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  9, 2003
//
// ****************************************************************************

class Engine
{
  public:
                   ~Engine();
    static Engine  *Instance();

    // Initialization routines
    void            Initialize(int *argc, char **argv[]);
    bool            ConnectViewer(int *argc, char **argv[]);
    void            SetUpViewerInterface(int *argc, char **argv[]);

    // Two event loops
    bool            EventLoop();
    void            PAR_EventLoop();

    // Get the network manager
    NetworkManager *GetNetMgr()                   { return netmgr; }

    // Method to write data back to the viewer
    void            WriteData(NonBlockingRPC *, avtDataObjectWriter_p &);

    // Tell the engine whether or not fatal exceptions have occurred
    void            SetNoFatalExceptions(bool nf) {noFatalExceptions = nf;}

    // Various callbacks
    static bool     EngineAbortCallback(void *);
    static bool     EngineAbortCallbackParallel(void*, bool);
    static void     EngineInitializeProgressCallback(void *, int);
    static void     EngineUpdateProgressCallback(void*, const char*,
                                                 const char*, int,int);
    static void     EngineWarningCallback(void *, const char *);

    // Internal methods
  protected:
                    Engine();
    void            ProcessCommandLine(int argc, char *argv[]);
    void            ProcessInput();
    void            ResetTimeout(int timeout);

    static void     AlarmHandler(int signal);

  protected:
    // The singleton object
    static Engine     *instance;

    // The Viewer
    ParentProcess      theViewer;

    // The destination machine's type representation.
    TypeRepresentation destinationFormat;

    // Here's the network manager!
    NetworkManager    *netmgr;

    // The connection
    Connection        *vtkConnection;

    // Flag for whether there were any fatal exceptions
    bool               noFatalExceptions;

    // Current Timeout
    int                timeout;

    // The load balancer
    LoadBalancer       *lb;

    // The Xfer object
#ifdef PARALLEL
    MPIXfer            *xfer;
#else
    Xfer               *xfer;
#endif

    // The list of all RPC executors (needed to clean up memory)
    std::vector<Observer*> rpcExecutors;

    // The RPCs
    QuitRPC                  *quitRPC;
    ReadRPC                  *readRPC;
    ApplyOperatorRPC         *applyOperatorRPC;
    ApplyNamedFunctionRPC    *applyNamedFunctionRPC;
    SetFinalVariableNameRPC  *setFinalVariableNameRPC;
    MakePlotRPC              *makePlotRPC;
    UseNetworkRPC            *useNetworkRPC;
    UpdatePlotAttsRPC        *updatePlotAttsRPC;
    SetWindowAttsRPC         *setWindowAttsRPC;
    PickRPC                  *pickRPC;
    StartPickRPC             *startPickRPC;
    ExecuteRPC               *executeRPC;
    ClearCacheRPC            *clearCacheRPC;
    QueryRPC                 *queryRPC;
    ReleaseDataRPC           *releaseDataRPC;
    OpenDatabaseRPC          *openDatabaseRPC;
    DefineVirtualDatabaseRPC *defineVirtualDatabaseRPC;
    RenderRPC                *renderRPC;
};

#endif
