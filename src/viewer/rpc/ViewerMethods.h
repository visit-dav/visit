/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#ifndef VIEWER_METHODS_H
#define VIEWER_METHODS_H
#include <string>
#include <vectortypes.h>
#include <viewerrpc_exports.h>

class ViewerState;

// ****************************************************************************
// Class: 
//
// Purpose: 
//   Translates method calls into ViewerRPC and notifies observers of the
//   ViewerRPC object.
//
// Note:       
//   We use this class within the viewer and the viewer proxy. This
//   allows different pieces of software to make ViewerRPC calls back
//   to the viewer's event loop. It provides a clean way for isolated pieces 
//   of the code to make the viewer do things. This separation from ViewerProxy
//   also will let the GUI integrate easier into the viewer if we decide to 
//   ever do that.
//
//   This class can manipulate some other state object classes such as
//   ColorTableAttributes to provide convenience functions.
//
// Programmer: VisIt team
// Creation:   Fri Feb 9 17:57:03 PST 2007
//
// Modifications:
//   Brad Whitlock, Fri Feb 9 17:57:17 PST 2007
//   I cut out most of ViewerProxy's methods into this class so the methods
//   that convert method calls into ViewerRPC's are centralized into one 
//   object that can be used outside of ViewerProxy.
//
//   Brad Whitlock, Fri Mar 9 16:22:07 PST 2007
//   Added RequestMetaData.
//
//   Brad Whitlock, Tue May 8 16:48:04 PST 2007
//   Added AddInitializedOperator to avoid CLI logging problems.
//
//   Kathleen Bonnell, Tue May 15 10:39:58 PDT 2007 
//   Added optional bool arg to Lineout, LineQuery.
//
//   Cyrus Harrison, Tue Sep 18 11:05:50 PDT 2007
//   Added SetQueryFloatFormat
//
//   Ellen Tarwater, Thurs, Oct 11, 2007
//   Added flag to DrawPlots to indicate all or only active...
//
//   Cyrus Harrison, Wed Nov 28 14:38:19 PST 2007
//   Added SetCreateVectorMagnitudeExpressions
//
//   Ellen Tarwater, Thurs, Dec 6, 2007
//   Added SetPlotFollowsTime
//
//   Brad Whitlock, Wed Jan 23 10:24:04 PST 2008
//   Added TurnOffAllLocks.
//
//   Jeremy Meredith, Wed Jan 23 16:36:06 EST 2008
//   Added SetDefaultFileOpenOptions.
//
//   Jeremy Meredith, Mon Feb  4 13:33:29 EST 2008
//   Added remaining support for axis array window modality.
//
//   Cyrus Harrision, Thu Feb 21 15:18:49 PST 2008
//   Added SetSuppressMessages
//
//   Brad Whitlock, Thu Jul 24 09:18:11 PDT 2008
//   Added time state argument to OverlayDatabase.
//
//   Brad Whitlock, Wed Jan 14 13:59:09 PST 2009
//   I removed UpdatePlotInfoAtts.
//
//   Hank Childs, Wed Jan 28 14:28:59 PST 2009
//   Added support for named selection methods.
//
// ****************************************************************************

class VIEWER_RPC_API ViewerMethods
{
public:
    ViewerMethods(ViewerState *);
    ~ViewerMethods();

    void Close();
    void Detach();

    void AddWindow();
    void CloneWindow();
    void DeleteWindow();
    void SetWindowLayout(int layout);
    void SetActiveWindow(int windowId);
    void IconifyAllWindows();
    void DeIconifyAllWindows();
    void ShowAllWindows();
    void HideAllWindows();
    void ClearWindow(bool clearAllPlots = true);
    void ClearAllWindows();
    void SaveWindow();
    void PrintWindow();
    void DisableRedraw();
    void RedrawWindow();
    void ResizeWindow(int win, int w, int h);
    void MoveWindow(int win, int x, int y);
    void MoveAndResizeWindow(int win, int x, int y, int w, int h);
    void HideToolbars(bool forAllWindows = false);
    void ShowToolbars(bool forAllWindows = false);

    void ConnectToMetaDataServer(const std::string &hostName, const stringVector &argv);
    void OpenMDServer(const std::string &hostName, const stringVector &argv);

