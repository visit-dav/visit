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
//                          LineoutListItem.C                                //
// ************************************************************************* //

#include <float.h>
#include <LineoutListItem.h>

#include <avtToolInterface.h>
#include <GlobalLineoutAttributes.h>
#include <Line.h>
#include <PlaneAttributes.h>
#include <PlotQueryInfo.h>
#include <ViewerOperator.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>


// ****************************************************************************
//  Method: LineoutListItem Constructor
//
//  Arguments:
//    op        The plot that originated the lineout.
//    ow        The window that originated the lineout.
//    rw        The window where lineout results will be displayed
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

LineoutListItem::LineoutListItem(ViewerPlot *op, ViewerWindow *ow, ViewerWindow *rw)
{
    origPlot = op;
    origWin = ow;
    resWin = rw;
    origPlotQueryInfo = 0;

    queries = 0;
    nQueries = 0;
    nQueriesAlloc = 0;
}


// ****************************************************************************
//  Method: LineoutListItem Copy Constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

LineoutListItem::LineoutListItem(const LineoutListItem &rhs)
{
    int i;
    origPlot = rhs.origPlot;
    origWin = rhs.origWin;
    resWin = rhs.resWin;
    origPlotQueryInfo = 0;
    if (rhs.origPlotQueryInfo)
        ObserveOriginatingPlot();

    nQueriesAlloc = rhs.nQueriesAlloc;
    nQueries = rhs.nQueries;
    queries = new ViewerQuery_p [nQueriesAlloc];
    for (i = 0; i < nQueries; ++i) 
    {
        queries[i] = rhs.queries[i];
    }
}


// ****************************************************************************
//  Method: LineoutListItem destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

LineoutListItem::~LineoutListItem()
{
    StopObservingPlot();
    //
    // Delete the list and any queries in the queries list.
    //
    if (nQueriesAlloc > 0)
    {
        for (int i = 0; i < nQueries; i++)
        {
            queries[i]->DeleteVisualCue();
            queries[i] = (ViewerQuery *)0;
        }

        delete [] queries;
    }
}


// ****************************************************************************
//  Method: LineoutListItem assignment operator
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

LineoutListItem & 
LineoutListItem::operator=(const LineoutListItem &rhs)
{
    int i;

    StopObservingPlot(); 
    if (nQueries > 0)
        delete [] queries;

    origPlot = rhs.origPlot;
    origWin = rhs.origWin;
    resWin = rhs.resWin;
    if (rhs.origPlotQueryInfo)
        ObserveOriginatingPlot();
    nQueries = rhs.nQueries;
    nQueriesAlloc = rhs.nQueriesAlloc;
    queries = new ViewerQuery_p [nQueriesAlloc];
    for (i = 0; i < nQueries; ++i) 
    {
        queries[i] = rhs.queries[i];
        queries[i]->SendVisualCue(); 
    }
    return *this;
}


// ****************************************************************************
//  Method: LineoutListItem::AddQuery 
//
//  Purpose:
//    Add a query to the list.
//
//  Arguments:
//    query     The query to be added.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  7 16:27:04 PST 2003
//    Removed memcpy, not needed since we are dealing with refptrs.
//
// ****************************************************************************

void
LineoutListItem::AddQuery(ViewerQuery_p query)
{
    if (nQueries >= nQueriesAlloc)
    {
        //
        // Expand the list of queries if necessary.
        //
        ViewerQuery_p *queriesNew=0;

        nQueriesAlloc += 10;
        queriesNew = new ViewerQuery_p [nQueriesAlloc];
        if (nQueries > 0)
        {
            for (int i = 0; i < nQueries; i++)
            {
                queriesNew[i] = queries[i];
            }

            delete [] queries;
        }
        queries = queriesNew;
    }

    //
    // Add the query to the list.
    //
    queries[nQueries] = query;

    nQueries++;
}


