/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             VisWinLegends.C                               //
// ************************************************************************* //

#include <VisWinLegends.h>

#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>

#include <VisWindow.h>
#include <VisWinPathTracker.h>

#include <avtActor.h>
#include <avtLegend.h>


using std::string;
using std::vector;


const double   VisWinLegends::leftColumnPosition  = 0.05;
const double   VisWinLegends::rightColumnPosition = 0.80;
const double   VisWinLegends::dbInfoHeight        = 0.07;
const double   VisWinLegends::dbInfoWidth         = 0.21;


// ****************************************************************************
//  Method: VisWinLegends constructor
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 11 12:13:01 PDT 2002
//    Initialized mainDBInfoAdded.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002    
//    Use vtkTextActor to replace vtkTextMapper and vtkScaledTextActor. 
//
//    Eric Brugger, Mon Jul 14 16:43:13 PDT 2003
//    Changed the way database information is handled.
//
//    Cyrus Harrison, Sun Jun 17 22:00:15 PDT 2007
//    Added init of path expansion mode.
//
//    Brad Whitlock, Tue Jan 29 16:44:06 PST 2008
//    Added dbIntoTextAttributes.
//
//    Brad Whitlock, Wed Mar 26 14:32:19 PDT 2008
//    Changed dbInfoTextAttributes so it uses scale instead of height.
//
//    Brad Whitlock, Mon Mar  2 14:11:07 PST 2009
//    I added dbInfoTimeScale and dbInfoTimeOffset.
//
// ****************************************************************************

VisWinLegends::VisWinLegends(VisWindowColleagueProxy &p) : VisWinColleague(p),
    dbInfoTextAttributes()
{
    dbInfoActor  = vtkTextActor::New();
    dbInfoActor->ScaledTextOn();
    dbInfoActor->SetWidth(dbInfoWidth);
    dbInfoActor->SetHeight(dbInfoHeight);
    dbInfoActor->GetTextProperty()->SetJustificationToLeft();
    dbInfoActor->GetTextProperty()->SetLineOffset(0);
    dbInfoActor->GetTextProperty()->SetLineSpacing(1);
    dbInfoIsAdded = false;
    dbInfoTimeScale = 1.;
    dbInfoTimeOffset = 0.;

    dbInfoVisible = true;
    pathExpansionMode = 0;
    legendVisible = true;
    homogeneous = true;
}


// ****************************************************************************
//  Method: VisWinLegends destructor
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002    
//    dbInfoMapper no longer required. 
//
//    Eric Brugger, Mon Jul 14 16:43:13 PDT 2003
//    Changed the way database information is handled.
//
// ****************************************************************************

