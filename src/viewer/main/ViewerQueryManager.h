// ************************************************************************* //
//                          ViewerQueryManager.h                             //
// ************************************************************************* //

#ifndef VIEWER_QUERY_MANAGER_H
#define VIEWER_QUERY_MANAGER_H
#include <viewer_exports.h>

#include <PickPointInfo.h>
#include <ColorAttribute.h>
#include <ViewerQuery.h>
#include <avtTypes.h>
#include <vector>

// Forward declarations.
class DataNode;
class GlobalLineoutAttributes;
class Line;
class LineoutListItem;
class PickAttributes;
class QueryAttributes;
class QueryList;
class ViewerOperatorFactory;
class ViewerWindow;
class ViewerPlot;
class avtToolInterface;

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
// ****************************************************************************
    
class VIEWER_API ViewerQueryManager 
{
  public:
                   ~ViewerQueryManager();
    static          ViewerQueryManager *Instance();      
    void            SetOperatorFactory(ViewerOperatorFactory *);

    void            DatabaseQuery(ViewerWindow *owin, const std::string &qName,
                            const std::vector<std::string> &vars,
                            const int arg1, const int arg2);

    void            LineQuery(const char *qName, const double *pt1, 
                      const double *pt2, const std::vector<std::string> &vars,
                      const int samples);

    void            PointQuery(const std::string &qName, const double *pt, 
                            const std::vector<std::string> &vars);

    void            Pick(PICK_POINT_INFO *pd);

    void            Delete(ViewerPlot *vp);
    void            Delete(ViewerWindow *vw);


    void            HandleTool(ViewerWindow *, const avtToolInterface &ti);
    bool            InitializeTool(ViewerWindow *, avtToolInterface &ti);
    void            DisableTool(ViewerWindow *, avtToolInterface &ti);

    QueryList      *GetQueryTypes() { return queryTypes; };

    void            UpdatePickAtts();
    void            SetPickAttsFromClient();
    void            ClearPickPoints(void);


    static QueryAttributes  *GetQueryClientAtts();
    static PickAttributes   *GetPickAtts();
    static PickAttributes   *GetPickClientAtts();
    static GlobalLineoutAttributes   *GetGlobalLineoutAtts();
    static GlobalLineoutAttributes   *GetGlobalLineoutClientAtts();


    void            Lineout(ViewerWindow *win, const double pt1[3], 
                            const double pt2[3], const std::string &,
                            const int samples);
    void            Lineout(ViewerWindow *, const bool fromLineout = true);
    void            Lineout(ViewerWindow *, Line *);

    void            SetGlobalLineoutAttsFromClient();
    void            SetDynamicLineout(bool);

    void            StartPickMode(const bool, const bool);
    void            StopPickMode();
    void            ViewDimChanged(ViewerWindow *);

    void            CreateNode(DataNode *);
    void            SetFromNode(DataNode *);

    void            InitializeQueryList(void);

  protected:
                    ViewerQueryManager();
    ColorAttribute  GetColor();

  private:
    void            AddQuery(ViewerWindow *origwin, Line *lineAtts,
                             const bool fromDefault = true);

    void            SimpleAddQuery(ViewerQuery_p query, ViewerPlot *, 
                                   ViewerWindow *, ViewerWindow *);

    void            UpdateDesignator(void);
    avtVarType      DetermineVarType(const char *hName, const char *dbName, 
                                     const char *varName);
    int             VerifyQueryVariables(const std::string &qName, 
                                         const std::vector<int> &varTypes);


    void            HandlePickCache();
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
    QueryList             *queryTypes;
    ViewerOperatorFactory *operatorFactory;

    static QueryAttributes    *queryClientAtts;
    static PickAttributes     *pickAtts;
    static PickAttributes     *pickClientAtts;
    static GlobalLineoutAttributes          *globalLineoutAtts;
    static GlobalLineoutAttributes          *globalLineoutClientAtts;

    static ViewerQueryManager *instance;
};

#endif
