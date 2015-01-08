/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <ViewerEngineManager.h>
#include <ViewerEngineManagerImplementation.h>

#include <MaterialAttributes.h>
#include <ExpressionList.h>
#include <MeshManagementAttributes.h>

#define IMPL() ViewerEngineManagerImplementation::Instance()

ViewerEngineManager::ViewerEngineManager() : ViewerEngineManagerInterface()
{
}

ViewerEngineManager::~ViewerEngineManager()
{
}

bool
ViewerEngineManager::CreateEngine(const EngineKey &ek,
                      const stringVector &arguments,
                      bool  skipChooser,
                      int   numRestarts,
                      bool  reverseLaunch)
{
    return IMPL()->CreateEngine(ek, arguments, skipChooser, numRestarts, reverseLaunch);
}

bool
ViewerEngineManager::CreateEngineEx(const EngineKey &ek,
                        const stringVector &arguments,
                        bool  skipChooser,
                        int   numRestarts,
                        bool  reverseLaunch,
                        ViewerConnectionProgress *progress)
{
    return IMPL()->CreateEngineEx(ek, arguments, skipChooser, numRestarts, reverseLaunch, progress);
}

void
ViewerEngineManager::CloseEngines()
{
    IMPL()->CloseEngines();
}

void
ViewerEngineManager::CloseEngine(const EngineKey &ek)
{
    IMPL()->CloseEngine(ek);
}

void
ViewerEngineManager::InterruptEngine(const EngineKey &ek)
{
    IMPL()->InterruptEngine(ek);
}

void
ViewerEngineManager::SendKeepAlives()
{
    IMPL()->SendKeepAlives();
}

bool
ViewerEngineManager::EngineExists(const EngineKey &ek)
{
    return IMPL()->EngineExists(ek);
}

void
ViewerEngineManager::ClearCacheForAllEngines()
{
    IMPL()->ClearCacheForAllEngines();
}

void
ViewerEngineManager::UpdateEngineList()
{
    IMPL()->UpdateEngineList();
}

bool
ViewerEngineManager::ConnectSim(const EngineKey &ek,
                                const stringVector &arguments,
                                const std::string &simHost,
                                int simPort,
                                const std::string &simSecurityKey)
{
    return IMPL()->ConnectSim(ek, arguments, simHost, simPort, simSecurityKey);
}


void
ViewerEngineManager::SendSimulationCommand(const EngineKey &ek,
                                           const std::string &command,
                                           const std::string &argument)
{
    IMPL()->SendSimulationCommand(ek, command, argument);
}


int
ViewerEngineManager::GetWriteSocket(const EngineKey &ek)
{
    return IMPL()->GetWriteSocket(ek);
}

void
ViewerEngineManager::ReadDataAndProcess(const EngineKey &ek)
{
    IMPL()->ReadDataAndProcess(ek);
}

avtDatabaseMetaData *
ViewerEngineManager::GetSimulationMetaData(const EngineKey &ek)
{
    return IMPL()->GetSimulationMetaData(ek);
}

SILAttributes *
ViewerEngineManager::GetSimulationSILAtts(const EngineKey &ek)
{
    return IMPL()->GetSimulationSILAtts(ek);
}

SimulationCommand *
ViewerEngineManager::GetCommandFromSimulation(const EngineKey &ek)
{
    return IMPL()->GetCommandFromSimulation(ek);
}

bool
ViewerEngineManager::OpenDatabase(const EngineKey &ek, 
                                  const std::string &format, 
                                  const std::string &filename, int time)
{
    return IMPL()->OpenDatabase(ek, format, filename, time);
}

bool
ViewerEngineManager::DefineVirtualDatabase(const EngineKey &ek,
                                       const std::string &format,
                                       const std::string &dbName,
                                       const std::string &path, 
                                       const stringVector &files,
                                       int time)
{
    return IMPL()->DefineVirtualDatabase(ek, format, dbName, path, files, time);
}

bool
ViewerEngineManager::ReadDataObject(const EngineKey &ek,
    const std::string &format,
    const std::string &filename,
    const std::string &var, const int ts,
    avtSILRestriction_p silr,
    const MaterialAttributes &ma,
    const ExpressionList &el,
    const MeshManagementAttributes &mma,
    bool treatAllDbsAsTimeVarying,
    bool ignoreExtents,
    const std::string &selName,
    int windowID)
{
    return IMPL()->ReadDataObject(ek, format, filename, var, ts, silr,
         ma, el, mma, treatAllDbsAsTimeVarying, ignoreExtents, selName, windowID);
}

bool
ViewerEngineManager::ApplyOperator(const EngineKey &ek,
                               const std::string &name,
                               const AttributeSubject *atts)
{
    return IMPL()->ApplyOperator(ek, name, atts);
}

bool
ViewerEngineManager::MakePlot(const EngineKey &ek,
                          const std::string &plotName,
                          const std::string &pluginID,
                          const AttributeSubject *atts,
                          const std::vector<double> &ext,
                          int winID, int *networkId)
{
    return IMPL()->MakePlot(ek, plotName, pluginID, atts, ext, winID, networkId);
}

bool
ViewerEngineManager::UpdatePlotAttributes(const EngineKey &ek,
                                               const std::string &name,
                                               int id, const AttributeSubject *atts)
{
    return IMPL()->UpdatePlotAttributes(ek, name, id, atts);
}

bool
ViewerEngineManager::UseNetwork(const EngineKey &ek, int networkId)
{
    return IMPL()->UseNetwork(ek, networkId);
}

bool
ViewerEngineManager::Execute(const EngineKey &ek, avtDataObjectReader_p &rdr,
                             bool replyWithNullData,
                             void (*waitCB)(void*), void *waitCBData)
{
    return IMPL()->Execute(ek, rdr, replyWithNullData, waitCB, waitCBData);
}

