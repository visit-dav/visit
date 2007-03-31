// ************************************************************************* // 
//                                ViewerPlot.h                               //
// ************************************************************************* //

#ifndef VIEWER_PLOT_H
#define VIEWER_PLOT_H
#include <viewer_exports.h>
#include <avtDataObjectReader.h>
#include <avtActor.h>
#include <avtTypes.h>
#include <avtPlot.h>

// Forward declarations
class AttributeSubject;
class AttributeSubjectMap;
class DatabaseAttributes;
class DataNode;
class EngineProxy;
class PickAttributes;
class PlotQueryInfo;
class ViewerPlotList;
class ViewerPlotPluginInfo;
class ViewerOperator;
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
// ****************************************************************************

class VIEWER_API ViewerPlot
{
  public:
    ViewerPlot(const int type_,
               ViewerPlotPluginInfo *viewerPluginInfo_, const char *hostName_,
               const char *databaseName_, const char *variableName_,
               avtSILRestriction_p silr, const int frame0_, const int frame1_,
               const int nStates);
    virtual ~ViewerPlot();

    void SetFrameRange(const int f0, const int f1);
    bool IsInFrameRange(const int frame) const;
    int GetBeginFrame() const;
    int GetEndFrame() const;
    const int *GetKeyframeIndices(int &) const;
    void DeleteKeyframe(const int frame);
    void MoveKeyframe(int oldFrame, int newFrame);

    void SetHostDatabaseName(const char *host, const char *database);
    const char *GetHostName() const;
    const char *GetDatabaseName() const;
    const char *GetPlotName() const;
    const char *GetPluginID() const;

    bool SetVariableName(const char *name);
    const char *GetVariableName() const;

    void SetDatabaseAtts(const AttributeSubjectMap *atts);
    const AttributeSubjectMap *GetDatabaseAtts() const;
    void SetDatabaseState(const int frame, const int state);
    int GetDatabaseState(const int frame) const;
    const int *GetDatabaseKeyframeIndices(int &) const;
    void DeleteDatabaseKeyframe(const int frame);
    void MoveDatabaseKeyframe(int oldFrame, int newFrame);

    void SetSILRestriction(avtSILRestriction_p silr);
    avtSILRestriction_p GetSILRestriction() const;

    int  AddOperator(const int type, const bool fromDefault = true);
    bool PromoteOperator(const int operatorIndex);
    bool DemoteOperator(const int operatorIndex);
    bool RemoveOperator(const int operatorIndex);
    void RemoveLastOperator();
    void RemoveAllOperators();
    void SetOperatorAttsFromClient(const int type);

    int GetNOperators() const;
    ViewerOperator *GetOperator(const int i) const;

    void SetActor(const int frame, const avtActor_p actor);
    void SetReader(const int frame, avtDataObjectReader_p reader);

    avtActor_p &GetActor(const int frame) const;
    avtDataObjectReader_p &GetReader(const int frame) const;
    bool NoActorExists(const int frame) const;

    void CreateActor(const int frame, bool createNew = true,
                                      bool turningOffScalableRendering = true);
    void ClearActors();
    void ClearActors(const int f0, const int f1);

    void TransmuteActor(int frame, bool turningOffScalableRendering);

    int GetSpatialDimension(const int frame) const;
    double *GetSpatialExtents(const int frame, 
                              avtExtentType = AVT_UNKNOWN_EXTENT_TYPE) const;
    void SetSpatialExtentsType(avtExtentType);
    double *GetDataExtents(const int frame) const;

    bool ExecuteEngineRPC(const int frame);

    int GetType() const;

    bool GetErrorFlag() const;
    void SetErrorFlag(bool val);

    void SetClientAttsFromPlot(int frame);
    void SetPlotAttsFromClient();
    void SetPlotAttsFromClient(int frame);
    bool SetPlotAtts(const AttributeSubject *atts);
    bool SetPlotAtts(int frame, const AttributeSubject *atts);
    bool SetPlotAtts(const AttributeSubjectMap *atts);
    const AttributeSubjectMap *GetPlotAtts() const;
    const AttributeSubject *GetCurrentPlotAtts() const;

    bool UpdateColorTable(const char *ctName);
    bool SetBackgroundColor(const double *);
    bool SetForegroundColor(const double *);

    bool HandleTool(const avtToolInterface &ti);
    bool HandleTool(int frame, const avtToolInterface &ti);
    bool InitializeTool(avtToolInterface &ti);

    int  GetNetworkID() const;
    void SetNetworkID(int id);

    void SetActiveOperatorIndex(int index);
    int  GetActiveOperatorIndex() const;
    void SetExpanded(bool val);
    bool GetExpanded() const;

    bool StartPick(const int);
    void StopPick(void);

    PlotQueryInfo* GetPlotQueryInfo();
    avtVarType     GetVarType();

    void CreateNode(DataNode *);
    void SetFromNode(DataNode *);
    void SetOpaqueMeshIsAppropriate(bool);
    bool IsMesh(void);

    void RegisterViewerPlotList(ViewerPlotList *vpl)
                                 { viewerPlotList = vpl; };

  protected:
    bool MoveOperator(const int operatorIndex, bool promote);

    int                     frame0, frame1;

  private:
    void CheckCache(const int f0, const int f1, const bool force);

    ViewerPlotList         *viewerPlotList;
    int                     type;
    ViewerPlotPluginInfo   *viewerPluginInfo;

    int                     networkID;
    char                   *hostName;
    char                   *databaseName;
    char                   *variableName;
    AttributeSubjectMap    *databaseAtts;
    DatabaseAttributes     *curDatabaseAtts;
    avtSILRestriction_p     silr;
    avtExtentType           spatialExtentsType;

    double                  bgColor[3];
    double                  fgColor[3];

    int                     nOperators;
    int                     nOperatorsAlloc;
    ViewerOperator        **operators;
    int                     activeOperatorIndex;

    bool                    expandedFlag;
    bool                    errorFlag;
    bool                    isMesh;

    AttributeSubjectMap    *plotAtts;
    AttributeSubject       *curPlotAtts;

    avtPlot_p              *plotList;
    avtActor_p             *actorList;
    avtDataObjectReader_p  *readerList;

    PlotQueryInfo         *queryAtts;      

    static avtActor_p             nullActor;
    static avtDataObjectReader_p  nullReader;
};

#endif