VisWinLegends::~VisWinLegends()
{
    if (dbInfoActor != NULL)
    {
        dbInfoActor->Delete();
        dbInfoActor = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinLegends::UpdatePlotList
//
//  Purpose:
//      Updates the legends based on what plots are in the vis window.
//
//  Arguments:
//      lst     A list of plots.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
VisWinLegends::UpdatePlotList(vector<avtActor_p> &lst)
{
    UpdateDBInfo(lst);
    UpdateLegendInfo(lst);
}


// ****************************************************************************
//  Method: VisWinUserInfo::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the legends info.
//
//  Arguments:
//      fr      The red (rgb) foreground color.
//      fg      The green (rgb) foreground color.
//      fb      The blue (rgb) foreground color.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use new vtkTextProperty. 
//
//    Eric Brugger, Mon Jul 14 16:43:13 PDT 2003
//    Changed the way database information is handled.
//
//    Brad Whitlock, Tue Jan 29 16:46:56 PST 2008
//    Don't use the foreground color unless the flag is set.
//
// ****************************************************************************
 
void
VisWinLegends::SetForegroundColor(double fr, double fg, double fb)
{
    if(dbInfoTextAttributes.useForegroundColor)
        dbInfoActor->GetTextProperty()->SetColor(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinLegends::UpdateLegendInfo
//
//  Purpose:
//      Positions the legends of all the plots.
//
//  Arguments:
//      lst     A list of actors in the current window.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 11 14:13:45 PST 2002
//    Made it so the legends can be taken away with a global flag.
//
//    Eric Brugger, Mon Jul 14 16:43:13 PDT 2003
//    Changed the way database information is handled.
//
//    Eric Brugger, Wed Jul 16 09:53:37 PDT 2003
//    Changed the positioning algorithm.  It now proceeds sequentially
//    through the list of plots, adding a plot's legend if it fits in the
//    remaining legend area.
//
//    Eric Brugger, Thu Jul 17 08:57:24 PDT 2003
//    Modified the call to GetLegendSize to pass the maximum space left
//    for legends.
//
//    Eric Brugger, Tue Oct  7 14:58:45 PDT 2003
//    Modified the routine to set the database information to NULL if
//    dbInfoVisible is false.
//
//    Kathleen Bonnell, Thu Aug 12 13:07:29 PDT 2004 
//    Added call to set legend's global visibility state. 
//
//    Brad Whitlock, Thu Mar 22 02:16:37 PDT 2007
//    Renamed to UpdateLegendInfo and removed all positioning code.
//
//    Cyrus Harrison, Sun Jun 17 21:46:50 PDT 2007
//    Added support for path expansion mode.
//
//    Cyrus Harrison, Tue Sep 25 09:32:50 PDT 2007
//    Added additional path expansion mode options
//
// ****************************************************************************

void
VisWinLegends::UpdateLegendInfo(vector<avtActor_p> &lst)
{
    std::vector<avtActor_p>::iterator it;
    vtkRenderer *foreground = mediator.GetForeground();

    for (it = lst.begin() ; it != lst.end() ; it++)
    {
        avtLegend_p legend = (*it)->GetLegend();
        avtBehavior_p b = (*it)->GetBehavior();
        avtDataAttributes &atts = b->GetInfo().GetAttributes();
        if (*legend != NULL)
        {
            legend->SetGlobalVisibility(legendVisible);
            if(legendVisible && legend->GetLegendOn())
            {
                if (homogeneous || !dbInfoVisible)
                {
                    legend->SetDatabaseInfo(NULL);
                }
                else
                {
                    char info[2048];
                    std::string dbname;
                    if(pathExpansionMode == 0 ) // file
                    { dbname = atts.GetFilename(); }
                    if(pathExpansionMode == 1 ) // directory
                    { 
                        dbname = VisWinPathTracker::Instance()
                                    ->GetDirectory(atts.GetFullDBName());
                    }
                    if(pathExpansionMode == 2 ) // full
                    { dbname = atts.GetFullDBName(); }
                    else if( pathExpansionMode == 3 ) // smart
                    {
                        dbname = VisWinPathTracker::Instance()
                                        ->GetSmartPath(atts.GetFullDBName());
                    }
                    else if( pathExpansionMode == 4 ) // smart parent
                    { 
                        dbname = VisWinPathTracker::Instance()
                                   ->GetSmartDirectory(atts.GetFullDBName());
                    }

                    CreateDatabaseInfo(info,dbname,atts);
                    legend->SetDatabaseInfo(info);
                }

                legend->Add(foreground);
            }
            else
            {
                legend->Remove();
            }
        }
    }
}


// ****************************************************************************
//  Method: VisWinLegends::UpdateDBInfo
//
//  Purpose:
//      Sets the database information based on the current plots.  If they all
//      have the same cycle, time, and file, it uses one indicator for them
//      all.  If not, it sets them on a per legend basis.
//
//  Arguments:
//      lst     A list of actors in the current window.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    dbInfoActor (vtkTextActor) replaces dbInfoMapper (vtkTextMapper).
//
//    Eric Brugger, Mon Jul 14 16:43:13 PDT 2003
//    Changed the way database information is handled.
//
//    Eric Brugger, Tue Oct  7 14:58:45 PDT 2003
//    Modified the routine to set the database information to NULL if
//    dbInfoVisible is false.
//
//    Kathleen Bonnell, Thu Nov 13 12:26:00 PST 2003 
//    Use a slightly larger width for the dbInfoActor if CreateDatabaseInfo
//    indicates that 'Time' was included. 
//
//    Cyrus Harrison,Sun Jun 17 21:46:50 PDT 2007
//    Added support for path expansion mode.
//
//    Cyrus Harrison, Tue Sep 25 09:32:50 PDT 2007
//    Added additional path expansion mode options and adjusted db info actor
//    width to better display long text.
//
//    Brad Whitlock, Tue Jan 29 16:44:23 PST 2008
//    Added code to scale the dbInfo based on dbIntoTextAttributes.
//
//    Brad Whitlock, Wed Mar 26 14:36:45 PDT 2008
//    Changed the height to scale.
//
// ****************************************************************************

void
VisWinLegends::UpdateDBInfo(vector<avtActor_p> &lst)
{
    std::vector<avtActor_p>::iterator it;

    //
    // Determine if the plots have the same database information.
    //
    int    cycle;
    double dtime;
    string filename;
    bool   haveSetData = false;
    homogeneous = true;
    for (it = lst.begin() ; it != lst.end() ; it++)
    {
        avtActor_p actor = *it;
        avtBehavior_p b = actor->GetBehavior();
        avtDataAttributes &atts = b->GetInfo().GetAttributes();
        if (!haveSetData)
        {
            cycle = atts.GetCycle();
            dtime = atts.GetTime();
            filename = atts.GetFullDBName();
            haveSetData = true;
        }
        else
        {
            if (cycle != atts.GetCycle() ||
                dtime != atts.GetTime() ||
                filename != atts.GetFullDBName())
            {
                homogeneous = false;
                break;
            }
        }
    }

    //
    // If the plots all have the same database information, create the
    // overall database information.
    //
    vtkRenderer *foreground = mediator.GetForeground();
    if (!lst.empty() && homogeneous && dbInfoVisible)
    {
        avtBehavior_p b = lst[0]->GetBehavior();
        avtDataAttributes &atts = b->GetInfo().GetAttributes();

        char info[2048];
        std::string dbname;

        if(pathExpansionMode == 0 ) // file
        { dbname = atts.GetFilename(); }
        if(pathExpansionMode == 1 ) // directory
        { 
            dbname = VisWinPathTracker::Instance()
                        ->GetDirectory(atts.GetFullDBName());
        }
        if(pathExpansionMode == 2 ) // full
        { dbname = atts.GetFullDBName(); }
        else if( pathExpansionMode == 3 ) // smart
        {
            dbname = VisWinPathTracker::Instance()
                       ->GetSmartPath(atts.GetFullDBName());
        }
        else if( pathExpansionMode == 4 ) // smart parent
        { 
            dbname = VisWinPathTracker::Instance()
                        ->GetSmartDirectory(atts.GetFullDBName());
        }
        bool hasTime = CreateDatabaseInfo(info,dbname,atts);

        dbInfoActor->SetInput(info);

        //
        //  If we are adding time, we need a larger width
        //

        double scale = dbInfoTextAttributes.scale;
        float info_width = dbInfoWidth; 
        if (hasTime)
            info_width += 0.03;
        
        // scale text width if necessary when path expansion is enabled
        if(pathExpansionMode > 0)
        {
            if(info_width < dbname.size() * .01)
                info_width = dbname.size() * .01;
            if(info_width  > .8)
                info_width = .8;
        }
        
        dbInfoActor->SetHeight(dbInfoHeight+0.04);        
        dbInfoActor->SetWidth(info_width * scale);
        
        double x = leftColumnPosition;
        double y = 0.98 - dbInfoHeight;
        vtkCoordinate *c = dbInfoActor->GetPositionCoordinate();
        c->SetCoordinateSystemToNormalizedViewport();
        c->SetValue(x, y);
        if (!dbInfoIsAdded)
        {
            foreground->AddActor2D(dbInfoActor);
            dbInfoIsAdded = true;
        }
    }
    else
    {
        if (dbInfoIsAdded)
        {
            foreground->RemoveActor2D(dbInfoActor);
            dbInfoIsAdded = false;
        }
    }
}


// ****************************************************************************
// Method: VisWinLegends::SetVisibility
//
// Purpose: 
//   Turns the main database info on and off.
//
// Arguments:
//   db : The new db visibility.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 11 12:18:49 PDT 2002
//
// Modifications:
//   Cyrus Harrison, Mon Jun 18 09:34:37 PDT 2007
//   Added database path expansion mode option
//
// ****************************************************************************

void
VisWinLegends::SetVisibility(bool db, int path_exp_mode, bool legend)
{
    dbInfoVisible = db;
    pathExpansionMode = path_exp_mode;
    legendVisible = legend;
}

// ****************************************************************************
// Method: VisWinLegends::SetTimeScaleAndOffset
//
// Purpose: 
//   Sets the scale and offset that will be applied to the time.
//
// Arguments:
//   scale  : Multiplier for the time.
//   offset : Offset that will be added to the time.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar  2 14:13:57 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
VisWinLegends::SetTimeScaleAndOffset(double scale, double offset)
{
    dbInfoTimeScale = scale;
    dbInfoTimeOffset = offset;
}

// ****************************************************************************
//  Method: VisWinLegends::CreateDatabaseInfo
//
//  Purpose:
//      Create the database information string.
//
//  Returns:
//      True if 'Time' was included, false otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   July 14, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov 13 12:26:00 PST 2003
//    Added bool return type to indicate whether or not 'Time' was included.
//
//    Cyrus Harrison, Sun Jun 17 21:42:53 PDT 2007
//    Added explicit pass of the database name to easily support path
//    expansion modes.
//
//    Brad Whitlock, Mon Mar  2 14:12:41 PST 2009
//    I added support for time scale and offset.
//
// ****************************************************************************

bool
VisWinLegends::CreateDatabaseInfo(char *info,
                                  const std::string &dbname,
                                  avtDataAttributes &atts)
{
    bool hasTime = false;
    sprintf(info, "DB: %s\n", dbname.c_str());
    if (atts.CycleIsAccurate())
    {
        sprintf(info+strlen(info), "Cycle: %-8d ", atts.GetCycle());

    }
    if (atts.TimeIsAccurate())
    {
        double t = atts.GetTime() * dbInfoTimeScale + dbInfoTimeOffset;
        sprintf(info+strlen(info), "Time:%-10g", t);
        hasTime = true;
    }
    return hasTime;
}

// ****************************************************************************
// Method: VisWinLegends::SetDatabaseInfoTextAttributes
//
// Purpose: 
//   Sets the database info's text properties.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:57:27 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisWinLegends::SetDatabaseInfoTextAttributes(const VisWinTextAttributes &textAtts)
{
    dbInfoTextAttributes = textAtts;

    // Update the actor's text color.
    if(dbInfoTextAttributes.useForegroundColor)
    {
        double color[3];
        mediator.GetForegroundColor(color);
        dbInfoActor->GetTextProperty()->SetColor(color[0],color[1],color[2]);
        dbInfoActor->GetTextProperty()->SetOpacity(1.);
    }
    else
    {
        dbInfoActor->GetTextProperty()->SetColor(
            dbInfoTextAttributes.color[0],
            dbInfoTextAttributes.color[1],
            dbInfoTextAttributes.color[2]);
        dbInfoActor->GetTextProperty()->SetOpacity(dbInfoTextAttributes.color[3]);
    }

    dbInfoActor->GetTextProperty()->SetFontFamily((int)dbInfoTextAttributes.font);
    dbInfoActor->GetTextProperty()->SetBold(dbInfoTextAttributes.bold?1:0);
    dbInfoActor->GetTextProperty()->SetItalic(dbInfoTextAttributes.italic?1:0);

    // We're not updating the size here. The plot list has to be updated for that
    // to happen.
}
