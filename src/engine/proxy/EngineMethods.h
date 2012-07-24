/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                              EngineMethods.h                            //
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

class EngineState;
class AttributeSubject;
class QueryAttributes;
class ExportDBAttributes;
class QueryOverTimeAttributes;
class AnnotationObjectList;
class ConstructDataBinningAttributes;
class VisualCueList;
class StatusAttributes;

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

    avtDataObjectReader_p    Render(bool, const intVector&, int, int, bool,
                                 void (*waitCB)(void *), void *cbData);

    avtDataObjectReader_p    Execute(bool, void (*waitCB)(void*),void *cbData);

    void                     ClearCache();
    void                     ClearCache(const std::string &);

    void                     Query(const std::vector<int> &,
                                   const QueryAttributes *,
                                   QueryAttributes &);
    void                     ExportDatabase(int, const ExportDBAttributes *);
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