    void OpenDatabase(const std::string &database, int timeState = 0,
                      bool addDefaultPlots = true,
                      const std::string &forcedFileType = "");
    void CloseDatabase(const std::string &database);
    void ActivateDatabase(const std::string &database);
    void CheckForNewStates(const std::string &database);
    void ReOpenDatabase(const std::string &database, bool forceClose = true);
    void ReplaceDatabase(const std::string &database, int timeState = 0);
    void OverlayDatabase(const std::string &database, int timeState = 0);
    void RequestMetaData(const std::string &database, int ts = -1);
    void ClearCache(const std::string &hostName, const std::string &simName);
    void ClearCacheForAllEngines();
    void UpdateDBPluginInfo(const std::string &hostName);
    void ExportDatabase(void);
    void ConstructDDF(void);

    void CreateDatabaseCorrelation(const std::string &name,
                                   const stringVector &dbs, int method,
                                   int nStates = -1);
    void AlterDatabaseCorrelation(const std::string &name,
                                  const stringVector &dbs, int method,
                                  int nStates = -1);
    void DeleteDatabaseCorrelation(const std::string &name);

    void OpenComputeEngine(const std::string &hostName, const stringVector &argv);
    void CloseComputeEngine(const std::string &hostName, const std::string &simName);
    void InterruptComputeEngine(const std::string &hostName, const std::string &simName);

    void ApplyNamedSelection(const std::string &selName);
    void CreateNamedSelection(const std::string &selName);
    void DeleteNamedSelection(const std::string &selName);
    void LoadNamedSelection(const std::string &selName);
    void SaveNamedSelection(const std::string &selName);

    void AnimationSetNFrames(int nFrames);
    void AnimationPlay();
    void AnimationReversePlay();
    void AnimationStop();
    void TimeSliderNextState();
    void TimeSliderPreviousState();
    void SetTimeSliderState(int state);
    void SetActiveTimeSlider(const std::string &ts);

