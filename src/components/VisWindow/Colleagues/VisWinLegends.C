// ************************************************************************* //
//                             VisWinLegends.C                               //
// ************************************************************************* //

#include <VisWinLegends.h>

#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>

#include <VisWindow.h>

#include <avtActor.h>
#include <avtLegend.h>


using std::string;
using std::vector;


const float   VisWinLegends::leftColumnPosition  = 0.05;
const float   VisWinLegends::rightColumnPosition = 0.80;
const float   VisWinLegends::dbInfoHeight        = 0.07;
const float   VisWinLegends::dbInfoWidth         = 0.21;


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
// ****************************************************************************

VisWinLegends::VisWinLegends(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    dbInfoActor  = vtkTextActor::New();
    dbInfoActor->ScaledTextOn();
    dbInfoActor->SetWidth(dbInfoWidth);
    dbInfoActor->SetHeight(dbInfoHeight);
    dbInfoActor->GetTextProperty()->SetJustificationToLeft();
    dbInfoActor->GetTextProperty()->SetLineOffset(0);
    dbInfoActor->GetTextProperty()->SetLineSpacing(1);
    dbInfoIsAdded = false;

    mainDBInfoVisible = true;
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
    PositionLegends(lst);
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
// ****************************************************************************
 
void
VisWinLegends::SetForegroundColor(float fr, float fg, float fb)
{
    dbInfoActor->GetTextProperty()->SetColor(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinLegends::PositionLegends
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
//    mainDBInfoVisible is false.
//
//    Kathleen Bonnell, Thu Aug 12 13:07:29 PDT 2004 
//    Added call to set legend's global visibility state. 
//
// ****************************************************************************

void
VisWinLegends::PositionLegends(vector<avtActor_p> &lst)
{
    std::vector<avtActor_p>::iterator it;
    vtkRenderer *foreground = mediator.GetForeground();
    float yTop = 0.90;
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
                if (homogeneous || !mainDBInfoVisible)
                {
                    legend->SetDatabaseInfo(NULL);
                }
                else
                {
                    char info[1024];
                    CreateDatabaseInfo(info, atts);
                    legend->SetDatabaseInfo(info);
                }

                float width, height;
                legend->GetLegendSize(yTop, width, height);

                if (yTop - height >= 0.)
                {
                    yTop -= height;

                    legend->Add(foreground);
                    legend->SetLegendPosition(0.05, yTop);
                    legend->Update();

                    yTop -= 0.02;
                }
                else
                {
                    legend->Remove();
                }
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
//    mainDBInfoVisible is false.
//
//    Kathleen Bonnell, Thu Nov 13 12:26:00 PST 2003 
//    Use a slightly larger width for the dbInfoActor if CreateDatabaseInfo
//    indicates that 'Time' was included. 
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
            filename = atts.GetFilename();
            haveSetData = true;
        }
        else
        {
            if (cycle != atts.GetCycle() ||
                dtime != atts.GetTime() ||
                filename != atts.GetFilename())
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
    if (!lst.empty() && homogeneous && mainDBInfoVisible)
    {
        avtBehavior_p b = lst[0]->GetBehavior();
        avtDataAttributes &atts = b->GetInfo().GetAttributes();

        char info[1024];
        bool hasTime = CreateDatabaseInfo(info, atts);
        dbInfoActor->SetInput(info);

        //
        //  If we are adding time, we need a larger width
        //
        if (hasTime)
        {
            dbInfoActor->SetWidth(dbInfoWidth+0.03);
        }
        else 
        {
            dbInfoActor->SetWidth(dbInfoWidth);
        }
        dbInfoActor->SetHeight(dbInfoHeight+0.04);
        float x = leftColumnPosition;
        float y = 0.98 - dbInfoHeight;
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
// ****************************************************************************

void
VisWinLegends::SetVisibility(bool db, bool legend)
{
    mainDBInfoVisible = db;
    legendVisible = legend;
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
// ****************************************************************************

bool
VisWinLegends::CreateDatabaseInfo(char *info, avtDataAttributes &atts)
{
    bool hasTime = false;
    sprintf(info, "DB: %s\n", atts.GetFilename().c_str());
    if (atts.CycleIsAccurate())
    {
        sprintf(info+strlen(info), "Cycle: %-8d ", atts.GetCycle());

    }
    if (atts.TimeIsAccurate())
    {
        sprintf(info+strlen(info), "Time:%-10g", atts.GetTime());
        hasTime = true;
    }
    return hasTime;
}
