// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          ViewerQueryManager.h                             //
// ************************************************************************* //

#ifndef VIEWER_QUERY_MANAGER_H
#define VIEWER_QUERY_MANAGER_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <PickPointInfo.h>
#include <ColorAttribute.h>
#include <ViewerQuery.h>
#include <avtTypes.h>
#include <vector>
#include <Line.h>

// Forward declarations.
class DataNode;
class GlobalLineoutAttributes;
class Line;
class LineoutListItem;
class PickAttributes;
class QueryAttributes;
class QueryList;
class QueryOverTimeAttributes;
class ViewerWindow;
class ViewerPlot;
class ViewerPlotList;
class avtToolInterface;


typedef struct {
   ViewerWindow *origWin;
   Line     line;
   bool     fromDefault;
   std::vector<std::string> vars;
   int      resWinId;
   bool     forceSampling;
} CachedLineout;



// ****************************************************************************
//  Class: ViewerQueryManager
//
//  Purpose:
//    Maintains a list of queries.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002
//    Added method UpdateScaleFactor.
//
//    Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002  
//    Added methods to handle tools.  Made UpdateScaleFactor public. 
//
//    Kathleen Bonnell, Fri Jul 26 16:52:47 PDT 2002   
//    Remove unnecessary methods GetNumQueries, DeleteQuery. 
//
//    Brad Whitlock, Fri Sep 6 14:05:20 PST 2002
//    I added a QueryList object. It may move later as the viewer better
//    supports queries.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002
//    Added queryClientAtts member, DatabaseQuery method.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Added pickAtts member, Pick, UpdatePickAtts, ClearPickPoints methods.
//
//    Brad Whitlock, Wed Nov 20 13:24:49 PST 2002
//    Added an internal helper method.
//
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002  
//    Added methods LineQuery, Lineout, UpdateDesignator, and members 
//    baseDesignator, cycleDesignator and designator.
//
//    Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003  
//    Reworked to allow lineouts to be updated dynamically: added global
//    lineout atts; store lineouts via LineoutListItem, instead of directly.
//
//    Brad Whitlock, Wed Mar 12 14:39:33 PST 2003
//    I made it keep its own pointer to the operator factory.
//
//    Kathleen Bonnell, Fri Mar 14 17:11:42 PST 2003 
//    Add ViewerWindow and string args to Lineout method.  Add method to
//    determine a variable's type.  
//    
//    Kathleen Bonnell,  Wed Mar 26 14:37:23 PST 2003 
//    Added methods Start/StopPickMode, HandlePick; variables initialPick,
//    handlingPick, preparingPick and pickCache.
//    
//    Kathleen Bonnell, Wed May 14 17:09:14 PDT 2003 
//    Added PointQuery method.
//
//    Kathleen Bonnell, Fri Jun  6 15:53:58 PDT 2003 
//    Added another overloaded Lineout method.  This one is called from
//    ViewerWindow where it used to call AddQuery directly.  Made AddQuery
//    a private method.
//
//    Kathleen Bonnell, Fri Jun 27 16:03:57 PDT 2003 
//    Added arguments to StartPickMode. Added SetPickAttsFromClient, 
//    GetPickClientAtts.
//
//    Kathleen Bonnell, Wed Jul  9 08:58:27 PDT 2003 
//    Add method ViewDimChanged. 
//
//    Brad Whitlock, Tue Jul 22 10:14:14 PDT 2003
//    Added methods that let the query manager write its data to a config file.
//
//    Kathleen Bonnell, Wed Jul 23 16:56:15 PDT 2003 
//    Added 'samples' arg to LineQuery and Lineout methods.
//    Added 'arg1' and 'arg2' to DatabaseQuery.
//
//    Eric Brugger, Wed Aug 20 11:04:40 PDT 2003
//    Removed UpdateScaleFactor.
//
//    Kathleen Bonnell, Thu Sep 11 12:04:26 PDT 2003
//    Added optional bool arg to AddQuery and a Lineout method. 
//
//    Kathleen Bonnell, Mon Sep 15 13:09:19 PDT 2003 
//    Added InitializeQueryList.
//
//    Kathleen Bonnell, Wed Nov 19 15:41:11 PST 2003 
//    Added VerifyQueryVariables.
//
//    Kathleen Bonnell, Tue Dec  2 08:16:12 PST 2003 
//    Added ResetPickAttributes, ResetDesignator.  Added optional int args
//    to Pick and PointQuery.
//
//    Kathleen Bonnell, Wed Dec 17 14:45:22 PST 2003 
//    Added SetPickAttsFromDefault, SetDefaultPickAttsFromClient,
//    SetClientPickAttsFromDefault. 
//
//    Brad Whitlock, Tue Jan 6 10:30:26 PDT 2004
//    I added some new methods related to pick.
//
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004
//    Added support for queries-over-time.
//
//    Kathleen Bonnell, Thu Jun  3 14:51:32 PDT 2004 
//    Added VerifySingleInputQuery, VerifyMultipleInputQuery, 
//    DoSpatialExtentsQuery.
//
//    Kathleen Bonnell, Fri Jul  9 16:55:32 PDT 2004 
//    In order to get Lineouts working successfully with parallel engine
//    and SR mode, split Lineout into two parts -- Start (where the Lineout
//    window gets created, and lineout options get cached), and Finish
//    (where the CurvePlot and LineoutOperator are created and realized).
//    To accomplish this, I  replace 'Lineout' methods with 'StartLineout',
//    and LineQuery with StartLineQuery.  Added 'FinishLineout' and 
//    'FinishLineQuery'.  Added lineoutCache to store the lineout options
//    between calling Start and Finish.
//
//    Kathleen Bonnell, Thu Aug  5 08:34:15 PDT 2004 
//    Added ResetLineoutColor.
//
//    Kathleen Bonnell, Wed Dec 15 17:16:17 PST 2004 
//    Added bool arg to DatabaseQuery and PointQuery.
//
//    Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005 
//    Added SetLineoutsFollowTime, SetLineoutsTimeSlider. Made
//    SetDynamicLinenout private, and GetColor public. 
//
//    Kathleen Bonnell, Wed Mar 23 15:09:16 PST 2005 
//    Added ClearRefLines.
//
//    Kathleen Bonnell, Thu Jul 14 09:16:22 PDT 2005
//    Added EngineExistsForQuery. 
//
//    Kathleen Bonnell, Wed Jul 27 15:47:34 PDT 2005 
//    Added SuppressQueryOutput.
//
//    Kathleen Bonnell, Wed Aug 10 16:46:17 PDT 2005
//    Added ActivePlotsChanged. 
//
//    Kathleen Bonnell, Tue Aug 16 10:03:27 PDT 2005 
//    Changed arg from ViewerWindow to ViewerPlot for EngineExistsForQuery. 
//
//    Kathleen Bonnell, Tue Jan 17 11:30:15 PST 2006 
//    Removed SetLineoutsTimeSlider.
//
//    Hank Childs, Tue Jul 11 14:34:06 PDT 2006
//    Added double arguments to DatabaseQuery.
//
//    Kathleen Bonnell, Wed Nov  1 13:50:32 PST 2006 
//    Added CloneQuery.
//
//    Brad Whitlock, Mon Feb 12 17:44:20 PST 2007
//    Added ViewerBase.
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Added optional bool arg to StartLineQuery, AddQuery, added bool
//    to CachedLineout struct.
//
//    Cyrus Harrison, Tue Sep 18 11:01:57 PDT 2007
//    Added floatFormat 
//
//    Kathleen Bonnell, Tue Mar  1 10:24:20 PST 2011
//    Added arg curvePlotType to PointQuery and PickThroughTime methods.
//
//    Kathleen Bonnell, Fri Jun 10 13:48:59 PDT 2011
//    Added shortened DatabaseQuery, added preserveCoord to PointQuery.
//
//    Kathleen Biagas, Fri Jun 17 16:30:51 PDT 2011
//    Add generic 'Query' method.
//
//    Kathleen Biagas, Tue Jun 21 10:56:41 PDT 2011
//    Public Database, Point and LineQuery methods replaced by generic 'Query'
//    method, which calls the private methods for Database, Point and Line.
//    Arguments now stored in MapNode.
//
//    Kathleen Biagas, Fri Jul 15 11:24:17 PDT 2011
//    Add GetQueryParameters.
//
//    Brad Whitlock, Thu Sep 11 23:16:44 PDT 2014
//    Moved some client state objects out.
//
//    Alister Maguire, Tue Oct  3 11:27:21 PDT 2017
//    Added overrideTimeStep for pick ranges that include
//    a pick time curve. 
//
//    Alister Maguire, Wed May 23 09:46:54 PDT 2018
//    Added RetrieveTimeSteps to simplify and reduce repetition. 
//
//    Alister Maguire, Wed Aug  8 15:21:46 PDT 2018
//    Added SwivelFocusToPickPoint, ClearRemovedPickPoints, 
//    and GetNextPickLabel. 
//
// ****************************************************************************
    
