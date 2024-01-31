// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              ViewerPlotList.h                             //
// ************************************************************************* //

#ifndef VIEWER_PLOT_LIST_H
#define VIEWER_PLOT_LIST_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <avtTypes.h>
#include <avtSILRestriction.h>
#include <AnimationAttributes.h>
#include <EngineKey.h>
#include <string>
#include <maptypes.h>
#include <vectortypes.h>
#include <VisWindowTypes.h>
#include <enumtypes.h>

#define KF_TIME_SLIDER "Keyframe animation"

// Forward declarations.
class AttributeSubject;
class DataNode;
class DatabaseCorrelation;
class PlotList;
class SILRestrictionAttributes;
class ViewerOperator;
class ViewerPlot;
class ViewerPlotList;
class ViewerWindow;
class avtToolInterface;
class PickAttributes;
class avtDatabaseMetaData;

typedef std::map<std::string, avtSILRestriction_p> SILRestrictionMap;
typedef std::map<std::string, int> StringIntMap;

// ****************************************************************************
//  Class: ViewerPlotList
//
//  Purpose:
//    A list of plots.  The plots in the list each have the attribute
//    hidden, active and realized.  Operations can also be performed on
//    the list.
//
//  Programmer: Eric Brugger
//  Creation:   August 2, 2000
//
//  Modifications:
//    Eric Brugger, Wed Feb 21 08:33:54 PST 2001
//    Replace the use of VisWindow with ViewerWindow.
//
//    Eric Brugger, Fri Feb 23 13:03:37 PST 2001
//    I added GetPlotLimits.
//
//    Eric Brugger, Thu Mar  8 14:53:27 PST 2001
//    Change the data type for the plot type to an integer.
//
//    Brad Whitlock, Thu Jun 14 16:33:44 PST 2001
//    I added the UpdateColorTable method.
//
//    Brad Whitlock, Thu Jun 21 13:52:42 PST 2001
//    I added support for SIL restrictions.
//
//    Jeremy Meredith, Thu Jul 26 03:16:29 PDT 2001
//    Removed all references to OperType.
//
//    Kathleen Bonnell, Wed Sep 26 15:02:36 PDT 2001 
//    Added methods to set foreground/background colors, and data members 
//    to store them. 
//
//    Brad Whitlock, Tue Oct 9 14:45:00 PST 2001
//    Added a method for handling tools.
//
//    Eric Brugger, Fri Oct 26 14:15:53 PDT 2001
//    I added the method GetNumRealizedPlots.
//
//    Eric Brugger, Wed Nov 21 11:02:24 PST 2001
//    I added the ClearPipelines method.
//
//    Kathleen Bonnell, Thu Nov 15 16:06:43 PST 2001
//    Added Pick, StartPick, StopPick.
//
//    Brad Whitlock, Thu Feb 7 16:43:02 PST 2002
//    Modified interfaces to SIL restriction related functions.
//
//    Brad Whitlock, Mon Feb 11 14:00:14 PST 2002
//    Added a method that is called when a tool is being enabled.
//
//    Brad Whitlock, Tue Feb 12 15:20:29 PST 2002
//    Added a new method to set the database name.
//
//    Brad Whitlock, Tue Feb 26 16:04:03 PST 2002
//    Added an optional argument to AddOperator, RemoveLastOperator, and
//    RemoveAllOperators.
//
//    Brad Whitlock, Wed Mar 6 16:33:33 PST 2002
//    Added ReplaceDatabase, OverlayDatabase, TopSetForVar methods. Changed
//    AddPlot so it has an argument that can replace all plots.
//
//    Kathleen Bonnell, Tue Mar 26 10:15:00 PST 2002  
//    StartPick no longer returns bool. 
//
//    Brad Whitlock, Thu Apr 11 17:29:32 PST 2002
//    Added an argument to SetPlotOperatorAtts.
//
//    Jeremy Meredith, Wed May  8 13:41:24 PDT 2002
//    Added ids to plots.
//
//    Kathleen Bonnell, Fri May 10 16:16:12 PDT 2002  
//    Added method to retrieve VarName.
//
//    Kathleen Bonnell, Wed May 29 10:42:28 PDT 2002  
//    Added methods GetPlotID and GetPlot.  Make AddPlot and SimpleAddPlot
//    return the index of the new plot.
//
//    Hank Childs, Mon Jul 15 15:54:32 PDT 2002
//    Added extents type for plot limits.
//
//    Brad Whitlock, Tue Jul 23 10:38:04 PDT 2002
//    I added an optional argument to SetPlotSILRestriction.
//
//    Brad Whitlock, Thu Jul 25 12:19:55 PDT 2002
//    I added an argument to UpdateWindow that tells the method whether or
//    not to do immediate updates. I also added a method to interrupt plot
//    updates.
//
//    Brad Whitlock, Mon Jul 29 15:34:22 PST 2002
//    I added a method to clear out the default SIL restriction for a
//    specified file.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Removed Pick method, now handled by ViewerQueryManager. 
//    
//    Eric Brugger, Mon Nov 18 07:47:00 PST 2002
//    I added support for keyframing.
//
//    Eric Brugger, Mon Jan 13 16:29:23 PST 2003
//    I added an nDimensions argument to GetPlotLimits.
//
//    Brad Whitlock, Fri Jan 17 09:59:55 PDT 2003
//    I added a default argument to the UpdatePlots method.
//
//    Kathleen Bonnell, Tue Mar  4 16:46:04 PST 2003  
//    Added DeletePlot method.
//
//    Brad Whitlock, Thu Apr 10 09:57:51 PDT 2003
//    I added PromoteOperator, DemoteOperator, and RemoveOperator methods and
//    added 3 new arguments to SetActivePlots.
//
//    Brad Whitlock, Wed Jul 16 13:15:58 PST 2003
//    I added CreateNode and SetFromNode.
//
//    Kathleen Bonnell, Thu Aug 28 10:03:42 PDT 2003
//    Added CanMeshPlotBeOpaque.
//
//    Kathleen Bonnell, Thu Sep 11 11:47:16 PDT 2003 
//    Added optional bool argument to AddOperator, indicates whether the
//    operator should be initialized from its Default or Client atts. 
//
//    Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//    Renamed GetPlotID to GetActivePlotIDs.
//
//    Brad Whitlock, Fri Oct 24 16:19:25 PST 2003
//    Added UpdateExpressionList.
//
//    Mark C. Miller, Wed Oct 29 14:36:35 PST 2003
//    Added method to TransmutePlots() to support smooth transitions into
//    and out of scalable rendering.
//
//    Mark C. Miller Wed Nov 19 13:28:08 PST 2003
//    Added method to FindCompatiblePlot() to support pushing time
//    varying SIL to client as necessary.
//
//    Brad Whitlock, Wed Dec 31 14:09:37 PST 2003
//    I added UpdateExpressionListUsingDB.
//
//    Brad Whitlock, Wed Jan 7 14:24:09 PST 2004
//    I added GetNumVisiblePlots.
//
//    Brad Whitlock, Fri Jan 23 16:34:52 PST 2004
//    I added support for multiple time sliders.
//
//    Jeremy Meredith, Thu Mar 25 17:19:11 PST 2004
//    Added support for simulations.
//
//    Eric Brugger, Tue Mar 30 16:07:32 PST 2004
//    Added ClearActors and GetMaintainDataMode.
//
//    Mark C. Miller, Wed Apr 14 10:44:42 PDT 2004
//    I added the method, GetMeshVarNameFromActivePlots, to support the
//    catch-all mesh feature which computes a mesh variable name from the
//    active plots
//
//    Eric Brugger, Fri Apr 16 18:32:47 PDT 2004
//    I added TimeSliderExists.
//
//    Brad Whitlock, Mon Apr 19 09:04:41 PDT 2004
//    I added ValidateTimeSlider.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Added method to GetNumberOfCells
//
//    Kathleen Bonnell, Tue Jun  1 17:57:52 PDT 2004 
//    Added args to StartPick. 
//
//    Mark C. Miller, Tue Jun  8 14:43:36 PDT 2004
//    Added GetWindowId
//
//    Brad Whitlock, Tue Jul 27 10:34:43 PDT 2004
//    Added ResizeTimeSliders.
//
//    Brad Whitlock, Wed Feb 2 16:06:09 PST 2005
//    Added NotActivePlotList to isolate logic for checking if the plot
//    list belongs to the active window. Changed AlterTimeSlider.
//
//    Brad Whitlock, Tue Feb 8 14:53:18 PST 2005
//    Added return value for ValidateTimeSlider.
//
//    Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//    Added DoAllPlotsAxesHaveSameUnits
//
//    Brad Whitlock, Wed Jan 11 14:54:42 PST 2006
//    Added SessionContainsErrors.
//
//    Brad Whitlock, Thu Nov 9 16:47:17 PST 2006
//    Added an argument to CreateNode and SetFromNode.
//
//    Brad Whitlock, Mon Feb 12 17:40:22 PST 2007
//    Added ViewerBase base class.
//
//    Kathleen Bonnell, Thu Mar 22 19:36:05 PDT 2007 
//    Added SetScaleMode.
//
//    Kathleen Bonnell, Wed May  9 17:27:40 PDT 2007 
//    Added GetScaleMode, CanDoLogViewScaling.
//
//    Ellen Tarwater, Thurs Sept 27, 2007
//    Added CopyActivePlots
//
//    Kathleen Bonnell, Fri Sep 28 08:34:36 PDT 2007 
//    Added scaleModeSet.
//
//    Ellen Tarwater October 12, 2007
//    Added flag to RealizePlots
//
//    Ellen Tarwater, Thurs, Dec 6, 2007
//    Added SetPlotFollowsTime & followsTime flag to ViewerPlotListElement
//
//    Brad Whitlock, Tue Jan 22 13:54:58 PST 2008
//    Added a returnDefault bool argument to GetDefaultSILRestriction. Removed
//    followsTime from the ViewerPlotListElement because it's not necessary.
//    Added inheritSILRestriction bool argument to AddPlot.
//
//    Brad Whitlock, Wed Dec 10 14:52:46 PST 2008
//    I made it use an animation attributes object.
//
//    Brad Whitlock, Thu Jan  8 15:08:33 PST 2009
//    I added methods to help deal with the plot information.
//
//    Hank Childs, Mon Feb  2 16:02:04 PST 2009
//    Added argument to GetActivePlotIDs.
//
//    Kathleen Bonnell, Tue Mar  3 15:03:19 PST 2009
//    Renamed CanDoLogViewScaling to PermitsLogViewScaling.
//
//    Cyrus Harrison, Tue Apr 14 13:34:15 PDT 2009
//    Modified ReplaceDatabase to add option for replacing only active plots.
//
//    Brad Whitlock, Tue Oct 20 14:55:44 PDT 2009
//    I added methods to manipulate the plot list order.
//
//    Brad Whitlock, Mon Oct 26 15:44:55 PDT 2009
//    I added DeleteAllPlots and I added a default argument to DeleteActivePlots.
//
//    Jeremy Meredith, Wed Feb  3 15:35:08 EST 2010
//    Removed maintain data; moved maintain view from Global settings
//    (Main window) to per-window Window Information (View window).
//
//    Eric Brugger, Fri Feb 12 15:34:07 PST 2010
//    I added maxPlotNumber, which is used to tell the ViewerPlot what
//    number to use to start numbering plots from.
//
//    Eric Brugger, Fri Oct 28 09:52:40 PDT 2011
//    Add a multi resolution display capability for AMR data.
//
//    Brad Whitlock, Thu Sep 12 16:32:35 PDT 2013
//    Added plot animation methods.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
//    Kevin Griffin, Wed May 11 16:47:05 PDT 2016
//    Added a default argument to DeleteActivePlots to determine if the current
//    active plots should be considered when updating the expression list.
//
//    Alister Maguire, Mon Jun  4 15:13:43 PDT 2018
//    Added SetViewScale. 
//
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Added operator keyframing.
//
// ****************************************************************************


