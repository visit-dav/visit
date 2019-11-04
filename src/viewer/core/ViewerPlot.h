// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                ViewerPlot.h                               //
// ************************************************************************* //

#ifndef VIEWER_PLOT_H
#define VIEWER_PLOT_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <avtDataObjectReader.h>
#include <avtActor.h>
#include <avtPlotMetaData.h>
#include <avtTypes.h>
#include <avtPlot.h>
#include <EngineKey.h>
#include <ExpressionList.h>

#include <string>
#include <vector>

// Forward declarations
class AttributeSubject;
class AttributeSubjectMap;
class DatabaseAttributes;
class DataNode;
class EngineKey;
class EngineProxy;
class ObserverToCallback;
class PickAttributes;
class Plot;
class PlotInfoAttributes;
class PlotQueryInfo;
class ViewerOperator;
class ViewerPlotList;
class ViewerPlotPluginInfo;
class avtDatabaseMetaData;
class avtToolInterface;


// ****************************************************************************
//  Class: ViewerPlot
//
//  Purpose:
//    ViewerPlot is a visualization that exists over multiple frames.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Hank Childs, Tue Oct 17 07:51:14 PDT 2000
//    Reference counted plots and added ownership of dataset reader to
//    prevent leaks.
//
//    Brad Whitlock, Wed Nov 8 15:55:58 PST 2000
//    Added a static avtPlot_p;
//
//    Kathleen Bonnell, Fri Dec  1 15:33:33 PST 2000
//    Added new pure virtual function 'Setup'.
//
//    Brad Whitlock, Fri Dec 8 17:08:27 PST 2000
//    Added required arguments to the constructor.
//
//    Eric Brugger, Wed Dec 27 15:15:00 PST 2000
//    I added the method GetSpatialExtents.
//
//    Hank Childs, Thu Dec 28 15:23:42 PST 2000
//    Replaced avtPlot_p in favor of avtActor_p, accounted for new plot scheme.
//
//    Kathleen Bonnell, Thu Feb 22 16:38:31 PST 2001
//    Added method GetDataExtents.
//
//    Eric Brugger, Tue Mar 08 15:01:50 PST 2001
//    I modified the class for use with plot plugins.  I also modified the
//    class so that it is no longer abstract.
//
//    Brad Whitlock, Tue Apr 24 14:42:51 PST 2001
//    Added error flag coding.
//
//    Eric Brugger, Tue Apr 24 16:13:41 PDT 2001
//    I added the method GetSpatialDimension.
//
//    Jeremy Meredith, Tue Jun  5 20:27:09 PDT 2001
//    Added nullReader, GetReader.
//
//    Hank Childs, Wed Jun 13 11:05:41 PDT 2001
//    Added SIL restrictions.
//
//    Brad Whitlock, Thu Jun 14 16:38:03 PST 2001
//    Added the UpdateColorTable method.
//
//    Hank Childs, Mon Jul 23 11:07:03 PDT 2001
//    Removed method Setup that was for a hack with the filled boundary.
//
//    Jeremy Meredith, Thu Jul 26 03:18:19 PDT 2001
//    Removed references to OperType.
//
//    Jeremy Meredith, Thu Jul 26 09:53:57 PDT 2001
//    Renamed plugin info to include the word "plot".
//
//    Eric Brugger, Fri Aug 31 09:40:19 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Brad Whitlock, Fri Sep 21 11:56:11 PDT 2001
//    I changed the return type of SetVariableName.
//
//    Kathleen Bonnell, Wed Sep 26 15:02:36 PDT 2001
//    Added SetForeground/Background methods and members to store them.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Brad Whitlock, Tue Oct 9 14:51:53 PST 2001
//    Added a method to handle tools.
//
//    Jeremy Meredith, Fri Nov  9 10:08:20 PST 2001
//    Added a network id.
//
//    Kathleen Bonnell, Thu Nov 15 16:06:43 PST 2001
//    Added Pick, StartPick, StopPick.
//
//    Brad Whitlock, Mon Feb 11 14:25:50 PST 2002
//    Added a new method to initialize tools.
//
//    Brad Whitlock, Wed Mar 6 16:51:33 PST 2002
//    Added a GetPlotName method.
//
//    Brad Whitlock, Fri Mar 8 09:40:05 PDT 2002
//    Added methods to set/get the plot attributes.
//
//    Kathleen Bonnell, Wed Jun 19 17:50:21 PDT 2002
//    Added methods GetVarType and GetPlotQueryInfo and
//    member queryAtts.
//
//    Hank Childs, Mon Jul 15 15:58:39 PDT 2002
//    Added support for different flavors of spatial extents.
//
//    Brad Whitlock, Mon Nov 4 10:45:31 PDT 2002
//    Added a method to get the frame range.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//    Removed Pick method, now handled by ViewerQueryManager.
//
//    Eric Brugger, Mon Nov 18 09:06:49 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Fri Apr 4 10:24:03 PDT 2003
//    I added the nStates argument to the constructor.
//
//    Brad Whitlock, Thu Apr 10 10:15:21 PDT 2003
//    I added MoveOperator, PromoteOperator, DemoteOperator, and
//    RemoveOperator methods. I also added methods to set/get the new
//    activeOperator index and the expanded flag.
//
//    Brad Whitlock, Wed Jul 16 13:29:03 PST 2003
//    Added CreateNode and SetFromNode.
//
//    Kathleen Bonnell, Thu Aug 28 10:10:35 PDT 2003
//    Added IsMesh and SetOpaqueMeshIsAppropriate.
//
//    Kathleen Bonnell, Thu Sep 11 11:50:02 PDT 2003
//    Added optional bool arg to 'AddOperator', indicates whether the operator
//    should be initialized from its default or client atts.
//
//    Mark C. Miller, Wed Oct 29 14:36:35 PST 2003
//    Added method to TransmuteActor() to support smooth transitions into
//    and out of scalable rendering. Also, added optional bools to CreateActor
//    for switching into scalable rendering.
//
//    Brad Whitlock, Thu Jan 29 15:54:23 PST 2004
//    I added GetState and SetState methods to get/set the state that the
//    plot shows in the vis window. I also added methods to set whether or
//    not the plot follows time for its time slider. If a plot does not follow
//    a time slider then it never changes when time changes. I changed the
//    constructor prototype.
//
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004
//    Removed GetDataExtents, no longer used.
//
//    Jeremy Meredith, Thu Mar 25 16:33:55 PST 2004
//    Added simulation support by using an engine key to map this plot
//    to the engine used to create it.
//
//    Eric Brugger, Tue Mar 30 15:08:09 PST 2004
//    Added data extents.
//
//    Kathleen Bonnell, Wed Mar 31 16:46:13 PST 2004
//    Added clonedNetworkId, which when not -1 indicates that this plot
//    should use a cloned network instead of creating a new one.
//
//    Brad Whitlock, Thu Apr 1 16:22:09 PST 2004
//    I added a copy constructor and made changes to make keyframing work
//    again.
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added actorHasNoData arg. to CreateActor. Eliminated default values for
//    other args to CreateActor
//
//    Kathleen Bonnell, Tue Jun  1 17:57:52 PDT 2004
//    Added bool args to StartPick.
//
//    Mark C. Miller, Tue Jun  8 14:43:36 PDT 2004
//    Added GetWindowId
//
//    Kathleen Bonnell, Wed Aug 25 08:31:44 PDT 2004
//    Added GetMeshType.
//
//    Kathleen Bonnell, Fri Oct 29 13:57:38 PDT 2004
//    Added GetBlockOrigin.
//
//    Kathleen Bonnell, Tue Jan 11 16:16:48 PST 2005
//    Added isLabel.
//
//    Brad Whitlock, Fri Feb 18 10:08:36 PDT 2005
//    Added GetExpressions method.
//
//    Kathleen Bonnell, Tue Jul  5 14:46:52 PDT 2005
//    Added GetRealVarType method.
//
//    Mark C. Miller, Wed Nov  9 12:35:15 PST 2005
//    Added PrepareCacheForReplace
//
//    Kathleen Bonnell, Thu Nov 17 12:00:23 PST 2005
//    Added GetTopologicalDimension method.
//
//    Brad Whitlock, Wed Jan 11 14:52:22 PST 2006
//    I added SessionContainsErrors.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added GetPlotInfoAtts.
//
//    Brad Whitlock, Mon Feb 12 12:18:39 PDT 2007
//    I made it inherit ViewerBase and I added support for alternate displays.
//
//    Brad Whitlock, Mon Mar 19 17:58:59 PST 2007
//    Changed GetPlotName to GetPlotTypeName and added new GetPlotName.
//
//    Kathleen Bonnell, Thu Mar 22 19:44:41 PDT 2007
//    Added SetScaleMode, xScaleMode, yScaleMode in support of log-scaled views
//
//    Kathleen Bonnell, Fri May 11 15:06:40 PDT 2007
//    Added WINDOW_MODE arg to SetScaleMode, Add CanDoLogViewScaling().
//
//    Hank Childs, Thu Aug 30 15:57:55 PDT 2007
//    Added AdaptsToAnyWindowMode.
//
//    Jeremy Meredith, Tue Feb 19 14:30:14 EST 2008
//    Save the meshname for the plot's variable.  This is useful for
//    operators that create their own variables.
//
//    Brad Whitlock, Tue Apr 29 15:34:48 PDT 2008
//    Added GetMenuName.
//
//    Brad Whitlock, Wed Jan  7 15:32:49 PST 2009
//    Added UpdatePlotInformation and removed GetPlotInfoAtts.
//
//    Kathleen Bonnell, Tue Mar  3 15:03:19 PST 2009
//    Renamed CanDoLogViewScaling to PermitsLogViewScaling.
//
//    Hank Childs, Tue Jul 14 14:28:36 PDT 2009
//    Added methods and data member for named selections.
//
//    Eric Brugger, Fri Feb 12 15:28:11 PST 2010
//    Added SetNumPlotsCreated.
//
//    Brad Whitlock, Mon Aug 22 10:59:48 PDT 2011
//    I moved some selection method bodies into the C file.
//
//    Kathleen Biagas, Wed Feb 29 07:46:10 MST 2012
//    Added GetExtraInfoForPick.
//
//    Brad Whitlock, Thu Sep 12 16:32:35 PDT 2013
//    Added plot animation methods.
//
//    Burlen Loring, Wed Aug 26 14:04:16 PDT 2015
//    clean up a compiler warning
//
//    Alister Maguire, Tue Jun  5 09:13:10 PDT 2018
//    Added SetViewScale.
//
//    Kathleen Biagas, Thu Oct 31 12:32:57 MST 2019
//    Added PlotHasBeenGlyphed.
//
// ****************************************************************************

