// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            ViewerEngineManager.h                          //
// ************************************************************************* //

#ifndef VIEWER_ENGINE_MANAGER_H
#define VIEWER_ENGINE_MANAGER_H
#include <viewer_exports.h>
#include <ViewerEngineManagerInterface.h>

// ****************************************************************************
//  Class: ViewerEngineManager
//
//  Purpose:
//      ViewerEngineManager implements the interface using
//      methods in the ViewerEngineManagerImplementation class.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Sep  9 16:51:11 PDT 2014
//
//  Modifications:
//    Kathleen Biagas, Mon Dec 22 10:12:05 PST 2014
//    Added UpdateRemoveDuplicateNodes.
//
//    Kathleen Biagas, Fri Apr 23 2021
//    ExportDatabases has new signature with return atts.
//
// ****************************************************************************

class VIEWER_API ViewerEngineManager : public ViewerEngineManagerInterface
{
public:
    ViewerEngineManager();
    virtual ~ViewerEngineManager();


    // *********************************************************************
    // ENGINE MANAGEMENT
    // *********************************************************************
    virtual bool CreateEngine(const EngineKey &ek,
                      const stringVector &arguments,
                      bool  skipChooser=false,
                      int   numRestarts=-1,
                      bool  reverseLaunch = false);

    virtual bool CreateEngineEx(const EngineKey &ek,
                        const stringVector &arguments,
                        bool  skipChooser,
                        int   numRestarts,
                        bool  reverseLaunch,
                        ViewerConnectionProgress *progress);
    virtual void CloseEngines();
    virtual void CloseEngine(const EngineKey &ek);
    virtual void InterruptEngine(const EngineKey &ek);
    virtual void SendKeepAlives();
    virtual bool EngineExists(const EngineKey &ek);
    virtual void ClearCacheForAllEngines();
    virtual void UpdateEngineList();
public:
    // *********************************************************************
    // SIMULATION METHODS
    // *********************************************************************
    virtual bool                 ConnectSim(const EngineKey &ek,
                                     const stringVector &arguments,
                                     const std::string &simHost,
                                     int simPort,
                                     const std::string &simSecurityKey);

    virtual void                 SendSimulationCommand(const EngineKey &ek,
                                     const std::string &command,
                                     const std::string &argument);

    virtual int                  GetWriteSocket(const EngineKey &ek);
    virtual void                 ReadDataAndProcess(const EngineKey &ek);
    virtual avtDatabaseMetaData *GetSimulationMetaData(const EngineKey &ek);
    virtual SILAttributes       *GetSimulationSILAtts(const EngineKey &ek);
    virtual SimulationCommand   *GetCommandFromSimulation(const EngineKey &ek);

public:
    // *********************************************************************
    // Engine RPCs
    // *********************************************************************
    virtual bool OpenDatabase(const EngineKey &ek, const std::string &format, 
                              const std::string &filename, int time=0);

    virtual bool DefineVirtualDatabase(const EngineKey &ek,
                                       const std::string &format,
                                       const std::string &dbName,
                                       const std::string &path, 
                                       const stringVector &files,
                                       int time=0);

    virtual bool ReadDataObject(const EngineKey &ek,
                        const std::string &format,
                        const std::string &filename,
                        const std::string &var, const int ts,
                        avtSILRestriction_p,
                        const MaterialAttributes&,
                        const ExpressionList &,
                        const MeshManagementAttributes &,
                        bool treatAllDbsAsTimeVarying,
                        bool ignoreExtents,
                        const std::string &selName,
                        int windowID);

    virtual bool ApplyOperator(const EngineKey &ek,
                               const std::string &name,
                               const AttributeSubject *atts);

    virtual bool MakePlot(const EngineKey &ek,
                          const std::string &plotName,
                          const std::string &pluginID,
                          const AttributeSubject *atts,
                          const std::vector<double> &,
                          int winID, int *networkId);

    virtual bool UpdatePlotAttributes(const EngineKey &ek,
                                      const std::string &name,
                                      int id, const AttributeSubject *atts);

    virtual bool UseNetwork(const EngineKey &ek, int networkId);

    virtual bool Execute(const EngineKey &ek, avtDataObjectReader_p &rdr,
                         bool replyWithNullData,
                         void (*waitCB)(void*), void *waitCBData);

    virtual int Render(const EngineKey &ek, avtImage_p &img,
                        avtImageType imgT, bool sendZBuffer,
                        const intVector &networkIds, 
                        int annotMode, int windowID, bool leftEye,
                        void (*waitCB)(void *), void *waitCBData);

    // query methods
    virtual bool Pick(const EngineKey &ek, const int nid, int wid,
                      const PickAttributes *atts, PickAttributes &retAtts);

    virtual bool StartPick(const EngineKey &ek, const bool forZones,
                           const bool flag, const int nid);

    virtual bool StartQuery(const EngineKey &ek, const bool flag, const int nid);

    virtual bool SetWinAnnotAtts(const EngineKey &ek,
                                 const WindowAttributes *wa,
                                 const AnnotationAttributes *aa,
                                 const AnnotationObjectList *ao,
                                 const std::string extStr,
                                 const VisualCueList *visCues,
                                 const int *frameAndState,
                                 const double *viewExtents,
                                 const std::string ctName,
                                 const int winID);

    virtual bool Query(const EngineKey &ek,
                       const intVector &networkIds, 
                       const QueryAttributes *atts,
                       QueryAttributes &retAtts);

    virtual bool GetQueryParameters(const EngineKey &ek,
                                    const std::string &qname,
                                    std::string *params);

    // network management
    virtual bool ClearCache(const EngineKey &ek, const std::string &dbName);
    virtual bool GetProcInfo(const EngineKey &ek, ProcessAttributes &retAtts);
    virtual bool ReleaseData(const EngineKey &ek, int id);
    virtual bool CloneNetwork(const EngineKey &ek, int id, 
                              const QueryOverTimeAttributes *qatts);

    // selections
    virtual bool CreateNamedSelection(const EngineKey &ek, 
                                      int id,
                                      const SelectionProperties &props,
                                      SelectionSummary &summary);

    virtual bool UpdateNamedSelection(const EngineKey &ek, 
                                      int id,
                                      const SelectionProperties &, 
                                      bool allowCache,
                                      SelectionSummary &summary);

    virtual bool DeleteNamedSelection(const EngineKey &ek,
                                      const std::string &selName);

    virtual bool LoadNamedSelection(const EngineKey &ek,
                                    const std::string &selName);

    virtual bool SaveNamedSelection(const EngineKey &ek,
                                    const std::string &selName);

    virtual bool ExportDatabases(const EngineKey &ek,
                                 const intVector &ids,
                                 const ExportDBAttributes *expAtts,
                                 const std::string &timeSuffix,
                                 ExportDBAttributes &retAtts);

    virtual bool ConstructDataBinning(const EngineKey &ek, int id);

    virtual bool UpdateExpressions(const EngineKey &ek, const ExpressionList &eL);
    virtual void UpdateDefaultFileOpenOptions(FileOpenOptions*);
    virtual void UpdatePrecisionType(const int);
    virtual void UpdateBackendType(const int);
    virtual void UpdateRemoveDuplicateNodes(const bool);

    virtual bool LaunchProcess(const EngineKey &ek, const stringVector &args);

public:
    virtual void CreateNode(DataNode *, bool detailed);
    virtual void SetFromNode(DataNode *, const std::string &);
};

#endif
