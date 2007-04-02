/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//    Kathleen Bonnell, Thu Feb  3 16:27:10 PST 2005 
//    Added SetLineoutsFollowTime, SetTimeSlider. 
//
//    Kathleen Bonnell, Tue Jan 17 11:30:15 PST 2006 
//    Removed SetTimeSlider. 
//
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
    void SetLineoutsFollowTime(bool);

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