class VIEWERCORE_API ViewerPlot : public ViewerBase
{
public:
    ViewerPlot(const int type_,
               ViewerPlotPluginInfo *viewerPluginInfo_,
               const EngineKey &ek_,
               const std::string &hostName_,
               const std::string &databaseName_,
               const std::string &variableName_,
               avtSILRestriction_p silr,
               const int plotState,    // Initial database state for the plot
               const int nStates,      // Number of total database states
               const int cacheIndex,   // Initial active cache index
               const int nCacheEntries // Number of cache entries
              );
    ViewerPlot(const ViewerPlot &);
    virtual ~ViewerPlot();
    void operator = (const ViewerPlot &obj);

    //
    // Methods to identify the plot plugin.
    //
    int         GetType() const;
    const char *GetPluginID() const;
    const char *GetName() const;
    const char *GetMenuName() const;

    //
    // Methods to set/get the name of the plot, which is an identifier that
    // can be used to lookup the plot.
    //
    const std::string &GetPlotName() const;
    void SetPlotName(const std::string &);

    // Returns whether the plot provides a legend.
    bool        ProvidesLegend() const;

    //
    // Get/Set the plot's database and variable.
    //
    void SetHostDatabaseName(const EngineKey &hostKey,
                             const std::string &database);
    const std::string &GetHostName() const;
    const std::string &GetDatabaseName() const;
    std::string GetSource() const;
    const EngineKey &GetEngineKey() const;
    bool SetVariableName(const std::string &name);
    const std::string &GetVariableName() const;
    void SetVariableDescription(const std::string &);
    void UpdateVariableDescription( bool resest = false );
    const std::string &GetVariableDescription() const;
    avtCentering GetVariableCentering() const;
    const std::string &GetMeshName() const;
    const std::string &GetPlotDescription() const;
    void SetPlotDescription(const std::string &);

