#ifndef ENGINE_H
#define ENGINE_H

// RPCs
class ApplyNamedFunctionRPC;
class ApplyOperatorRPC;
class ClearCacheRPC;
class CloneNetworkRPC;
class DefineVirtualDatabaseRPC;
class ExecuteRPC;
class KeepAliveRPC;
class MakePlotRPC;
class OpenDatabaseRPC;
class PickRPC;
class QueryRPC;
class QuitRPC;
class ReadRPC;
class ReleaseDataRPC;
class RenderRPC;
class SetFinalVariableNameRPC;
class SetWinAnnotAttsRPC;
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

    // Methods needed for an external event loop
    int             GetInputSocket();
    void            ProcessInput();

    // Method to write data back to the viewer
    void            WriteData(NonBlockingRPC *, avtDataObjectWriter_p &,
                        bool respondWithNull=false, int scalableThresold=-1,
                        bool *scalableThresholdExceeded=0,
                        int currentTotalGlobalCellCount=0,
                        int* currentNetworkGlobalCellCount=0);
    void            SendKeepAliveReply();

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
    KeepAliveRPC             *keepAliveRPC;
    ReadRPC                  *readRPC;
    ApplyOperatorRPC         *applyOperatorRPC;
    ApplyNamedFunctionRPC    *applyNamedFunctionRPC;
    SetFinalVariableNameRPC  *setFinalVariableNameRPC;
    MakePlotRPC              *makePlotRPC;
    UseNetworkRPC            *useNetworkRPC;
    UpdatePlotAttsRPC        *updatePlotAttsRPC;
    PickRPC                  *pickRPC;
    StartPickRPC             *startPickRPC;
    ExecuteRPC               *executeRPC;
    ClearCacheRPC            *clearCacheRPC;
    QueryRPC                 *queryRPC;
    ReleaseDataRPC           *releaseDataRPC;
    OpenDatabaseRPC          *openDatabaseRPC;
    DefineVirtualDatabaseRPC *defineVirtualDatabaseRPC;
    RenderRPC                *renderRPC;
    SetWinAnnotAttsRPC       *setWinAnnotAttsRPC;
    CloneNetworkRPC          *cloneNetworkRPC;
};

#endif
