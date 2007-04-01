// ************************************************************************* //
//                                EngineProxy.h                              //
// ************************************************************************* //

#ifndef ENGINE_PROXY_H
#define ENGINE_PROXY_H
#include <engine_proxy_exports.h>

#include <RemoteProxyBase.h>
#include <ReadRPC.h>
#include <RenderRPC.h>
#include <ExecuteRPC.h>
#include <ApplyOperatorRPC.h>
#include <ClearCacheRPC.h>
#include <CloneNetworkRPC.h>
#include <DefineVirtualDatabaseRPC.h>
#include <MakePlotRPC.h>
#include <OpenDatabaseRPC.h>
#include <PickRPC.h>
#include <QueryRPC.h>
#include <ReleaseDataRPC.h>
#include <SetWinAnnotAttsRPC.h>
#include <StartPickRPC.h>
#include <UpdatePlotAttsRPC.h>
#include <UseNetworkRPC.h>
#include <ExpressionList.h>

#include <avtDataObjectReader.h>

#include <vectortypes.h>
#include <string>

class StatusAttributes;

// ****************************************************************************
//  Class: EngineProxy
//
//  Purpose:
//      EngineProxy creates a compute engine and provides methods
//      for retrieving data from it.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   July 26, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Wed Aug  9 14:39:33 PDT 2000
//    Added file descriptors for the state and vtk communication sockets.
//    Switched out plotAtts for plotRPC.
//
//    Jeremy Meredith, Thu Sep  7 13:06:10 PDT 2000
//    Added the new RPC types for doing network-style computation.
//
//    Jeremy Meredith, Mon Sep 11 13:03:43 PDT 2000
//    Added GetDataSet.
//
//    Jeremy Meredith, Mon Sep 25 12:56:05 PDT 2000
//    Overhauled interface.
//
//    Hank Childs, Thu Sep 28 22:14:46 PDT 2000
//    Made Execute return an avtDataset instead of a vtkDataSet.
//
//    Brad Whitlock, Fri Sep 29 19:16:52 PST 2000
//    I added the AddArgument method.
//
//    Kathleen Bonnell, Tue Oct 10 16:29:02 PDT 200 
//    I added the ApplyOnionPeel method, and OnionPeelRPC data member.
//
//    Hank Childs, Tue Oct 17 08:36:36 PDT 2000
//    Made Execute return an avtDataSetReader to prevent memory leaks.
//
//    Eric Brugger, Wed Oct 25 16:31:22 PDT 2000
//    I removed the argument "engineName" from the Create method.
//
//    Kathleen Bonnell, Fri Nov 17 16:33:40 PST 2000 
//    I added the PlotMaterial method, and MatPlotRPC data member.
//
//    Kathleen Bonnell, Fri Dec  1 14:51:50 PST 2000 
//    I added FilledBoundaryRPC.
//
//    Jeremy Meredith, Tue Dec 12 13:54:27 PST 2000
//    I added MaterialSelectRPC.
//
//    Hank Childs, Fri Dec 29 08:48:12 PST 2000
//    Made DataSetReaders become DataObjectReaders.
//
//    Hank Childs, Thu Jan 11 10:45:05 PST 2001
//    Added volume plot RPCs.  Removed unnecessary attribute members.
//
//    Kathleen Bonnell, Thu Feb 22 15:08:32 PST 2001 
//    Added contour plot RPC.
//
//    Jeremy Meredith, Thu Mar  1 13:58:55 PST 2001
//    Made ApplyMaterialSelect take a MaterialSelectAttributes instead of
//    an int vector.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Removed all plot and operator specific RPCs.
//    Created generic ApplyOperator and MakePlot RPCs. 
//
//    Brad Whitlock, Mon Apr 30 17:40:54 PST 2001
//    Added StatusAttributes that can be observed. Also added Status methods.
//
//    Jeremy Meredith, Wed Nov  7 17:34:23 PST 2001
//    Added UseNetwork and a UseNetworkRPC.  Made MakePlot return a network id.
//
//    Hank Childs, Thu Nov 29 15:29:24 PST 2001
//    Added UpdatePlotAttributes.
//
//    Kathleen Bonnell, Wed Dec 12 11:47:16 PST 2001
//    Added Pick, StartPick.
//
//    Sean Ahern, Wed Mar 20 20:39:17 PST 2002
//    Added ApplyUnaryOp.
//
//    Sean Ahern, Thu Apr 18 16:54:53 PDT 2002
//    Removed ApplyUnaryOp and added ApplyNamedFunction.
//
//    Brad Whitlock, Mon Mar 25 10:47:22 PDT 2002
//    I removed all the communication file descriptors.
//
//    Brad Whitlock, Tue May 7 16:09:30 PST 2002
//    I added callbacks to the Execute function.
//
//    Brad Whitlock, Tue Jul 30 13:02:53 PST 2002
//    I added ClearCacheRPC.
//
//    Kathleen Bonnell, Wed Sep 18 10:11:32 PDT 2002  
//    I added QueryRPC, ReleaseDataRPC.
//
//    Brad Whitlock, Thu Sep 26 17:28:09 PST 2002
//    I added a progress callback to be called while the engine launches.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options to ReadDataObject.
//
//    Sean Ahern, Thu Nov 21 20:12:50 PST 2002
//    Removed AddNamedFunction.
//
//    Brad Whitlock, Wed Nov 27 13:29:46 PST 2002
//    I added methods to allow certain parallel options to be specified
//    so the object can be queried for the values later.
//
//    Brad Whitlock, Tue Dec 10 14:19:32 PST 2002
//    I added an RPC that tells the engine to open a file.
//
//    Brad Whitlock, Tue Mar 25 13:51:31 PST 2003
//    I added an RPC that tells the engine to define a virtual database.
//
//    Brad Whitlock, Fri May 2 15:28:24 PST 2003
//    I made it inherit from RemoteProxyBase.
//
//    Jeremy Meredith, Thu Jun 26 10:31:39 PDT 2003
//    Made the numprocs/nodes/lb methods virtual.
//
//    Jeremy Meredith, Mon Sep 15 17:15:59 PDT 2003
//    Removed SetFinalVariableName.
//
//    Hank Childs, Thu Oct  2 16:20:17 PDT 2003
//    Allow for queries to involve multiple networks.
//
//    Hank Childs, Fri Mar  5 11:41:12 PST 2004
//    Add file format type to open database.
//
//    Brad Whitlock, Fri Mar 12 10:43:41 PDT 2004
//    I added an override of the base class's SendKeepAlive method.
//
//    Mark C. Miller, Mon Mar 29 15:01:58 PST 2004
//    Added new bool arg for controlling 3D annoations in Render method
//
//    Kathleen Bonnell, Wed Mar 31 17:23:01 PST 2004 
//    Added CloneNetworkRPC. 
//
// ****************************************************************************

