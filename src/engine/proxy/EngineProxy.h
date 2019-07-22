// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                EngineProxy.h                              //
// ************************************************************************* //

#ifndef ENGINE_PROXY_H
#define ENGINE_PROXY_H
#include <engine_proxy_exports.h>

#include <RemoteProxyBase.h>
#include <EngineState.h>
#include <EngineMethods.h>
#include <avtDataObjectReader.h>
#include <avtDatabaseMetaData.h>
#include <SimulationCommand.h>

#include <vectortypes.h>
#include <string>
#include <vector>

class StatusAttributes;
class ConstructDataBinningAttributes;
class ExportDBAttributes;
class ParentProcess;

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
//    Eric Brugger, Fri Mar 19 15:11:41 PST 2004
//    I modified the MakePlot rpc to pass the data limits to the engine.
//
//    Mark C. Miller, Mon Mar 29 15:01:58 PST 2004
//    Added new bool arg for controlling 3D annoations in Render method
//
//    Kathleen Bonnell, Wed Mar 31 17:23:01 PST 2004 
//    Added CloneNetworkRPC. 
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added argument for extents type string to SetWinAnnotAtts
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added waitCB and cbData args to Render method
//
//    Mark C. Miller, Tue May 25 17:25:55 PDT 2004
//    Added AnnotationObjectList arg to SetWinAnnotAtts
//
//    Kathleen Bonnell, Wed Jun  2 09:43:07 PDT 2004 
//    Added another bool arg to StartPick.
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added VisualCueList argument to SetWinAnnotAtts
//
//    Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//    Removed waitCB and cbData arguments from Render method
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added argument for frame and state to SetWinAnnotAtts
// 
//    Jeremy Meredith, Tue Aug 24 22:30:21 PDT 2004
//    Added methods and data needed for simulations.
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added explicit view extents to SetWinAnnotAtts
//    Changed bool arg for 3D annotations to an integer mode in Render
//
//    Mark C. Miller, Tue Oct 19 19:51:43 PDT 2004
//    Added arg to SetWinAnnotAtts for changed color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id to various methods to support multi-window SR
//
//    Brad Whitlock, Fri Feb 18 09:35:11 PDT 2005
//    I added ExpressionList to the call to ReadDataObject so we can pass
//    in the right list of expressions.
//
//    Hank Childs, Mon Feb 28 17:22:13 PST 2005
//    Added StartQuery.
//
//    Kathleen Bonnell, Tue Mar  1 11:20:15 PST 2005 
//    Added UpdateExpressions. 
//
//    Mark C. Miller, Tue Mar  8 17:59:40 PST 2005
//    Added GetProcInfo
//
//    Jeremy Meredith, Mon Mar 21 08:50:09 PST 2005
//    Added ExecuteSimulationControlCommand methods.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes to ReadDataObject
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added bool arg (leftEye) to Render method to support stereo SR
//
//    Brad Whitlock, Thu Jan 25 13:51:10 PST 2007
//    Added a SimulationCommand object so the simulation can send simulation
//    commands back to the viewer.
//
//    Brad Whitlock, Wed Mar 21 22:54:33 PST 2007
//    Added plotName to MakePlot.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//    Added two bool args to OpenDatabase and DefineVirtualDatabase to support 
//    turning on/off creation of MeshQuality and TimeDerivative expressions.
//
//    Jeremy Meredith, Wed Jan 23 16:10:37 EST 2008
//    Added ability to notify engines of the file opening options for the
//    database reader plugins.
//
//    Mark C. Miller, Tue Jun 10 15:57:15 PDT 2008
//    Added support for ignoring extents during ReadDataObject/OpenDatabase
//
//    Hank Childs, Wed Jan 28 15:57:02 PST 2009
//    Add support for named selections.
//
//    Brad Whitlock, Thu Apr  9 15:02:51 PDT 2009
//    I added reverse launch support.
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
//    Brad Whitlock, Mon Aug 22 10:07:44 PDT 2011
//    I added a selection argument to ReadDataObject. I also removed
//    ApplyNamedSelection.
//
//    Kathleen Biagas, Fri Jul 15 11:34:11 PDT 2011
//    Added QueryParametersRPC.
//
//    Brad Whitlock, Wed Sep  7 14:29:35 PDT 2011
//    Added UpdateNamedSelection.
//
//    Eric Brugger, Mon Oct 31 10:33:28 PDT 2011
//    Added a window id to ReadDataObject.
//
//    Brad Whitlock, Mon Nov 28 16:22:35 PST 2011
//    I added a LaunchProcess method.
//
// ****************************************************************************

class ENGINE_PROXY_API EngineProxy : public RemoteProxyBase
{
public:
    EngineProxy(bool sim = false);
    virtual ~EngineProxy();

    virtual void Create(const MachineProfile &profile, 
                        ConnectCallback *connectCallback = 0, 
                        void *connectCallbackData = 0,
                        bool createAsThoughLocal = false);

    // This version of Create is specifically for reverse launch.
    void Connect(const stringVector &args);

    virtual void SendKeepAlive();

    virtual bool Parallel() const { return numProcs > 1; }
    virtual std::string GetComponentName() const;

    virtual void SetNumProcessors(int np) { numProcs = np; }
    virtual void SetNumNodes(int nn)      { numNodes = nn; }
    virtual void SetLoadBalancing(int lb) { loadBalancing = lb; }
    virtual int  NumProcessors() const    { return numProcs; }
    virtual int  NumNodes() const         { return numNodes; }
    virtual int  LoadBalancing() const    { return loadBalancing; }

    void Interrupt();

    // Needed for simulations to pass back metadata; may have expanded
    // functionality in the future
    int                      GetWriteSocket();
    void                     ReadDataAndProcess();
    avtDatabaseMetaData     *GetSimulationMetaData();
    SILAttributes           *GetSimulationSILAtts();
    SimulationCommand       *GetCommandFromSimulation();

    EngineState             *GetEngineState() { return state; }
    EngineMethods           *GetEngineMethods() { return methods; }

protected:
    virtual void             SetupComponentRPCs();
    void                     ExtractEngineInformation();
private:
    ParentProcess           *engineP;

    // Metadata, SIL published by a simulation
    Xfer                    *simxfer;
    avtDatabaseMetaData     *metaData;
    SILAttributes           *silAtts;
    SimulationCommand       *commandFromSim;

    // Information that can be queried about the engine.
    int                      numProcs;
    int                      numNodes;
    int                      loadBalancing;

    EngineState             *state;
    EngineMethods           *methods;
};

#endif
