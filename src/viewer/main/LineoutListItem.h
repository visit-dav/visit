// ************************************************************************* //
//                          LineoutListItem.h                                //
// ************************************************************************* //

#ifndef LINEOUT_LIST_ITEM_H
#define LINEOUT_LIST_ITEM_H
#include <viewer_exports.h>

#include <SimpleObserver.h>
#include <ViewerQuery.h>

// Forward declarations.

class ViewerPlot;
class ViewerWindow;
class PlotQueryInfo;
class avtToolInterface;


// ****************************************************************************
//  Class:  LineoutListItem                                
//
//  Purpose:
//    Maintains information about lineouts, including the plot and window that
//    originated the query, and the window where query results (curve plots)
//    are drawn.   Keeps a list of ViewerQueries (lineouts) associated with the 
//    originating plot.
//
//    Programmer: Kathleen Bonnell
//    Creation:   March 5, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  7 16:27:04 PST 2003
//    Removed unnecessary method 'DeleteResultsWindow'.
//
//    Kathleen Bonnell, Wed Jul  9 09:21:22 PDT 2003  
//    Added method ViewDimChanged.
//
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004 
//    Removed unused GetMaxQueryWidth, GetMaxQueryHeight. 
//
// ****************************************************************************

class VIEWER_API LineoutListItem : public SimpleObserver
{
  public:
    LineoutListItem(ViewerPlot *, ViewerWindow *, ViewerWindow *);
    LineoutListItem(const LineoutListItem &);
    ~LineoutListItem();

    LineoutListItem & operator=(const LineoutListItem &);

    bool Matches(ViewerPlot *, ViewerWindow *, ViewerWindow *);
    bool MatchOriginatingPlot(ViewerPlot *);
    bool MatchOriginatingWindow(ViewerWindow *);
    bool MatchResultsWindow(ViewerWindow *);

    bool IsEmpty(void);

    void AddQuery(ViewerQuery_p);
    void Update(Subject *);

    void ObserveOriginatingPlot(void);
    void StopObservingPlot(void);

    void DeleteOriginatingPlot(void);
    bool DeleteResultsPlot(ViewerPlot *vp);
    void DeleteOriginatingWindow(void);

    ViewerWindow *GetResultsWindow(void) { return resWin; };

    void HandleTool(const avtToolInterface &);
    bool InitializeTool(avtToolInterface &);
    void DisableTool(void);
    void ViewDimChanged(void);


  private:
    ViewerPlot *origPlot;
    ViewerWindow *origWin;
    ViewerWindow *resWin;
    PlotQueryInfo *origPlotQueryInfo;
    ViewerQuery_p *queries;
    int nQueries;
    int nQueriesAlloc;
};

#endif