struct ViewerPlotListElement
{
    ViewerPlot *plot;
    bool       hidden;
    bool       active;
    bool       realized;
    int        id;
};
    
class VIEWERCORE_API ViewerPlotList : public ViewerBase
{
public:
    ViewerPlotList(ViewerWindow *const viewerWindow);
    virtual ~ViewerPlotList();

    void SetActiveTimeSlider(const std::string &ts);
    const std::string &GetActiveTimeSlider() const;
    bool HasActiveTimeSlider() const;
    void GetTimeSliderInformation(int &activeTimeSlider,
                                  stringVector &timeSliders,
                                  intVector &timeSliderCurrentStates);
    void CreateTimeSlider(const std::string &ts, int state);
    bool AlterTimeSlider(const std::string &ts, bool = true);
    bool DeleteTimeSlider(const std::string &ts, bool update = true);
    void GetTimeSliderStates(const std::string &ts, int &state, int &nStates) const;
    bool TimeSliderExists(const std::string &ts) const;
    bool ValidateTimeSlider();
    int  ResizeTimeSliders(const stringVector &sliders, bool clearCache);
    void ForwardStep();
    void BackwardStep();
    void SetTimeSliderState(int state);
    void UpdateFrame(bool updatePlotStates = true);
    bool UpdateFrameForPlots(const intVector &);
    void RegenerateFrame();

