// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              EngineMethods.h                              //
// ************************************************************************* //

#ifndef ENGINE_METHODS_H
#define ENGINE_METHODS_H


#include <engine_proxy_exports.h>
#include <string>
#include <maptypes.h>
#include <vectortypes.h>
#include <avtSILRestriction.h>
#include <MaterialAttributes.h>
#include <SelectionSummary.h>
#include <ExpressionList.h>
#include <MeshManagementAttributes.h>
#include <WindowAttributes.h>
#include <AnnotationAttributes.h>
#include <SelectionProperties.h>
#include <PickAttributes.h>
#include <EngineProperties.h>
#include <ProcessAttributes.h>
#include <FileOpenOptions.h>
#include <ParentProcess.h>
#include <RemoteProcess.h>
#include <avtDataObjectReader.h>
#include <avtImageType.h>

class EngineState;
class AttributeSubject;
class QueryAttributes;
class ExportDBAttributes;
class QueryOverTimeAttributes;
class AnnotationObjectList;
class ConstructDataBinningAttributes;
class VisualCueList;
class StatusAttributes;

// ****************************************************************************
//  Class: EngineMethods
//
//  Purpose:
//      EngineMethods provides an interface to calling engine methods via RPCs.
//
//  Note: These methods were separated out from EngineProxy.
//
//  Programmer: Eric Brugger
//  Creation:   July 26, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Apr  7 15:40:13 PDT 2014
//    I changed export database so it takes a vector of network ids.
//    Work partially supported by DOE Grant SC0007548.
//
//    Kathleen Biagas, Fri Apr 23 2021
//    Change ExportDatabases to have a return atts.
//
// ****************************************************************************


class ENGINE_PROXY_API EngineMethods
{
public:
    EngineMethods(EngineState*);
    ~EngineMethods();

    StatusAttributes        *GetStatusAttributes() const;

    // RPCs to access functionality on the engine.
    void                     OpenDatabase(const std::string &,
                                          const std::string &, int = 0,
                                          bool=true, bool=true, bool=false);
    void                     DefineVirtualDatabase(const std::string &,
                                                   const std::string &,
                                                   const std::string &,
                                                   const stringVector &,
                                                   int = 0, bool=true,
                                                   bool=true);
    void                     ReadDataObject(const std::string &format,
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
    void                     ApplyOperator(const std::string&,
                                           const AttributeSubject*);
    void                     ApplyNamedFunction(const std::string &name,
                                                int nargs);
    int                      MakePlot(const std::string &plotName,
                                      const std::string &pluginID,
                                      const AttributeSubject*,
                                      const std::vector<double>&,
                                      int winID);

    void                     UseNetwork(int);
    void                     UpdatePlotAttributes(const std::string &, int,
                                                  const AttributeSubject*);
    void                     Pick(const int, const PickAttributes *,
                                  PickAttributes &, const int);
    void                     StartPick(const bool, const bool, const int);
    void                     StartQuery(const bool, const int);

    void                     SetWinAnnotAtts(const WindowAttributes*,
                                             const AnnotationAttributes*,
                                             const AnnotationObjectList*,
                                             std::string,
                                             const VisualCueList*,
                                             const int *frameAndState,
                                             const double *viewExtents,
                                             const std::string,
                                             const int winID);

    void                     SetDefaultFileOpenOptions(const FileOpenOptions&);
    void                     SetPrecisionType(const int);
    void                     SetBackendType(const int);
    void                     SetRemoveDuplicateNodes(bool);

    avtDataObjectReader_p    Render(avtImageType imgT, bool sendZBuffer,
                                    const intVector& networkIDs,
                                    int annotMode, int windowID, bool leftEye,
                                    void (*waitCB)(void *), void *cbData);

    avtDataObjectReader_p    Execute(bool replyWithNullData, void (*waitCB)(void*),void *cbData);

    void                     ClearCache();
    void                     ClearCache(const std::string &);

    void                     Query(const std::vector<int> &,
                                   const QueryAttributes *,
                                   QueryAttributes &);
    void                     ExportDatabases(const intVector &ids,
                                             const ExportDBAttributes *,
                                             const std::string &timeSuffix,
                                             ExportDBAttributes &);
    void                     ConstructDataBinning(int, const ConstructDataBinningAttributes *);

    const SelectionSummary  &CreateNamedSelection(int id, const SelectionProperties &props);
    const SelectionSummary  &UpdateNamedSelection(int id, const SelectionProperties &props, bool cache);
    void                     DeleteNamedSelection(const std::string selName);
    void                     LoadNamedSelection(const std::string selName);
    void                     SaveNamedSelection(const std::string selName);
    void                     ReleaseData(const int);
    void                     CloneNetwork(const int,
                                          const QueryOverTimeAttributes *);
    void                     UpdateExpressions(const ExpressionList &);

    std::string              GetQueryParameters(const std::string &qName);

    void                     GetProcInfo(ProcessAttributes &);

    void                     ExecuteSimulationControlCommand(
                                                      const std::string &cmd);
    void                     ExecuteSimulationControlCommand(
                                                      const std::string &cmd,
                                                      const std::string &arg);
    EngineProperties         GetEngineProperties();

    void                     LaunchProcess(const stringVector &programArgs);

    void                    SetParentProcess(ParentProcess* _engineP) { engineP = _engineP; }
    void                    SetRemoteProcess(RemoteProcess* _component) { component = _component; }
    void                    SetNumNodes(int _numNodes) { numNodes = _numNodes; }

protected:
    void                     BlockForNamedSelectionOperation();
    void                     Status(const char *message);
    void                     Status(int percent, int curStage,
                                    const std::string &curStageName,
                                    int maxStage);
    void                     ClearStatus();
    void                     Warning(const char *message);

private:
    int                      numNodes;

    // For indicating status.
    ParentProcess           *engineP;
    RemoteProcess           *component;
    EngineState* state;
};
#endif