class ENGINE_PROXY_API EngineProxy : public RemoteProxyBase
{
public:
    EngineProxy();
    virtual ~EngineProxy();

    virtual void SendKeepAlive();

    virtual bool Parallel() const { return numProcs > 1; };
    virtual std::string GetComponentName() const;

    virtual void SetNumProcessors(int np) { numProcs = np; };
    virtual void SetNumNodes(int nn)      { numNodes = nn; };
    virtual void SetLoadBalancing(int lb) { loadBalancing = lb; };
    virtual int  NumProcessors() const    { return numProcs; };
    virtual int  NumNodes() const         { return numNodes; };
    virtual int  LoadBalancing() const    { return loadBalancing; };

    StatusAttributes        *GetStatusAttributes() const;

    // RPCs to access functionality on the engine.
    void                     OpenDatabase(const std::string &, 
                                          const std::string &, int = 0);
    void                     DefineVirtualDatabase(const std::string &,
                                                   const std::string &,
                                                   const std::string &,
                                                   const stringVector &,
                                                   int = 0);
    void                     ReadDataObject(const std::string&,
                                            const std::string&,
                                            const std::string&, const int,
                                            avtSILRestriction_p,
                                            const MaterialAttributes&);
    void                     ApplyOperator(const std::string&, 
                                           const AttributeSubject*);
    void                     ApplyNamedFunction(const std::string &name, int nargs);
    int                      MakePlot(const std::string&, 
                                      const AttributeSubject*);

    void                     UseNetwork(int);
    void                     UpdatePlotAttributes(const std::string &, int,
                                                  const AttributeSubject*);
    void                     Pick(const int, const PickAttributes *,
                                  PickAttributes &);
    void                     StartPick(const bool, const int);

    void                     SetWinAnnotAtts(const WindowAttributes*,
                                             const AnnotationAttributes*);
    avtDataObjectReader_p    Render(bool, const intVector&, bool);

    avtDataObjectReader_p    Execute(bool, void (*waitCB)(void *), void *cbData);

    void                     ClearCache();
    void                     ClearCache(const std::string &);

    void                     Interrupt();

    void                     Query(const std::vector<int> &,
                                   const QueryAttributes *, 
                                   QueryAttributes &);

    void                     ReleaseData(const int);
    void                     CloneNetwork(const int, const QueryOverTimeAttributes *);

protected:
    virtual void             SetupComponentRPCs();
    void                     ExtractEngineInformation();
    void                     Status(const char *message);
    void                     Status(int percent, int curStage,
                                    const std::string &curStageName,
                                    int maxStage);
    void                     ClearStatus();
    void                     Warning(const char *message);
private:
    ReadRPC                  readRPC;
    ApplyOperatorRPC         applyOperatorRPC;
    MakePlotRPC              makePlotRPC;
    UseNetworkRPC            useNetworkRPC;
    UpdatePlotAttsRPC        updatePlotAttsRPC;
    ExecuteRPC               executeRPC;
    PickRPC                  pickRPC;
    StartPickRPC             startPickRPC;
    ClearCacheRPC            clearCacheRPC;
    QueryRPC                 queryRPC;
    ReleaseDataRPC           releaseDataRPC;
    OpenDatabaseRPC          openDatabaseRPC;
    DefineVirtualDatabaseRPC defineVirtualDatabaseRPC;
    RenderRPC                renderRPC;
    SetWinAnnotAttsRPC       setWinAnnotAttsRPC;
    ExpressionList           exprList;
    CloneNetworkRPC          cloneNetworkRPC;

    // For indicating status.
    StatusAttributes        *statusAtts;

    // Information that can be queried about the engine.
    int                      numProcs;
    int                      numNodes;
    int                      loadBalancing;
};

#endif
