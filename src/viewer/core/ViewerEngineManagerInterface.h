// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            ViewerEngineManagerInterface.h                 //
// ************************************************************************* //

#ifndef VIEWER_ENGINE_MANAGER_INTERFACE_H
#define VIEWER_ENGINE_MANAGER_INTERFACE_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <avtDataObjectReader.h>
#include <avtImage.h>
#include <avtImageType.h>
#include <vectortypes.h>
#include <avtSILRestriction.h>

class avtDatabaseMetaData;
class AnnotationAttributes;
class AnnotationObjectList;
class AttributeSubject;
class EngineKey;
class ExpressionList;
class ExportDBAttributes;
class FileOpenOptions;
class MaterialAttributes;
class MeshManagementAttributes;
class PickAttributes;
class ProcessAttributes;
class QueryAttributes;
class QueryOverTimeAttributes;
class SelectionProperties;
class SelectionSummary;
class SimulationCommand;
class ViewerConnectionProgress;
class VisualCueList;
class WindowAttributes;

// ****************************************************************************
//  Class: ViewerEngineManagerInterface
//
//  Purpose:
//      ViewerEngineManagerInterface provides an API for creating engines and
//      calling their methods.
//
//  Notes: Keep this class free of implementation.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Sep  9 16:51:11 PDT 2014
//
//  Modifications:
//    Kathleen Biagas, Mon Dec 22 10:22:11 PST 2014
//    Add UpdateRemoveDuplicateNodes.
//
//    Kathleen Biagas, Fri Apr 23 2021
//    ExportDatabases has new signature with return atts.
//
// ****************************************************************************

class VIEWERCORE_API ViewerEngineManagerInterface : public ViewerBase
{
public:
    ViewerEngineManagerInterface();
    virtual ~ViewerEngineManagerInterface();

    // *********************************************************************
    // ENGINE MANAGEMENT
    // *********************************************************************
    virtual bool CreateEngine(const EngineKey &ek,
                      const stringVector &arguments,
                      bool  skipChooser=false,
                      int   numRestarts=-1,
                      bool  reverseLaunch = false) = 0;

    virtual bool CreateEngineEx(const EngineKey &ek,
                        const stringVector &arguments,
                        bool  skipChooser,
                        int   numRestarts,
                        bool  reverseLaunch,
                        ViewerConnectionProgress *progress) = 0;
    virtual void CloseEngines() = 0;
    virtual void CloseEngine(const EngineKey &ek) = 0;
    virtual void InterruptEngine(const EngineKey &ek) = 0;
    virtual void SendKeepAlives() = 0;
    virtual bool EngineExists(const EngineKey &ek) = 0;
    virtual void ClearCacheForAllEngines() = 0;
    virtual void UpdateEngineList() = 0;
public:
    // *********************************************************************
    // SIMULATION METHODS
    // *********************************************************************
    virtual bool                 ConnectSim(const EngineKey &ek,
                                     const stringVector &arguments,
                                     const std::string &simHost,
                                     int simPort,
                                     const std::string &simSecurityKey) = 0;

    virtual void                 SendSimulationCommand(const EngineKey &ek,
                                     const std::string &command,
                                     const std::string &argument) = 0;

    virtual int                  GetWriteSocket(const EngineKey &ek) = 0;
    virtual void                 ReadDataAndProcess(const EngineKey &ek) = 0;
    virtual avtDatabaseMetaData *GetSimulationMetaData(const EngineKey &ek) = 0;
    virtual SILAttributes       *GetSimulationSILAtts(const EngineKey &ek) = 0;
    virtual SimulationCommand   *GetCommandFromSimulation(const EngineKey &ek) = 0;

public:
    // *********************************************************************
    // Engine RPCs
    // *********************************************************************
    virtual bool OpenDatabase(const EngineKey &ek, const std::string &format, 
                              const std::string &filename, int time=0) = 0;