    //
    // Get/Set the SIL restriction, get metadata, get expressions.
    //
    void SetSILRestriction(avtSILRestriction_p silr);
    void SetSILRestriction(avtSILRestriction_p silr, bool checkActors);
    avtSILRestriction_p GetSILRestriction() const;
    const avtDatabaseMetaData *GetMetaData() const;
    ExpressionList GetExpressions() const;
    void SetNamedSelection(const std::string &s);
    const std::string &GetNamedSelection() const;
    avtPlotMetaData GetPlotMetaData() const;

    //
    // Returns the database state that the plot currently displays.
    //
    int GetState() const;

    //
    // Get/Set the cache index, which is where the plot is in time.
    //
    void SetCacheIndex(int cacheIndex_);
    int  GetCacheIndex() const;

    //
    // Get/Set whether the plot allows its cache index to be changed.
    //
    bool FollowsTime() const;
    void SetFollowsTime(bool);

    //
    // Plot animation
    //
    bool SupportsAnimation() const;
    bool GetAnimating() const;
    bool SetAnimating(bool);
    bool AnimationStep();

    //
    // Set/Get plot attributes and keyframes.
    //
    void SetClientAttsFromPlot();
    void SetPlotAttsFromClient();
    bool SetPlotAtts(const AttributeSubject *atts);
    const AttributeSubject *GetPlotAtts() const;
    void DeleteKeyframe(const int cacheIndex);
    void MoveKeyframe(int oldIndex, int newIndex);