    bool HasAnimatingPlots() const;
    bool AnimationStep();
    void StartPlotAnimation(const intVector &plotIds);
    void StopPlotAnimation(const intVector &plotIds);

    void SetAnimationAttributes(const AnimationAttributes &);
    const AnimationAttributes &GetAnimationAttributes() const;

    void ActivateSource(const std::string &database, const EngineKey &ek);
    void SetHostDatabaseName(const std::string &database);
    void SetDatabaseName(const std::string &database);
    int  CloseDatabase(const std::string &database);
    void SetEngineKey(const EngineKey &ek);
    const std::string &GetHostDatabaseName() const;
    const std::string &GetDatabaseName() const;
    const std::string &GetHostName() const;
    const EngineKey &GetEngineKey() const;
    bool GetPlotHostDatabase(std::string &h, std::string &d) const;
    bool FileInUse(const std::string &host,
                   const std::string &database) const;

    int  GetNumPlots() const;
    int  GetNumRealizedPlots() const;
    int  GetNumVisiblePlots() const;
    int  AddPlot(int type, const std::string &var, bool replacePlots = false,
                 bool applyOperators = false, bool inheritSILRestriction = false,
                 bool applySelections = false, DataNode *attributesNode = 0,
                 int embeddedPlotId = -1);

    long long GetNumberOfCells(bool polysOnly = false) const;