class VIEWERCORE_API ViewerQueryManager : public ViewerBase
{
  public:
                   ~ViewerQueryManager();
    static          ViewerQueryManager *Instance();      


    void            Query(const MapNode &queryParams);
    void            GetQueryParameters(const std::string &queryName);

    void            Pick(PICK_POINT_INFO *pd, const int dom = -1,
                         const int el = -1);
    void            NoGraphicsPick(PICK_POINT_INFO *ppi);

    void            Delete(ViewerPlot *vp);
    void            Delete(ViewerWindow *vw);


    void            HandleTool(ViewerWindow *, const avtToolInterface &ti);
    bool            InitializeTool(ViewerWindow *, avtToolInterface &ti);
    void            DisableTool(ViewerWindow *, avtToolInterface &ti);

    void            UpdatePickAtts();
    void            SetPickAttsFromClient();
    void            SetPickAttsFromDefault();
    void            SetDefaultPickAttsFromClient();
    void            SetClientPickAttsFromDefault();
    void            ClearPickPoints(void);
    void            ClearRemovedPickPoints(void);
    void            ResetPickLetter(void);
    void            ResetDesignator(void);

    void            ResetLineoutColor(void);

    void            UpdateQueryOverTimeAtts();
    void            SetQueryOverTimeAttsFromClient();
    void            SetQueryOverTimeAttsFromDefault();
    void            SetDefaultQueryOverTimeAttsFromClient();
    void            SetClientQueryOverTimeAttsFromDefault();
    void            ActivePlotsChanged()
                       {  activePlotsChanged = true; } ;

