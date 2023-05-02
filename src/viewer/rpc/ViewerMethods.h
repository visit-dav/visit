// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_METHODS_H
#define VIEWER_METHODS_H
#include <string>
#include <vectortypes.h>
#include <viewerrpc_exports.h>
#include <MapNode.h>
#include <SelectionProperties.h>

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
//   Gunther H. Weber, Mon Apr  6 19:03:03 PDT 2009
//   Added arguments for host name and simulation name to methods for loading
//   and saving named selections. 
//
//   Cyrus Harrison, Tue Apr 14 13:35:54 PDT 2009
//   Added argument to ReplaceDatabase to  allow replace of only active 
//   plots.
//
//   Brad Whitlock, Tue Oct 20 10:25:15 PDT 2009
//   I added methods to set a plot description and to adjust plot order.
//
//   Jeremy Meredith, Wed Feb  3 15:35:08 EST 2010
//   Removed maintain data; moved maintain view from Global settings
//   (Main window) to per-window Window Information (View window).
//
//   Brad Whitlock, Fri Aug 13 14:27:22 PDT 2010
//   I added UpdateNamedSelection, SetNamedSelectionAutoApply
//
//   Hank Childs, Sat Aug 21 14:20:04 PDT 2010
//   Rename method: DDF to DataBinning.
//
//   Brad Whitlock, Fri Aug 27 10:38:23 PDT 2010
//   I added RenamePickLabel.
//
//   Brad Whitlock, Tue Dec 14 16:45:05 PST 2010
//   I added new methods that let me pass selection properties when creating
//   or updating a selection.
//
//   Kathleen Bonnell, Tue Mar  1 11:12:51 PST 2011
//   Added another int arg to PointQuery.
//
//   Brad Whitlock, Tue Mar 29 11:05:58 PDT 2011
//   Added a bool to SetPlotFollowsTime.
//
//   Kathleen Bonnell, Fri Jun 10 14:00:11 PDT 2011
//   Added another bool to PointQuery.
//
//   Kathleen Biagas, Fri Jun 17 16:38:04 PDT 2011
//   Add Query method, removed other forms of query methods.
//
//   Gunther H. Weber, Mon Jul 11 13:12:35 PDT 2011
//   Added flag to SetActiveWindow to specify whether to raise window.
//
//   Kathleen Biagas, Fri Jul 15 11:35:27 PDT 2011
//   Add GetQueryParameters.
//
//   Marc Durant, Thu Jan 12 12:35:00 MST 2012
//   Added ToggleAllowPopup.
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added methods for connecting/disconnecting with DDT and for
//   instructing DDT to focus on a specific domain
//
//   Kathleen Biagas, Wed Aug  7 13:05:32 PDT 2013
//   Added SetPrecisionType.
//
//   Brad Whitlock, Thu Sep 12 15:17:57 PDT 2013
//   Added StartPlotAnimation, StopPlotAnimation.
//
//   Cameron Christensen, Tuesday, June 10, 2014
//   Added SetBackendType.
//
//   Kathleen Biagas, Mon Dec 22 10:19:45 PST 2014
//   Add SetRemoveDuplicateNodes.
//
//   David Camp, Thu Aug 27 09:40:00 PDT 2015
//   Added host name to the Export and Import of the state functions.
//
//   Alister Maguire, Mon Oct 16 15:41:23 PDT 2017
//   Added RemovePicks.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//
//   Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Added operator keyframing.
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
    void SetActiveWindow(int windowId, bool raiseWindow = true);
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
    void ReplaceDatabase(const std::string &database, int timeState = 0, 
                         bool onlyReplaceActive = false);
    void OverlayDatabase(const std::string &database, int timeState = 0);
    void RequestMetaData(const std::string &database, int ts = -1);
    void ClearCache(const std::string &hostName, const std::string &simName);
    void ClearCacheForAllEngines();
    void UpdateDBPluginInfo(const std::string &hostName);
    void ExportDatabase(void);
    void ConstructDataBinning(void);

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
    void CreateNamedSelection(const std::string &selName, 
                              const SelectionProperties &props, 
                              bool useCurrentPlot = false);
    void DeleteNamedSelection(const std::string &selName);
    void LoadNamedSelection(const std::string &selName,
                            const std::string &hostName,
                            const std::string &simName);
    void SaveNamedSelection(const std::string &selName);
    void SetNamedSelectionAutoApply(bool);
    void UpdateNamedSelection(const std::string &selName, bool updatePlots, bool allowCaching=true);
    void UpdateNamedSelection(const std::string &selName, const SelectionProperties &props,
                              bool updatePlots, bool allowCaching);
    void InitializeNamedSelectionVariables(const std::string &selName);

    void AnimationSetNFrames(int nFrames);
    void AnimationPlay();
    void AnimationReversePlay();
    void AnimationStop();
    void TimeSliderNextState();
    void TimeSliderPreviousState();
    void SetTimeSliderState(int state);
    void SetActiveTimeSlider(const std::string &ts);
    void StartPlotAnimation(const intVector &plotIds);
    void StopPlotAnimation(const intVector &plotIds);

    void AddPlot(int type, const std::string &var);
    void AddEmbeddedPlot(int type, const std::string &var, int id);
    void CopyActivePlots(void);
    void SetPlotFrameRange(int plotId, int frame0, int frame1);
    void DeletePlotKeyframe(int plotId, int frame);
    void MovePlotKeyframe(int plotId, int oldFrame, int newFrame);
    void DeleteOperatorKeyframe(int plotId, int operatorId, int frame);
    void MoveOperatorKeyframe(int plotId, int operatorId,
                              int oldFrame, int newFrame);
    void SetPlotDatabaseState(int plotId, int frame, int state);
    void DeletePlotDatabaseKeyframe(int plotId, int frame);
    void MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame);
    void DeleteActivePlots();
    void HideActivePlots();
    void SetPlotFollowsTime(bool);
    void DrawPlots(bool drawAllPlots = true);
    void SetActivePlots(const intVector &activePlotIds);
    void SetActivePlots(const intVector &activePlotIds,
                        const intVector &activeOperatorIds,
                        const intVector &expandedPlots);
    void ChangeActivePlotsVar(const std::string &var);
    void SetPlotDescription(int plotId, const std::string &description);
    void MovePlotOrderTowardFirst(int plotId);
    void MovePlotOrderTowardLast(int plotId);
    void SetPlotOrderToFirst(int plotId);
    void SetPlotOrderToLast(int plotId);

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

    void SetDefaultContinuousColorTable(const std::string &colorTableName);
    void SetDefaultDiscreteColorTable(const std::string &colorTableName);
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
    void UndoView();
    void RedoView();
    void ToggleAllowPopup();
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
    void RemovePicks(const std::string);
    void ClearReferenceLines();

    void SetRenderingAttributes();
    void SetWindowArea(int x, int y, int w, int h);

    void SetGlobalLineoutAttributes();
    void SetPickAttributes();
    void SetDefaultPickAttributes();
    void ResetPickAttributes();
    void ResetPickLetter();
    void RenamePickLabel(const std::string &, const std::string &);
    void ResetLineoutColor();

    void SetQueryOverTimeAttributes();
    void SetDefaultQueryOverTimeAttributes();
    void ResetQueryOverTimeAttributes();

    void SetTryHarderCyclesTimes(int flag);
    void SetTreatAllDBsAsTimeVarying(int flag);
    void SetCreateMeshQualityExpressions(int flag);
    void SetCreateTimeDerivativeExpressions(int flag);
    void SetCreateVectorMagnitudeExpressions(int flag);
    void SetPrecisionType(int flag);
    void SetBackendType(int flag);
    void SetRemoveDuplicateNodes(bool flag);
    
    void SetSuppressMessages(int flag);

    void SetMeshManagementAttributes();
    void SetDefaultMeshManagementAttributes();
    void ResetMeshManagementAttributes();

    void WriteConfigFile();
    void ExportEntireState(const std::string &filename, const std::string &hostname);
    void ImportEntireState(const std::string &filename, bool inVisItDir, const std::string &hostname);
    void ImportEntireStateWithDifferentSources(const std::string &filename,
                                               bool inVisItDir,
                                               const stringVector &sources,
                                               const std::string &hostname);
    void ImportEntireStateWithDifferentSourcesFromString(const std::string &s,
                                               const stringVector &sources);
    void ReadHostProfilesFromDirectory(const std::string &dir, bool clear);

    void SetPlotSILRestriction();

    void SetDefaultFileOpenOptions();

    // Methods for querying
    void SuppressQueryOutput(bool onOff);
    void SetQueryFloatFormat(const std::string &format);
    
    void Query(const MapNode &queryParams);
    void GetQueryParameters(const std::string &qName);

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

    void DDTConnect(bool connect);
    void DDTFocus(int domain, const std::string &variable, int element, const std::string &value);

    /// Experimental features (that are not official yet)
    void ExportWindows(const intVector &windowIds, const std::string& format);
    void ExportHostProfile(const std::string &profile, const std::string &filename, bool saveInUserDir = false);
    void UpdateMouseActions(const int& windowId, const std::string& mouseButton, const double &start_dx, const double& start_dy, const double& end_dx, const double &end_dy, const bool &ctrl, const bool &shift);
    void GetFileList(const std::string& path);
    void ForceRedraw(int windowId);
    void RegisterNewWindow(int windowId);
private:
    ViewerState *state;
};

#endif