    //
    // Set/Get database keyframes.
    //
    void AddDatabaseKeyframe(int index, int state);
    void DeleteDatabaseKeyframe(const int index);
    void MoveDatabaseKeyframe(int oldIndex, int newIndex);
    void SetKeyframeMode(bool);

    //
    // Manage operators.
    //
    int  AddOperator(const int type, const bool fromDefault = true);
    bool PromoteOperator(const int operatorIndex);
    bool DemoteOperator(const int operatorIndex);
    bool RemoveOperator(const int operatorIndex);
    void RemoveLastOperator();
    void RemoveAllOperators();
     void SetOperatorAttsFromClient(const int type,
                                   const bool activePlot,
                                   const bool applyToAll);
    int  GetNOperators() const;
    ViewerOperator *GetOperator(const int i) const;
    void SetActiveOperatorIndex(int index);
    int  GetActiveOperatorIndex() const;
    void SetExpanded(bool val);
    bool GetExpanded() const;

    //
    // Manage the actor,reader,plot caches.
    //
    void SetRange(const int i0, const int i1);
    void UpdateCacheSize(bool kfMode, bool clearActors, int newsize=-1);
    bool IsInRange() const;
    avtActor_p &GetActor() const;
    avtDataObjectReader_p &GetReader() const;
    bool NoActorExists() const;
    void CreateActor(bool createNew,
                     bool turningOffScalableRendering,
                     bool &actorHasNoData);
    void ClearActors();
    void ClearCurrentActor();
    void TransmuteActor(bool turningOffScalableRendering);
    void PrepareCacheForReplace(int newCacheIndex, int newNumStates,
                                bool kfMode);

    bool ExecuteEngineRPC();


    bool HandleTool(const avtToolInterface &ti);
    bool InitializeTool(avtToolInterface &ti);
    bool StartPick(const bool, const bool);
    void StopPick();
    void ResetNetworkIds(const EngineKey &key);

    //
    // Get/Set specific plot attributes.
    //
    int GetBlockOrigin() const;
    int GetTopologicalDimension() const;
    int GetSpatialDimension() const;
    double *GetSpatialExtents(avtExtentType = AVT_UNKNOWN_EXTENT_TYPE) const;
    void SetSpatialExtentsType(avtExtentType);
    bool UpdateColorTable(const std::string &ctName);

    bool SetBackgroundColor(const double *);
    bool SetForegroundColor(const double *);
    bool GetErrorFlag() const;
    void SetErrorFlag(bool val);
    int GetEmbeddedPlotId() const;
    void SetEmbeddedPlotId(int id);
    int  GetNetworkID() const;
    void SetNetworkID(int id);
    PlotQueryInfo* GetPlotQueryInfo();
    avtVarType     GetVarType() const;
    avtVarType     GetVarType(const std::string &var) const;
    avtVarType     GetRealVarType() const;
    avtVarType     GetRealVarType(const std::string &var) const;
    void           SetOpaqueMeshIsAppropriate(bool);
    bool           IsMesh();
    bool           IsLabel();
    avtMeshType    GetMeshType() const;
    bool           AdaptsToAnyWindowMode();