    void SetDDTPickCallback(void (*cb)(PickAttributes *, void*), void *cbdata);

    static PickAttributes            *GetPickAtts();
    static GlobalLineoutAttributes   *GetGlobalLineoutAtts();
    static QueryOverTimeAttributes   *GetQueryOverTimeAtts();

    static PickAttributes            *GetPickDefaultAtts();
    static QueryOverTimeAttributes   *GetQueryOverTimeDefaultAtts();


    void            SetGlobalLineoutAttsFromClient();

    void            StartPickMode(const bool, const bool);
    void            StopPickMode();
    void            ViewDimChanged(ViewerWindow *);

    void            CreateNode(DataNode *);
    void            SetFromNode(DataNode *, const std::string &);

    void            InitializeQueryList(void);

    void            DoSpatialExtentsQuery(ViewerPlot *, bool);

    void            StartLineout(ViewerWindow *, bool);
    void            StartLineout(ViewerWindow *, Line *);
    void            FinishLineout();

    void            FinishLineQuery();
    ColorAttribute  GetColor();

    void            ClearRefLines(ViewerWindow *);

    void            SuppressQueryOutput(bool onOff)
                        { suppressQueryOutput = onOff; } ;
    
    void            SetQueryFloatFormat(const std::string format)
                        { floatFormat = format; } ;

    void            CloneQuery(ViewerQuery *, int, int);

  protected:
                    ViewerQueryManager();

  private:
    bool            EngineExistsForQuery(ViewerPlot *);
    void            AddQuery(ViewerWindow *origwin, Line *lineAtts,
                             const bool fromDefault = true,
                             const bool forceSampling = false);

    void            SimpleAddQuery(ViewerQuery_p query, ViewerPlot *, 
                                   ViewerWindow *, ViewerWindow *);

    void            UpdateDesignator(void);
    int             VerifyQueryVariables(const std::string &qName, 
                                         const std::vector<int> &varTypes);

    bool            RetrieveTimeSteps(int    &startT, 
                                      int    &endT, 
                                      int    &stride,
                                      int     nStates,
                                      MapNode timeParams);

    std::string     GetNextPickLabel();

    bool            ComputePick(PICK_POINT_INFO *pd, const int dom = -1,
                                const int el = -1);
    void            PickThroughTime(PICK_POINT_INFO *pd, 
                                const int curvePlotType = 0,
                                const int dom = -1,
                                const int el = -1);

    void            DoTimeQuery(ViewerWindow *origWin, QueryAttributes *qA);
    void            ResetLineoutCache();
    void            SetDynamicLineout(bool);
    void            SetLineoutsFollowTime(bool);

    void            HandlePickCache();

    void            PointQuery(const MapNode &queryParams);
    void            DatabaseQuery(const MapNode &queryParams);
    void            StartLineQuery(const MapNode &queryParams);

    bool            VerifySingleInputQuery(ViewerPlotList *, const int,
                            const std::string &,
                            const std::vector<std::string> &,
                            QueryAttributes &); 

    bool            VerifyMultipleInputQuery(ViewerPlotList *, const int, 
                            const std::string &,
                            const std::vector<std::string> &,
                            QueryAttributes &); 

    bool            SwivelFocusToPickPoint(ViewerWindow *);

    bool            initialPick;
    bool            preparingPick;
    bool            handlingCache;
    std::vector<PICK_POINT_INFO> pickCache;

    char                 *designator;
    char                  baseDesignator; 
    bool                  cycleDesignator; 

    LineoutListItem     **lineoutList;
    int                   nLineouts;
    int                   nLineoutsAlloc;
    int                   colorIndex;

    CachedLineout         lineoutCache;

    bool                  suppressQueryOutput; 
    bool                  activePlotsChanged; 
    bool                  overrideTimeStep;

    std::string           floatFormat;

    void (*DDTPickCB)(PickAttributes *, void*);
    void *DDTPickCBData;

    // Intermediate values kept while query operations are happening. These seem to
    // be needed because a query operation can take place in multiple stages.
    static PickAttributes          *pickAtts;
    static GlobalLineoutAttributes *globalLineoutAtts;
    static QueryOverTimeAttributes *timeQueryAtts;

    // Default atts
    static PickAttributes          *pickDefaultAtts;
    static QueryOverTimeAttributes *timeQueryDefaultAtts;

    static ViewerQueryManager *instance;

};

#endif