// ****************************************************************************
//  Method: LineoutListItem::IsEmpty 
//
//  Purpose:
//    Determines if this list item is empty. 
//
//  Returns:
//    True if the origPlot is not set and the querie list is empty, False
//    otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::IsEmpty()
{
    return (origPlot == 0 && nQueries == 0); 
}


// ****************************************************************************
//  Method: LineoutListItem::Matches 
//
//  Purpose:
//    Determines if this list item's members match the argument list. 
//
//  Arguments:
//    op        to compare with origPlot.
//    ow        to compare with origWin.
//    rw        to compare with resWin.
//
//  Returns:
//    True if the passed origiating plot, originating window and results
//    window match the corresponding members, false otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::Matches(ViewerPlot *op, ViewerWindow *ow, ViewerWindow *rw)
{
    return ((origPlot != 0 && origPlot == op) && 
            (origWin  != 0 && origWin  == ow) && 
            (resWin   != 0 && resWin   == rw)); 
}


// ****************************************************************************
//  Method: LineoutListItem::MatchOriginatingPlot
//
//  Purpose:
//    Determines if this list item's origPlot matches the argument. 
//
//  Arguments:
//    op        to compare with origPlot.
//  Returns:
//    True if the passed origiating plot, matches the origPlot member.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::MatchOriginatingPlot(ViewerPlot *op)
{
    return (origPlot != 0 && origPlot == op); 
}


// ****************************************************************************
//  Method: LineoutListItem::MatchOriginatingWindow
//
//  Purpose:
//    Determines if this list item's origWin matches the argument. 
//
//  Argument:
//    ow        The window to compare with.
//
//  Returns:
//    True if the passed origiating window, matches the origWin member.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::MatchOriginatingWindow(ViewerWindow *ow)
{
    return (origWin != 0 && origWin == ow); 
}


// ****************************************************************************
//  Method: LineoutListItem::MatchResultsWindow
//
//  Purpose:
//    Determines if this list item's resWin matches the argument. 
//
//  Argument:
//    rw        The window to compare with.
//
//  Returns:
//    True if the passed results window, matches the resWin member.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::MatchResultsWindow(ViewerWindow *rw)
{
    return (resWin != 0 && resWin == rw); 
}


// ****************************************************************************
//  Method: LineoutListItem::ObserveOriginatingPlot
//
//  Purpose:
//    Attach to the originating plot's query atts.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

void
LineoutListItem::ObserveOriginatingPlot()
{
    if (origPlotQueryInfo == 0 && origPlot != 0)
    {
        origPlotQueryInfo  = origPlot->GetPlotQueryInfo();
        origPlotQueryInfo->Attach(this);
    }
}


// ****************************************************************************
//  Method: LineoutListItem::ObserveOriginatingPlot
//
//  Purpose:
//    Detach from the originating plot's query atts.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

void
LineoutListItem::StopObservingPlot()
{
    if (origPlotQueryInfo != 0)
    {
        origPlotQueryInfo->Detach(this);
        origPlotQueryInfo = 0;
    }
}


// ****************************************************************************
//  Method: LineoutListItem::DeleteOriginatingPlot
//
//  Purpose:
//    The originating plot has been deleted, remove references to it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

void
LineoutListItem::DeleteOriginatingPlot()
{
    StopObservingPlot();
    for (int i = 0; i < nQueries; i++)
        queries[i]->DeleteOriginatingPlot();
    origPlot = 0;
    origWin = 0;
}


// ****************************************************************************
//  Method: LineoutListItem::DeleteResultsPlot
//
//  Purpose:
//    A results plot has been deleted, delete associated queries. 
//
//  Arguments:
//    vp        The plot that has been deleted.
//
//  Returns:
//    True if an associated query was deleted, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::DeleteResultsPlot(ViewerPlot *vp)
{
    bool found = false;
    int nQueriesNew = 0;
    for (int i = 0;  i < nQueries; i++)
     {
        if (queries[i]->MatchResultsPlot(vp))
        {
            queries[i]->DeleteVisualCue();
            queries[i] = (ViewerQuery *)0;
            found = true;
        }
        else
        {
            queries[nQueriesNew] = queries[i];
            nQueriesNew++;
        }
    }
    nQueries= nQueriesNew;
    return found;
}


