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
// ****************************************************************************
    
class VIEWER_API ViewerQueryManager 
{
  public:
                    ~ViewerQueryManager();
     static         ViewerQueryManager *Instance();      
     void           SetOperatorFactory(ViewerOperatorFactory *);

     void           AddQuery(ViewerWindow *origwin, Line *lineAtts);

     void           DatabaseQuery(ViewerWindow *owin, const std::string &qName,
                            const std::vector<std::string> &vars);

     void           LineQuery(const char *qName, const double *pt1, 
                      const double *pt2, const std::vector<std::string> &vars);

     void           PointQuery(const std::string &qName, const double *pt, 
                            const std::vector<std::string> &vars);

     void           Pick(PICK_POINT_INFO *pd);

     void           Delete(ViewerPlot *vp);
     void           Delete(ViewerWindow *vw);


     void           HandleTool(ViewerWindow *, const avtToolInterface &ti);
     bool           InitializeTool(ViewerWindow *, avtToolInterface &ti);
     void           DisableTool(ViewerWindow *, avtToolInterface &ti);

     void           UpdateScaleFactor(ViewerWindow *);

     QueryList     *GetQueryTypes() { return queryTypes; };

     void           UpdatePickAtts();
     void           ClearPickPoints(void);


     static QueryAttributes  *GetQueryClientAtts();
     static PickAttributes   *GetPickAtts();
     static GlobalLineoutAttributes   *GetGlobalLineoutAtts();
     static GlobalLineoutAttributes   *GetGlobalLineoutClientAtts();


     void           Lineout(ViewerWindow *win, const double pt1[3], 
                            const double pt2[3], const std::string &);
     void           Lineout(ViewerWindow *);

    void            SetGlobalLineoutAttsFromClient();
    void            SetDynamicLineout(bool);

    void            StartPickMode();
    void            StopPickMode();

  protected:
                    ViewerQueryManager();
    ColorAttribute  GetColor();

  private:
    void            SimpleAddQuery(ViewerQuery_p query, ViewerPlot *, 
                                   ViewerWindow *, ViewerWindow *);

    void            UpdateDesignator(void);
    avtVarType      DetermineVarType(const char *hName, const char *dbName, 
                                     const char *varName);

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
    static GlobalLineoutAttributes          *globalLineoutAtts;
    static GlobalLineoutAttributes          *globalLineoutClientAtts;

    static ViewerQueryManager *instance;
};

#endif