bool 
ViewerEngineManager::Render(const EngineKey &ek, avtDataObjectReader_p &rdr,
                            bool sendZBuffer, const intVector &networkIds, 
                            int annotMode, int windowID, bool leftEye,
                            void (*waitCB)(void *), void *waitCBData)
{
    return IMPL()->Render(ek, rdr, sendZBuffer, networkIds,
                          annotMode, windowID, leftEye,
                          waitCB, waitCBData);
}


bool
ViewerEngineManager::Pick(const EngineKey &ek, const int nid, int wid,
                          const PickAttributes *atts, PickAttributes &retAtts)
{
    return IMPL()->Pick(ek, nid, wid, atts, retAtts);
}

bool
ViewerEngineManager::StartPick(const EngineKey &ek, const bool forZones,
                               const bool flag, const int nid)
{
    return IMPL()->StartPick(ek, forZones, flag, nid);
}

bool
ViewerEngineManager::StartQuery(const EngineKey &ek, const bool flag, const int nid)
{
    return IMPL()->StartQuery(ek, flag, nid);
}

bool
ViewerEngineManager::SetWinAnnotAtts(const EngineKey &ek,
                                 const WindowAttributes *wa,
                                 const AnnotationAttributes *aa,
                                 const AnnotationObjectList *ao,
                                 const std::string extStr,
                                 const VisualCueList *visCues,
                                 const int *frameAndState,
                                 const double *viewExtents,
                                 const std::string ctName,
                                 const int winID)
{
    return IMPL()->SetWinAnnotAtts(ek, wa, aa, ao, extStr, visCues,
                                   frameAndState, viewExtents, ctName, winID);
}

bool
ViewerEngineManager::Query(const EngineKey &ek,
                           const intVector &networkIds, 
                           const QueryAttributes *atts,
                           QueryAttributes &retAtts)
{
    return IMPL()->Query(ek, networkIds, atts, retAtts);
}

bool
ViewerEngineManager::GetQueryParameters(const EngineKey &ek,
                                        const std::string &qname,
                                        std::string *params)
{
    return IMPL()->GetQueryParameters(ek, qname, params);
}

bool
ViewerEngineManager::ClearCache(const EngineKey &ek, const std::string &dbName)
{
    return IMPL()->ClearCache(ek, dbName);
}

bool
ViewerEngineManager::GetProcInfo(const EngineKey &ek, ProcessAttributes &retAtts)
{
    return IMPL()->GetProcInfo(ek, retAtts);
}

bool
ViewerEngineManager::ReleaseData(const EngineKey &ek, int id)
{
    return IMPL()->ReleaseData(ek, id);
}

bool
ViewerEngineManager::CloneNetwork(const EngineKey &ek, int id, 
                                  const QueryOverTimeAttributes *qatts)
{
    return IMPL()->CloneNetwork(ek, id, qatts);
}

bool
ViewerEngineManager::CreateNamedSelection(const EngineKey &ek, 
                                          int id,
                                          const SelectionProperties &props,
                                          SelectionSummary &summary)
{
    return IMPL()->CreateNamedSelection(ek, id, props, summary);
}

bool
ViewerEngineManager::UpdateNamedSelection(const EngineKey &ek, 
                                          int id,
                                          const SelectionProperties &props, 
                                          bool allowCache,
                                          SelectionSummary &summary)
{
    return IMPL()->UpdateNamedSelection(ek, id, props, allowCache, summary);
}

bool
ViewerEngineManager::DeleteNamedSelection(const EngineKey &ek,
                                          const std::string &selName)
{
    return IMPL()->DeleteNamedSelection(ek, selName);
}

bool
ViewerEngineManager::LoadNamedSelection(const EngineKey &ek,
                                        const std::string &selName)
{
    return IMPL()->LoadNamedSelection(ek, selName);
}

bool
ViewerEngineManager::SaveNamedSelection(const EngineKey &ek,
                                        const std::string &selName)
{
    return IMPL()->SaveNamedSelection(ek, selName);
}

bool
ViewerEngineManager::ExportDatabases(const EngineKey &ek,
                                     const intVector &ids,
                                     const ExportDBAttributes &expAtts, 
                                     const std::string &timeSuffix)
{
    return IMPL()->ExportDatabases(ek, ids, expAtts, timeSuffix);
}

bool
ViewerEngineManager::ConstructDataBinning(const EngineKey &ek, int id)
{
    return IMPL()->ConstructDataBinning(ek, id);
}

bool
ViewerEngineManager::UpdateExpressions(const EngineKey &ek, const ExpressionList &eL)
{
    return IMPL()->UpdateExpressions(ek, eL);
}

void
ViewerEngineManager::UpdateDefaultFileOpenOptions(FileOpenOptions *opts)
{
    IMPL()->UpdateDefaultFileOpenOptions(opts);
}

void ViewerEngineManager::UpdatePrecisionType(const int type)
{
    IMPL()->UpdatePrecisionType(type);
}

void ViewerEngineManager::UpdateBackendType(const int type)
{
    IMPL()->UpdateBackendType(type);
}

void ViewerEngineManager::UpdateRemoveDuplicateNodes(const bool flag)
{
    IMPL()->UpdateRemoveDuplicateNodes(flag);
}


bool ViewerEngineManager::LaunchProcess(const EngineKey &ek, const stringVector &args)
{
    return IMPL()->LaunchProcess(ek, args);
}

void
ViewerEngineManager::CreateNode(DataNode *parent, bool detailed)
{
    IMPL()->CreateNode(parent, detailed);
}

void
ViewerEngineManager::SetFromNode(DataNode *parent, const std::string &configVersion)
{
    IMPL()->SetFromNode(parent, configVersion);
}