// ****************************************************************************
//  Method: LineoutListItem::DeleteOriginatingWindow
//
//  Purpose:
//    The originating window has been deleted, remove references to it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

void
LineoutListItem::DeleteOriginatingWindow()
{
    StopObservingPlot();
    for (int i = 0; i < nQueries; i++)
        queries[i]->DeleteOriginatingWindow();
    origPlot = 0;
    origWin = 0;
}


// ****************************************************************************
//  Method: LineoutListItem::HandleTool
//
//  Purpose:
//    Allow the lineout queries to handle a tool.   
//
//   Arguments:
//     ti       The tool interface to be handled.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//    Brad Whitlock, Tue Jan 27 00:52:37 PDT 2004
//    I made it use the plot list instead of an animation.
//
//    Kathleen Bonnell, Mon Aug 23 09:31:07 PDT 2004 
//    Don't set the active window.
//
// ****************************************************************************

void
LineoutListItem::HandleTool(const avtToolInterface &ti)
{
    int i;
    bool success = false;
    if (ti.GetAttributes()->TypeName() == "Line")
    {
        for (i = 0; i < nQueries; i++)
        {
            success |= queries[i]->HandleTool(ti); 
        }
    }
    else if (origPlotQueryInfo)  // in Dynamic mode, other tools can be handled
    {
        if (ti.GetAttributes()->TypeName() == "PlaneAttributes")
        {
           PlaneAttributes *planeAtts = (PlaneAttributes*)ti.GetAttributes()->
                                        CreateCompatible("PlaneAttributes"); 
           if (planeAtts)
           {
               for (i =0; i < nQueries; i++)
               {
                   success |= queries[i]->UpdateLineFromSlice(planeAtts);
               }
               delete planeAtts;
           }
        } 
        else // some other tool, have the plot list handle it in the usual way.
        {
            ViewerPlotList *resPL = resWin->GetPlotList();
            resPL->HandleTool(ti);
        }
    }
    if (success)
    {
        //
        //  We want to update the window where lineouts are drawn,
        //
        ViewerPlotList *resPL = resWin->GetPlotList();
        resPL->UpdatePlotAtts(false);
        resPL->UpdateFrame();
    }
}


// ****************************************************************************
//  Method: LineoutListItem::InitializeTool
//
//  Purpose:
//    Allow the lineout queries to initialize a tool.   
//
//   Arguments:
//     ti       The tool interface to be initialized.
//
//   Returns:
//     True if the tool was updated, false otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

bool
LineoutListItem::InitializeTool(avtToolInterface &ti)
{
    bool success;
    for (int i = 0; i < nQueries; i++)
    {
        if (queries[i]->CanHandleTool()) 
        {
            success = queries[i]->InitializeTool(ti);
            break;
        }
    }
    return success;
}


// ****************************************************************************
//  Method: LineoutListItem::DisableTool
//
//  Purpose:
//    Allow the lineout queries to stop handling a tool.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//
// ****************************************************************************

void
LineoutListItem::DisableTool()
{
    for (int i = 0; i < nQueries; i++)
    {
        queries[i]->DisableTool();
    }
}


// ****************************************************************************
//  Method: LineoutListItem::Update
//
//  Purpose:
//    This method is called when certains apects of the originating Plot
//    are modified.  Allows lineout queries to be udpated as necessary.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Apr 10 10:53:43 PDT 2003
//    I added new fall through cases for MoveOperator and RemoveOperator.
//
//    Eric Brugger, Wed Aug 20 10:51:31 PDT 2003
//    I removed calls to UpdateScaleFactor since it no longer exists.
//
//    Brad Whitlock, Mon Nov 3 10:17:12 PDT 2003
//    Changed the interface to ViewerPlotList::ReplaceDatabase.
//
//    Brad Whitlock, Fri Mar 26 10:53:24 PDT 2004
//    Made it use more strings.
//
//    Brad Whitlock, Mon May 3 13:49:17 PST 2004
//    I made it use an engine key in ReplaceDatabase.
//
//    Kathleen Bonnell, Thu Feb  3 16:27:10 PST 2005 
//    Added case for CacheIndex change. 
//
//    Kathleen Bonnell, Wed Jun 21 17:52:26 PDT 2006
//    Modified the way CacheIndex change is handled. 
//
// ****************************************************************************