    //
    // Let the plot manage its custom display, if one exists.
    //
    void AlternateDisplayHide();
    void AlternateDisplayShow();
    void AlternateDisplayIconify();
    void AlternateDisplayDeIconify();
    void AlternateDisplayClear();
    void AlternateDisplayUpdatePlotAttributes();
    void AlternateDisplaySetAllowClientUpdates(bool);
    bool AlternateDisplayAllowClientUpdates() const;

    //
    // Let the plot read/write itself to a config file.
    //
    void CreateNode(DataNode *);
    void SetFromNode(DataNode *, const std::string &);
    static bool SessionContainsErrors(DataNode *);
    void InitializePlot(Plot &p) const;
    void RegisterViewerPlotList(ViewerPlotList *vpl);

    bool CloneNetwork(void) { return clonedNetworkId != -1; }
    void SetCloneId(int id) { clonedNetworkId = id; }
    int  GetCloneId(void) { return clonedNetworkId; }

    int GetWindowId(void) const;

    void UpdateDataExtents();
    bool SetFullFrameScaling(bool, double *);
    bool SetViewScale(const double);

    void UpdatePlotInformation() const;

    void SetScaleMode(ScaleMode ds, ScaleMode rs, WINDOW_MODE wm);
    bool PermitsLogViewScaling(WINDOW_MODE wm);

    static void SetNumPlotsCreated(int);

    void GetExtraInfoForPick(MapNode &);

    bool PlotHasBeenGlyphed();

protected:
    void CopyHelper(const ViewerPlot &);
    bool MoveOperator(const int operatorIndex, bool promote);
    void SetActor(const avtActor_p actor);
    void ClearActors(const int f0, const int f1);
    void CheckCache(const int f0, const int f1, const bool force);
    void ResizeCache(int size);

    avtDataObjectReader_p GetDataObjectReader();
    avtDataObjectReader_p UseDataObjectReader(bool turningOffScalableRendering);

    void emitAlternateDisplayChangedPlotAttributes();
    static void emitAlternateDisplayChangedPlotAttributesCB(Subject *, void *cbdata);
private:
    ViewerPlotList         *viewerPlotList;
    int                     type;
    ViewerPlotPluginInfo   *viewerPluginInfo;
    std::string             plotName;
    std::string             plotDescription;

    void                   *alternateDisplay;
    ObserverToCallback     *alternateDisplayObserver;
    bool                    updateFromAlternateDisplay;
    bool                    alternateDisplayAllowsClientUpdates;

    int                     networkID;
    int                     embeddedPlotId;
    EngineKey               engineKey;
    std::string             hostName;
    std::string             databaseName;
    std::string             variableName;
    std::string             variableDescription;
    std::string             meshName;
    std::vector<double>     dataExtents;
    bool                    followsTime;

    int                     cacheIndex;
    int                     cacheSize;
    int                     beginCacheIndex, endCacheIndex;

    AttributeSubjectMap    *databaseAtts;
    DatabaseAttributes     *curDatabaseAtts;
    avtSILRestriction_p     silr;
    std::string             namedSelection;
    avtExtentType           spatialExtentsType;

    double                  bgColor[3];
    double                  fgColor[3];

    ScaleMode               xScaleMode;
    ScaleMode               yScaleMode;

    int                     nOperators;
    int                     nOperatorsAlloc;
    ViewerOperator        **operators;
    int                     activeOperatorIndex;

    bool                    expandedFlag;
    bool                    errorFlag;
    bool                    isMesh;
    bool                    isLabel;
    bool                    animating;

    AttributeSubjectMap    *plotAtts;
    AttributeSubject       *curPlotAtts;

    avtPlot_p              *plotList;
    avtActor_p             *actorList;
    avtDataObjectReader_p  *readerList;

    PlotQueryInfo         *queryAtts;

    static avtActor_p             nullActor;
    static avtDataObjectReader_p  nullReader;
    static std::vector<double>    nullDataExtents;
    static int                    numPlotsCreated;

    int                    clonedNetworkId;
};

#endif