    virtual bool DefineVirtualDatabase(const EngineKey &ek,
                                       const std::string &format,
                                       const std::string &dbName,
                                       const std::string &path, 
                                       const stringVector &files,
                                       int time=0) = 0;
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
                        int windowID) = 0;
    virtual bool ApplyOperator(const EngineKey &ek,
                               const std::string &name,
                               const AttributeSubject *atts) = 0;

    virtual bool MakePlot(const EngineKey &ek,
                          const std::string &plotName,
                          const std::string &pluginID,
                          const AttributeSubject *atts,
                          const std::vector<double> &,
                          int winID, int *networkId) = 0;

    virtual bool UpdatePlotAttributes(const EngineKey &ek,
                                      const std::string &name,
                                      int id, const AttributeSubject *atts) = 0;

    virtual bool UseNetwork(const EngineKey &ek, int networkId) = 0;

    virtual bool Execute(const EngineKey &ek, avtDataObjectReader_p &rdr,
                         bool replyWithNullData,
                         void (*waitCB)(void*), void *waitCBData) = 0;

    virtual int  Render(const EngineKey &ek, avtImage_p &img,
                        avtImageType imgT, bool sendZBuffer, 
                        const intVector &networkIds, 
                        int annotMode, int windowID, bool leftEye,
                        void (*waitCB)(void *), void *waitCBData) = 0;

    // query methods
    virtual bool Pick(const EngineKey &ek, const int nid, int wid,
                      const PickAttributes *atts, PickAttributes &retAtts) = 0;

    virtual bool StartPick(const EngineKey &ek, const bool forZones,
                           const bool flag, const int nid) = 0;

    virtual bool StartQuery(const EngineKey &ek, const bool flag, const int nid) = 0;

    virtual bool SetWinAnnotAtts(const EngineKey &ek,
                                 const WindowAttributes *wa,
                                 const AnnotationAttributes *aa,
                                 const AnnotationObjectList *ao,
                                 const std::string extStr,
                                 const VisualCueList *visCues,
                                 const int *frameAndState,
                                 const double *viewExtents,
                                 const std::string ctName,
                                 const int winID) = 0;

    virtual bool Query(const EngineKey &ek,
                       const intVector &networkIds, 
                       const QueryAttributes *atts,
                       QueryAttributes &retAtts) = 0;

    virtual bool GetQueryParameters(const EngineKey &ek,
                                    const std::string &qname,
                                    std::string *params) = 0;

    // network/memory management
    virtual bool ClearCache(const EngineKey &ek, const std::string &dbName) = 0;
    virtual bool GetProcInfo(const EngineKey &ek, ProcessAttributes &retAtts) = 0;
    virtual bool ReleaseData(const EngineKey &ek, int id) = 0;
    virtual bool CloneNetwork(const EngineKey &ek, int id, 
                              const QueryOverTimeAttributes *qatts) = 0;

    // selections
    virtual bool CreateNamedSelection(const EngineKey &ek, 
                                      int id,
                                      const SelectionProperties &props,
                                      SelectionSummary &summary) = 0;

    virtual bool UpdateNamedSelection(const EngineKey &ek, 
                                      int id,
                                      const SelectionProperties &, 
                                      bool allowCache,
                                      SelectionSummary &summary) = 0;

    virtual bool DeleteNamedSelection(const EngineKey &ek,
                                      const std::string &selName) = 0;

    virtual bool LoadNamedSelection(const EngineKey &ek,
                                    const std::string &selName) = 0;

    virtual bool SaveNamedSelection(const EngineKey &ek,
                                    const std::string &selName) = 0;

    virtual bool ExportDatabases(const EngineKey &ek,
                                 const intVector &ids,
                                 const ExportDBAttributes *expAtts,
                                 const std::string &timeSuffix,
                                 ExportDBAttributes &retAtts) = 0;

    virtual bool ConstructDataBinning(const EngineKey &ek, int id) = 0;

    virtual bool UpdateExpressions(const EngineKey &ek, const ExpressionList &eL) = 0;
    virtual void UpdateDefaultFileOpenOptions(FileOpenOptions*) = 0;
    virtual void UpdatePrecisionType(const int) = 0;
    virtual void UpdateBackendType(const int) = 0;
    virtual void UpdateRemoveDuplicateNodes(const bool) = 0;

    virtual bool LaunchProcess(const EngineKey &ek, const stringVector &args) = 0;

public:
    virtual void CreateNode(DataNode *, bool detailed) = 0;
    virtual void SetFromNode(DataNode *, const std::string &) = 0;
};

#endif