void
LineoutListItem::Update(Subject *TheChangedSubject)
{
     if (origPlotQueryInfo == TheChangedSubject)
     {
         ViewerPlotList *vpl = resWin->GetPlotList();
         int i, nOps;
         switch(origPlotQueryInfo->GetChangeType())
         {
             case PlotQueryInfo::Database:
                 vpl->ReplaceDatabase(origPlot->GetEngineKey(),
                     origPlot->GetDatabaseName(), 0, false, false);
                 break;
             case PlotQueryInfo::VarName:
                 vpl->SetPlotVar(origPlot->GetVariableName());
                 break;
             case PlotQueryInfo::OpAtts:
                 nOps = origPlot->GetNOperators();
                 for (i = 0; i < nOps; i++)
                 {
                     vpl->SetPlotOperatorAtts(origPlot->GetOperator(i)->GetType());
                 }
                 break;
             case PlotQueryInfo::CacheIndex:
                 if (ViewerQueryManager::Instance()->GetGlobalLineoutAtts()->
                     GetCurveOption() == GlobalLineoutAttributes::CreateCurve)
                 {
                     int newf = origPlotQueryInfo->GetNewFrameIndex();
                     int oldf = origPlotQueryInfo->GetOldFrameIndex();
                     vector<ViewerQuery_p> addme;
                     for (i = 0; i < nQueries; i++)
                     {
                         if (queries[i]->MatchTimeState(oldf))
                         {
                             ViewerQuery_p nq = new ViewerQuery(*queries[i], newf);
                             addme.push_back(nq);
                         }
                     }
                     for (i = 0; i < addme.size(); i++)
                     {
                         AddQuery(addme[i]);
                     }
                 }
                 else
                 {
                     int newf = origPlotQueryInfo->GetNewFrameIndex();
                     for (i = 0; i < nQueries; i++)
                     {
                         queries[i]->UpdateResultsTime(newf);
                     }
                     resWin->SetMergeViewLimits(true);
                     vpl->UpdateFrame(false);
                 }
                 break;
             case PlotQueryInfo::AddOp:          // fall through
             case PlotQueryInfo::MoveOperator:   // fall through
             case PlotQueryInfo::RemoveOperator: // fall through
             case PlotQueryInfo::RemoveAll:      // fall through
             case PlotQueryInfo::RemoveLast:
                 for (i = 0; i < nQueries; i++)
                 {
                     queries[i]->ReCreateLineout(); 
                 }
                 break;
             default:
                 // do nothing
                 break;

        }  // switch
    } // if
}


// ****************************************************************************
//  Method: LineoutListItem::ViewDimChanged
//
//  Purpose:
//    This method is called when the view dimension of the originating window 
//    has changed, and lineouts are nto dynamic (e.g. can be updated by changes
//    to the originating plot). Remove all visual cues from the window.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 9, 2003 
//
// ****************************************************************************

void
LineoutListItem::ViewDimChanged()
{
    if (nQueriesAlloc > 0)
    {
        for (int i = 0; i < nQueries; i++)
        {
            queries[i]->DeleteVisualCue();
        }
    }
}


// ****************************************************************************
//  Method: LineoutListItem::SetLineoutsFollowTime
//
//  Purpose:
//    Tells the queries whether or not their results plot should follow time.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 3, 2005 
//
// ****************************************************************************

void
LineoutListItem::SetLineoutsFollowTime(bool newMode)
{
    for (int i = 0; i < nQueries; i++)
        queries[i]->SetFollowsTime(newMode);
}