    void AddPlot(int type, const std::string &var);
    void CopyActivePlots(void);
    void SetPlotFrameRange(int plotId, int frame0, int frame1);
    void DeletePlotKeyframe(int plotId, int frame);
    void MovePlotKeyframe(int plotId, int oldFrame, int newFrame);
    void SetPlotDatabaseState(int plotId, int frame, int state);
    void DeletePlotDatabaseKeyframe(int plotId, int frame);
    void MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame);
    void DeleteActivePlots();
    void HideActivePlots();
    void SetPlotFollowsTime();
    void DrawPlots(bool drawAllPlots = true);
    void SetActivePlots(const intVector &activePlotIds);
    void SetActivePlots(const intVector &activePlotIds,
                        const intVector &activeOperatorIds,
                        const intVector &expandedPlots);
    void ChangeActivePlotsVar(const std::string &var);

    void AddOperator(int oper, const bool fromDefault = true);
    void AddInitializedOperator(int oper);
    void PromoteOperator(int operatorId);
    void DemoteOperator(int operatorId);
    void RemoveOperator(int operatorId);
    void RemoveLastOperator();
    void RemoveAllOperators();

    void SetDefaultPlotOptions(int type);
    void SetPlotOptions(int type);
    void ResetPlotOptions(int type);
    void SetDefaultOperatorOptions(int oper);
    void SetOperatorOptions(int oper);
    void ResetOperatorOptions(int type);

    void SetActiveContinuousColorTable(const std::string &colorTableName);
    void SetActiveDiscreteColorTable(const std::string &colorTableName);
    void DeleteColorTable(const std::string &colorTableName);
    void UpdateColorTable(const std::string &colorTableName);
    void ExportColorTable(const std::string &colorTableName);
    void InvertBackgroundColor();

    void SetCenterOfRotation(double, double, double);
    void ChooseCenterOfRotation();
    void ChooseCenterOfRotation(double, double);
    void SetViewAxisArray();
    void SetViewCurve();
    void SetView2D();
    void SetView3D();
    void ClearViewKeyframes();
    void DeleteViewKeyframe(int frame);
    void MoveViewKeyframe(int oldFrame, int newFrame);
    void SetViewKeyframe();
    void ResetView();
    void RecenterView();
    void SetViewExtentsType(int t);
    void ToggleMaintainViewMode();
    void ToggleMaintainDataMode();
    void UndoView();
    void RedoView();
    void ToggleLockViewMode();
    void ToggleLockTime();
    void ToggleLockTools();
    void TurnOffAllLocks();
    void ToggleSpinMode();
    void ToggleCameraViewMode();
    void ToggleFullFrameMode();

    void SetWindowMode(int mode);
    void ToggleBoundingBoxMode();
    void EnableTool(int tool, bool enabled);

    void CopyViewToWindow(int from, int to);
    void CopyLightingToWindow(int from, int to);
    void CopyAnnotationsToWindow(int from, int to);
    void CopyPlotsToWindow(int from, int to);

    void SetAnnotationAttributes();
    void SetDefaultAnnotationAttributes();
    void ResetAnnotationAttributes();
    void AddAnnotationObject(int annotType, const std::string &annotName);
    void HideActiveAnnotationObjects();
    void DeleteActiveAnnotationObjects();
    void RaiseActiveAnnotationObjects();
    void LowerActiveAnnotationObjects();
    void SetAnnotationObjectOptions();
    void SetDefaultAnnotationObjectList();
    void ResetAnnotationObjectList();

    void SetInteractorAttributes();
    void SetDefaultInteractorAttributes();
    void ResetInteractorAttributes();

    void SetKeyframeAttributes();

    void SetMaterialAttributes();
    void SetDefaultMaterialAttributes();
    void ResetMaterialAttributes();

    void SetLightList();
    void SetDefaultLightList();
    void ResetLightList();

    void SetAnimationAttributes();

    void SetAppearanceAttributes();
    void ProcessExpressions();

    void ClearPickPoints();
    void ClearReferenceLines();

    void SetRenderingAttributes();
    void SetWindowArea(int x, int y, int w, int h);

    void SetGlobalLineoutAttributes();
    void SetPickAttributes();
    void SetDefaultPickAttributes();
    void ResetPickAttributes();
    void ResetPickLetter();

    void ResetLineoutColor();

    void SetQueryOverTimeAttributes();
    void SetDefaultQueryOverTimeAttributes();
    void ResetQueryOverTimeAttributes();

    void SetTryHarderCyclesTimes(int flag);
    void SetTreatAllDBsAsTimeVarying(int flag);
    void SetCreateMeshQualityExpressions(int flag);
    void SetCreateTimeDerivativeExpressions(int flag);
    void SetCreateVectorMagnitudeExpressions(int flag);
    
    void SetSuppressMessages(int flag);

    void SetMeshManagementAttributes();
    void SetDefaultMeshManagementAttributes();
    void ResetMeshManagementAttributes();

    void WriteConfigFile();
    void ExportEntireState(const std::string &filename);
    void ImportEntireState(const std::string &filename, bool inVisItDir);
    void ImportEntireStateWithDifferentSources(const std::string &filename,
                                               bool inVisItDir,
                                               const stringVector &sources);

    void SetPlotSILRestriction();

    void SetDefaultFileOpenOptions();

    // Methods for querying
    void SuppressQueryOutput(bool onOff);
    void SetQueryFloatFormat(const std::string &format);
    
    void DatabaseQuery(const std::string &queryName, const stringVector &vars,
                       const bool = false, const int arg1 = 0, 
                       const int arg2 = 0, const bool = false,
                       const doubleVector &darg1 = doubleVector(),
                       const doubleVector &darg2 = doubleVector());
    void PointQuery(const std::string &queryName, const double pt[3],
                    const stringVector &vars, const bool = false,
                    const int arg1 = -1, const int arg2 = -1,
                    const bool = false);
    void LineQuery(const std::string &queryName, const double pt1[3],
                   const double pt2[3], const stringVector &vars,
                   const int samples, const bool forceSampling = false);
    void Pick(int x, int y, const stringVector &vars);
    void Pick(double xyz[3], const stringVector &vars);
    void NodePick(int x, int y, const stringVector &vars);
    void NodePick(double xyz[3], const stringVector &vars);
    void Lineout(const double p0[3], const double p1[3],
                 const stringVector &vars, const int samples,
                 const bool forceSampling = false);

    void QueryProcessAttributes(int id,
                                const std::string engineHostName,
                                const std::string engineDbName);

    void SendSimulationCommand(const std::string &hostName,
                               const std::string &simName,
                               const std::string &command);

    void SendSimulationCommand(const std::string &hostName,
                               const std::string &simName,
                               const std::string &command,
                               const std::string &argument);

    void OpenClient(const std::string &clientName, 
                    const std::string &program,
                    const stringVector &args);

private:
    ViewerState *state;
};

#endif