    // 
    // Keyframing methods
    //
    void SetPlotRange(int plotId, int frame0, int frame1);
    void DeletePlotKeyframe(int plotId, int frame);
    void MovePlotKeyframe(int plotId, int oldFrame, int newFrame);
    void DeleteOperatorKeyframe(int plotId, int operatorId, int frame);
    void MoveOperatorKeyframe(int plotId, int operatorId,
                              int oldFrame, int newFrame);
    void SetPlotDatabaseState(int plotId, int frame, int state);
    void DeletePlotDatabaseKeyframe(int plotId, int frame);
    void MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame);
    void SetKeyframeMode(const bool mode);
    bool GetKeyframeMode() const;
    void SetNKeyframes(int nFrames);
    int  GetNKeyframes() const;
    bool GetNKeyframesWasUserSet() const;


    StringStringMap CopyFrom(const ViewerPlotList *pl, bool copyPlots);
    void ClearPlots(bool clearAll = true);
    void ClearActors();
    void TransmutePlots(bool turningOffScalableRendering);
    void DeleteActivePlots(bool doUpdates = true, bool considerPlots = true);
    void DeleteAllPlots(bool doUpdates);

    int  FindCompatiblePlot(ViewerPlot *);

    void DeletePlot(ViewerPlot *, bool);
    void HideActivePlots();
    void RealizePlots(bool drawAllPlots = true);
    void SetErrorFlagAllPlots(bool errorFlag);
    void SetPlotVar(const std::string &variable);
    void SetPlotAtts(const int plotType);
    void SetPlotOperatorAtts(const int operatorType,
                             bool activeWindow = false,
                             bool applyToAllWindows = false,
                             bool applyToAllPlots = false);
    bool ReplaceDatabase(const EngineKey &ek, const std::string &database,
                         int timeState, bool setTimeState, bool onlyReplaceSame,
                         bool onlyReplaceActive);
    void OverlayDatabase(const EngineKey &ek,
                         const std::string &database, int timeState);
    void UpdateSILRestriction(const EngineKey &ek, const std::string &database);

    void SetActivePlots(const intVector &activePlots,
                        const intVector &activeOperators,
                        const intVector &expandedPlots,
                        bool moreThanPlotsValid);
    void CopyActivePlots();
    void SetPlotFollowsTime(bool);
    void SetPlotDescription(int plotId, const std::string &description);
    void SetPlotOrderToFirst(int plotId);
    void SetPlotOrderToLast(int plotId);
    void MovePlotOrderTowardFirst(int plotId);
    void MovePlotOrderTowardLast(int plotId);

    void SetPlotSILRestriction(bool applyToAll = false);

    void AddOperator(const int type, bool applyToAll = false, 
                     const bool fromDefault = true);
    void PromoteOperator(const int operatorId, bool applyToAll = false);
    void DemoteOperator(const int operatorId, bool applyToAll = false);
    void RemoveOperator(const int operatorId, bool applyToAll = false);
    void RemoveLastOperator(bool applyToAll = false);
    void RemoveAllOperators(bool applyToAll = false);

    bool UpdateColorTable(const std::string &ctName);

    bool NotActivePlotList() const;
    void UpdatePlotAtts(bool=true) const;
    void UpdatePlotInformation() const;
    void GetPlotAtts(std::vector<std::string> &plotNames,
                     std::vector<const char*> &pluginIDsList,
                     std::vector<EngineKey> &engineKeysList,
                     intVector &plotIdsList,
                     std::vector<const AttributeSubject*> &attsList) const; 
    void UpdatePlotList() const;
    void UpdateSILRestrictionAtts();
    void InterruptUpdatePlotList();
    void UpdateExpressionList(bool considerPlots, bool update = true);
    void UpdateExpressionListUsingDB(const std::string &host,
                                     const std::string &db, int ts) const;

    void GetPlotLimits(int nDimensions, double *limits) const;
    void SetSpatialExtentsType(avtExtentType);

    bool SetBackgroundColor(const double *);
    bool SetForegroundColor(const double *);

    void HandleTool(const avtToolInterface &ti, bool applyToAll = false);
    bool InitializeTool(avtToolInterface &ti);

    void ClearPipelines();

    void StartPick(const bool, const bool);
    void StopPick();
    void ResetNetworkIds(const EngineKey &key);
    std::string GetVarName();
    ViewerPlot *GetPlot(const int id) const;
    void GetActivePlotIDs(intVector &, bool onlyRealizedAndUnhidden = true) const;

    int GetWindowId() const;

    static void ClearDefaultSILRestrictions(const std::string &host,
                                            const std::string &database);
    avtSILRestriction_p GetDefaultSILRestriction(const std::string &host,
                                                 const std::string &database,
                                                 const std::string &var,
                                                 int state,
                                                 bool returnDefault=false);
    bool SetFullFrameScaling(bool, double *);
    bool SetViewScale(const double);

    void CreateNode(DataNode *, const std::map<std::string, std::string> &);
    bool SetFromNode(DataNode *, const std::map<std::string, std::string> &,
                     const std::string &);
    static bool SessionContainsErrors(DataNode *);

    void SetScaleMode(ScaleMode ds, ScaleMode rs, WINDOW_MODE);
    void GetScaleMode(ScaleMode &ds, ScaleMode &rs, WINDOW_MODE);
    bool PermitsLogViewScaling(WINDOW_MODE);

    bool ShouldRefineData(double smallestCellSize) const;

    void AlternateDisplayChangedPlotAttributes(ViewerPlot *);
