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
//    Brad Whitlock, Thu Apr 11 12:13:01 PDT 2002
//    Initialized mainDBInfoAdded.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002    
//    Use vtkTextActor to replace vtkTextMapper and vtkScaledTextActor. 
//
// ****************************************************************************

VisWinLegends::VisWinLegends(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    for (int i = 0 ; i < MAX_LEGENDS ; i++)
    {
        dbInfoActor[i]  = vtkTextActor::New();
        dbInfoActor[i]->ScaledTextOn();
        dbInfoActor[i]->SetWidth(dbInfoWidth);
        dbInfoActor[i]->SetHeight(dbInfoHeight);
        dbInfoActor[i]->GetTextProperty()->SetJustificationToLeft();
        dbInfoActor[i]->GetTextProperty()->SetLineOffset(0);
        dbInfoActor[i]->GetTextProperty()->SetLineSpacing(1);
        dbInfoIsAdded[i] = false;
    }
    mainDBInfoVisible = true;
    legendVisible = true;
    homogeneous = true;
    numLegends = 0;
}


// ****************************************************************************
//  Method: VisWinLegends destructor
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002    
//    dbInfoMapper no longer required. 
//
// ****************************************************************************

VisWinLegends::~VisWinLegends()
{
    for (int i = 0 ; i < MAX_LEGENDS ; i++)
    {
        if (dbInfoActor[i] != NULL)
        {
            dbInfoActor[i]->Delete();
            dbInfoActor[i] = NULL;
        }
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
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use new vtkTextProperty. 
//
// ****************************************************************************
 
void
VisWinLegends::SetForegroundColor(float fr, float fg, float fb)
{
    for (int i = 0 ; i < MAX_LEGENDS ; i++)
    {
        dbInfoActor[i]->GetTextProperty()->SetColor(fr, fg, fb);
    }
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
// ****************************************************************************

void
VisWinLegends::PositionLegends(vector<avtActor_p> &lst)
{
    int positionCount = 0;
    std::vector<avtActor_p>::iterator it;
    vtkRenderer *foreground = mediator.GetForeground();
    for (it = lst.begin() ; it != lst.end() ; it++)
    {
        avtLegend_p legend = (*it)->GetLegend();
        if (*legend != NULL)
        {
            if(legendVisible)
            {
                if (positionCount < MAX_LEGENDS && legendVisible)
                {
                    float x = (positionCount<MAX_LEGENDS/2 ? leftColumnPosition
                                                        : rightColumnPosition);
                    float y = GetPosition(positionCount);
                    legend->Add(foreground);
                    legend->SetLegendPosition(x, y);
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

            positionCount++;
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
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    dbInfoActor (vtkTextActor) replaces dbInfoMapper (vtkTextMapper).
//
// ****************************************************************************

void
VisWinLegends::UpdateDBInfo(vector<avtActor_p> &lst)
{
    std::vector<avtActor_p>::iterator it;

    //
    // Try to gauge how many DB-infos we will need and where they will fall
    // with respect to the legend placement.
    //
    int    cycle;
    double dtime;
    string filename;
    bool   haveSetData = false;
    numLegends  = 0;
    homogeneous = true;
    for (it = lst.begin() ; it != lst.end() ; it++)
    {
        avtActor_p actor = *it;
        avtLegend_p leg = actor->GetLegend();
        if (*leg == NULL)
        {
            continue;
        }
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
                cycle = atts.GetCycle();
                dtime = atts.GetTime();
                filename = atts.GetFilename();
                homogeneous = false;
            }
        }

        char info[1024];
        sprintf(info, "DB: %s\n", filename.c_str());
        if (atts.CycleIsAccurate())
        {
            sprintf(info+strlen(info), "Cycle: %-8d ", cycle);
        }
        if (atts.TimeIsAccurate())
        {
            sprintf(info+strlen(info), "Time:%-10g", dtime);
        }
        dbInfoActor[numLegends]->SetInput(info);
        numLegends++;
        if (numLegends >= MAX_LEGENDS)
        {
            break;
        }
    }

    //
    // Now add them back in, in the appropriate positions.
    //
    AddDBInfos();
}


// ****************************************************************************
//  Method: VisWinLegends::AddDBInfos
//
//  Purpose:
//      Adds in the appropriate DB infos.  This method is called internally
//      (once the state has been updated) and externally (when the user turns
//      the info on or off).
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Mar 19 09:10:53 PST 2002
//    Removed bold type for database information text.
//
//    Brad Whitlock, Thu Apr 11 12:13:56 PDT 2002
//    Added a little code that can prevent the main database info from
//    being displayed.
//
//    Hank Childs, Tue May 21 08:34:41 PDT 2002
//    Allow annotations to be configured out.
//
// ****************************************************************************

void
VisWinLegends::AddDBInfos(void)
{
    //
    // Add the DBinfos to the screen.
    //
    vtkRenderer *foreground = mediator.GetForeground();
    if (homogeneous)
    {
        dbInfoActor[0]->SetHeight(dbInfoHeight+0.04);
        float x = leftColumnPosition;
        float y = 0.98 - dbInfoHeight;
        vtkCoordinate *c = dbInfoActor[0]->GetPositionCoordinate();
        c->SetCoordinateSystemToNormalizedViewport();
        c->SetValue(x, y);
        if (!dbInfoIsAdded[0] && mainDBInfoVisible)
        {
#ifndef NO_ANNOTATIONS
            foreground->AddActor2D(dbInfoActor[0]);
#endif
            dbInfoIsAdded[0] = true;
        }
        for (int i = 1 ; i < MAX_LEGENDS ; i++)
        {
            if (dbInfoIsAdded[i])
            {
#ifndef NO_ANNOTATIONS
                foreground->RemoveActor2D(dbInfoActor[i]);
#endif
                dbInfoIsAdded[i] = false;
            }
        }
    }
    else
    {
        dbInfoActor[0]->SetHeight(dbInfoHeight);
        for (int i = 0 ; i < MAX_LEGENDS ; i++)
        {
            float x = (i < MAX_LEGENDS/2 ? leftColumnPosition 
                                         : rightColumnPosition);
            float y = GetPosition(i) + 0.17;
            vtkCoordinate *c = dbInfoActor[i]->GetPositionCoordinate();
            c->SetCoordinateSystemToNormalizedViewport();
            c->SetValue(x, y);
            if (!dbInfoIsAdded[i] && mainDBInfoVisible)
            {
#ifndef NO_ANNOTATIONS
                foreground->AddActor2D(dbInfoActor[i]);
#endif
                dbInfoIsAdded[i] = true;
            }
        }
    }

    for (int i = numLegends ; i < MAX_LEGENDS ; i++)
    {
        if (dbInfoIsAdded[i])
        {
#ifndef NO_ANNOTATIONS
            foreground->RemoveActor2D(dbInfoActor[i]);
#endif
            dbInfoIsAdded[i] = false;
        }
    }
}


// ****************************************************************************
//  Method: VisWinLegends::RemoveDBInfos
//
//  Purpose:
//      Removes the database information from the window.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2002
//
//  Modifications:
//
//    Hank Childs, Tue May 21 08:34:41 PDT 2002
//    Allow annotations to be configured out.
//
// ****************************************************************************

void
VisWinLegends::RemoveDBInfos(void)
{
    vtkRenderer *foreground = mediator.GetForeground();
    for (int i = 0 ; i < MAX_LEGENDS ; i++)
    {
        if (dbInfoIsAdded[i])
        {
#ifndef NO_ANNOTATIONS
            foreground->RemoveActor2D(dbInfoActor[i]);
#endif
            dbInfoIsAdded[i] = false;
        }
    }
}


// ****************************************************************************
//  Method: VisWinLegends::GetPosition
//
//  Purpose:
//      Gets the position for the legend based on how many database infos
//      we have to draw.
//
//  Arguments:
//      pos     The position index.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2002
//
// ****************************************************************************

float
VisWinLegends::GetPosition(int pos)
{
    float allHaveOwnDB[MAX_LEGENDS] = { 0.78, 0.50, 0.22, 0.78, 0.50, 0.22 };
    float onlyOneDB[MAX_LEGENDS] = { 0.72, 0.48, 0.24, 0.80, 0.56, 0.32 };

    return (homogeneous ? onlyOneDB[pos] : allHaveOwnDB[pos]);
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
//   
// ****************************************************************************

void
VisWinLegends::SetVisibility(bool db, bool legend)
{
    mainDBInfoVisible = db;
    legendVisible = legend;
    RemoveDBInfos();
}