protected:
    bool ReplaceDatabaseHelper(const EngineKey &ek, const std::string &database,
                         int timeState, bool onlyReplaceSame,
                         bool onlyReplaceActive, bool justUpdateSIL);
    bool        AskForCorrelationPermission(const stringVector &dbs) const;
    bool        AllowAutomaticCorrelation(const stringVector &dbs) const;
    DatabaseCorrelation *GetMostSuitableCorrelation(const std::string &, bool);
    ViewerPlot *NewPlot(int type, const EngineKey &ek,
                        const std::string &host, const std::string &db,
                        const std::string &var, bool applyOperators,
                        bool inheritSILRestriction,
                        const char *optionalPlotName = 0);
    int         SimpleAddPlot(ViewerPlot *plot, bool replacePlots);
    void        SetNextState(int nextState, int boundary);
    bool        UpdatePlotStates();
    bool        UpdateSinglePlotState(ViewerPlot *plot);

    bool        ArePlotsUpToDate(const intVector &somePlots) const;
    bool        UpdatePlots(const intVector &somePlots, bool animating = false);
    void        UpdateWindow(const intVector &somePlots, bool immediateUpdate);

    static std::string SILRestrictionKey(const std::string &, const std::string &, int);
 
private:
    static SILRestrictionMap SILRestrictions;
    static int               lastPlotId;
    static int               maxPlotNumber;

    ViewerWindow           *window;

    EngineKey              engineKey;
    std::string            hostDatabaseName;
    std::string            hostName;
    std::string            databaseName;

    std::string            activeTimeSlider;
    StringIntMap           timeSliders;
    AnimationAttributes    animationAtts;

    ScaleMode              xScaleMode;
    ScaleMode              yScaleMode;
    bool                   scaleModeSet;

    bool                   keyframeMode;
    bool                   nKeyframesWasUserSet;
    int                    nKeyframes;

    ViewerPlotListElement *plots;
    int                    nPlots;
    int                    nPlotsAlloc;
    bool                   interrupted;

    double                 bgColor[3];
    double                 fgColor[3];
    avtExtentType          spatialExtentsType;


    void                   CanMeshPlotBeOpaque();
    void                   GetMeshVarNameForActivePlots(
                               const std::string &host,
                               const std::string &db,
                               std::string &meshName) const;
    std::string            GetNamedSelection(const std::string &plotName) const;

};

#endif
