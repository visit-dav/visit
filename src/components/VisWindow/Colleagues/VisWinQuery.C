// ************************************************************************* //
//                              VisWinQuery.C                                //
// ************************************************************************* //

#include <VisWinQuery.h>

#include <VisWindow.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <avtPickActor.h>
#include <avtLineoutActor.h>
#include <PickAttributes.h>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: VisWinQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 09:32:20 PST 2002
//    Removed pickScaleFactor, added hidden.
//
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002 
//    Add support for other query types.
//
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002 
//    Removed designator.
//
// ****************************************************************************

VisWinQuery::VisWinQuery(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    hidden = false;
    type = QUERYTYPE_NONE;
    attachmentPoint[0] = attachmentPoint[1] = attachmentPoint[2] = 0;
    secondaryPoint[0] = secondaryPoint[1] = secondaryPoint[2] = 1;
}


// ****************************************************************************
//  Method: VisWinQuery destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2002
//
// ****************************************************************************

VisWinQuery::~VisWinQuery()
{
}


// ****************************************************************************
//  Method: VisWinQuery::StartBoundingBox
//
//  Purpose:
//    Hides all of the pick point letters. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 09:32:20 PST 2002
//    Removed call to mediator.GetRender.  Set hidden flag 
//
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002 
//    Use avtPickActor methods.
//
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002 
//    Add support for lineout.
//
// ****************************************************************************

void
VisWinQuery::StartBoundingBox(void)
{
    //
    // Hide all of the pick point letters. 
    //
    std::vector< avtPickActor_p >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        (*it)->Hide();
    }
    std::vector< avtLineoutActor_p >::iterator it2;
    for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
    {
        (*it2)->Hide();
    }
    hidden = true;
}


// ****************************************************************************
//  Method: VisWinQuery::EndBoundingBox
//
//  Purpose:
//    Unhides all fo the pick point letters.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 09:32:20 PST 2002
//    Removed call to mediator.GetRender.  Set hidden flag 
//
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002 
//    Use avtPickActor methods.
//
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002 
//    Add support for lineout.
//
// ****************************************************************************

void
VisWinQuery::EndBoundingBox(void)
{
    //
    // Unhide all of the pick points. 
    //
    std::vector< avtPickActor_p >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        (*it)->UnHide();
    }
    std::vector< avtLineoutActor_p >::iterator it2;
    for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
    {
        (*it2)->UnHide();
    }
    hidden = false;
}

 
// ****************************************************************************
//  Method: VisWinInteractions::SetQueryType
//
//  Purpose:
//    Sets the query type for the vis window.
//
//  Arguments:
//    t         The new query type.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 15, 2002 
//
// ****************************************************************************
 
void
VisWinQuery::SetQueryType(QUERY_TYPE t)
{
    type = t;
}


// ****************************************************************************
//  Method: VisWinInteractions::GetQueryType
//
//  Purpose:
//    Returns the current query type.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 15 2002 
//
// ****************************************************************************
 
QUERY_TYPE
VisWinQuery::GetQueryType() const
{
    return type;
}
 
 
// ****************************************************************************
//  Method: VisWinQuery::SetForegroundColor
//
//  Purpose:
//    Sets the color for the pick point letters. 
//
//  Arguments:
//      fr      The red component (rgb) of the foreground.
//      fg      The green component (rgb) of the foreground.
//      fb      The blue component (rgb) of the foreground.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002 
//    Use avtPickActor methods.
//
// ****************************************************************************

void
VisWinQuery::SetForegroundColor(float fr, float fg, float fb)
{
    //
    // Set the color for the pick points.
    //
    std::vector< avtPickActor_p >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        (*it)->SetForegroundColor(fr, fg, fb);
    }
}




// ****************************************************************************
//  Method: VisWinQuery::UpdateView
//
//  Purpose:
//    Updates the pickletters by scaling. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar 18 09:32:20 PST 2002
//    Change code to only compute and set scale factor if we have pick
//    points and they are not hidden. 
//
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002 
//    Use avtPickActor methods.
//
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002 
//    Have mediator calculate needed scale factor for picks. Add Lineout. 
//
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Set scale or lineout actors, as they now have labels. 
//
// ****************************************************************************

void
VisWinQuery::UpdateView()
{
    if (!pickPoints.empty() && !hidden)
    {
        std::vector< avtPickActor_p >::iterator it;
        for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
        {
            const float *pos = (*it)->GetAttachmentPoint();
            (*it)->SetScale(mediator.ComputeVectorTextScaleFactor(pos));
            (*it)->UpdateView();
        }
    }
    if (!lineOuts.empty() && !hidden)
    {
        std::vector< avtLineoutActor_p >::iterator it;
        for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
        {
            const float *pos = (*it)->GetAttachmentPoint();
            (*it)->SetScale(mediator.ComputeVectorTextScaleFactor(pos));
            (*it)->UpdateView();
        }
    }
}


// ****************************************************************************
//  Method: VisWinQuery::SetAttachmentPoint
//
//  Purpose:
//    Sets the attachment point for the current query. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 15, 2002
//
// ****************************************************************************

void
VisWinQuery::SetAttachmentPoint(float x, float y, float z)
{
    attachmentPoint[0] = x;
    attachmentPoint[1] = y;
    attachmentPoint[2] = z;
}


// ****************************************************************************
//  Method: VisWinQuery::SetSecondaryPoint
//
//  Purpose:
//    Sets the secondary point for the current query. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 15, 2002
//
// ****************************************************************************

void
VisWinQuery::SetSecondaryPoint(float x, float y, float z)
{
    secondaryPoint[0] = x;
    secondaryPoint[1] = y;
    secondaryPoint[2] = z;
}


// ****************************************************************************
//  Method: VisWinQuery::QueryIsValid
//
//  Purpose:
//    Adds new visual cue since the query has been deemed valid by the viewer.
//
//  Arguments:
//    designator   The identifier for the valid query.
//    lineAtts     The attributes associated with a Lineout query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Changed argument to Line*, to convey more information.
// 
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002 
//    Added designator argument. Removed code that incremented the old
//    member designator.
// 
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003   
//    Replaced designator argument with PickAttributes.
//
// ****************************************************************************

void
VisWinQuery::QueryIsValid(const PickAttributes *pa, const Line *lineAtts)
{
    switch(type)
    {
        case QUERYTYPE_PICK    : Pick(pa); break;
        case QUERYTYPE_LINEOUT : Lineout(lineAtts); break;
        case QUERYTYPE_NONE    : 
        default                : return;    /* do nothing */
    }

    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinQuery::Pick
//
//  Purpose:
//    Displays a label for the picked position.
//
//  Arguments:
//    designator  Identifies this pick.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 16:08:23 PST 2002 
//    Use avtPickActor methods.
//
//    Kathleen Bonnell, Tue Apr 15 15:38:14 PDT 2002  
//    Use new members 'designator' and 'attachmentPoint'. 
//    
//    Kathleen Bonnell, Thu Dec 19 13:32:47 PST 2002 
//    'designator' is now an argument.
//    
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    'designator' , and attachment point no retrieved from PickAttributes 
//    argument.
//    
//    Kathleen Bonnell, Fri Apr 11 15:20:47 PDT 2003 
//    Force z-coord of attachmentPoint in 2D to be zero.  
//    
//    Kathleen Bonnell, Fri Jun  6 15:17:45 PDT 2003  
//    Added support for FullFrame mode. 
//    
// ****************************************************************************

void 
VisWinQuery::Pick(const PickAttributes *pa)
{
    const float *pt = pa->GetPickPoint();
    attachmentPoint[0] = pt[0];
    attachmentPoint[1] = pt[1];
    attachmentPoint[2] = pt[2];

    avtPickActor_p pp = new avtPickActor;
    if (mediator.GetMode() == WINMODE_3D)
    {
        pp->SetMode3D(true);
    }
    else
    {
        //
        // Due to numerical precision issues on different platforms, the
        // calculated PickPoint in 2D does not always have z == 0.
        // Force it so that the pick letters will always be displayed in 
        // front of the plot.
        //
        attachmentPoint[2] = 0.;
        pp->SetMode3D(false);
    }
    
    pp->SetDesignator(pa->GetPickLetter().c_str());
    pp->SetAttachmentPoint(attachmentPoint);

    float fg[3];
    mediator.GetForegroundColor(fg);
    pp->SetForegroundColor(fg);

    //
    // Pull the pick actors a little closer to the camera to make sure
    // there are no z-buffer errors.  Note that canvas issues are hidden
    // by GetCanvas routine.
    //
    float distance = 0.003;
    float foc[3];
    float pos[3];
    mediator.GetCanvas()->GetActiveCamera()->GetPosition(pos);
    mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint(foc);
    float projection[3];
    projection[0] = distance*(pos[0] - foc[0]);
    projection[1] = distance*(pos[1] - foc[1]);
    projection[2] = distance*(pos[2] - foc[2]);
    pp->Shift(projection);

    if (mediator.GetFullFrameMode())
    {
        double scale;
        int type;

        mediator.GetScaleFactorAndType(scale, type);
        float vec[3] = { 1., 1., 1.};
        if (type == 0) // x_axis
        {
            vec[0] = scale; 
        }
        else // y_axis 
        {
            vec[1] = scale; 
        }
        pp->Translate(vec);
    }

    //
    //  Add the pickpoint to the renderer. 
    //
    pp->Add(mediator.GetCanvas());


    //
    //  Save this for removal later.
    //
    pickPoints.push_back(pp);


    //
    //  Draw the label. 
    //
    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinQuery::ClearAllQueries
//
//  Purpose:
//    Clears the visual cues from the renderer.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 15, 2002 
//
// ****************************************************************************

void 
VisWinQuery::ClearAllQueries()
{
    ClearPickPoints(); 
    ClearLineouts(); 
}


// ****************************************************************************
//  Method: VisWinQuery::ClearQueries
//
//  Purpose:
//    Clears the visual cues from the renderer.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 15, 2002 
//
// ****************************************************************************

void 
VisWinQuery::ClearQueries()
{
    switch(type)
    {
        case QUERYTYPE_PICK : 
            ClearPickPoints(); 
            break;

        case QUERYTYPE_LINEOUT : 
            ClearLineouts();  
            break;
        case QUERYTYPE_NONE :
        default :
            ClearPickPoints(); 
            ClearLineouts(); 
            break;

    }
}


// ****************************************************************************
//  Method: VisWinQuery::ClearPickPoints
//
//  Purpose:
//    Clears all the pick points from the renderer. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2002 
//
// ****************************************************************************
void 
VisWinQuery::ClearPickPoints()
{
    if (pickPoints.empty())
    {
        return;
    }
    std::vector< avtPickActor_p >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        (*it)->Remove();
    }
    pickPoints.clear();
}



// ****************************************************************************
//  Method: VisWinQuery::Lineout
//
//  Purpose:
//    Displays the ref line for LineOut. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 26, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 30 14:44:37 PDT 2002
//    Pull lineout a little closer to camera to avoid z-buffer issues with
//    other plots in the window.
//
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Changed argument to Line*, to allow more information to be passed 
//    besides just color. 
//   
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002
//    Set the actor's designator from the lineAtts designator. Set 
//    attachmentPoint and secondaryPoint from lineAtts. 
//
//    Kathleen Bonnell, Fri Jun  6 15:17:45 PDT 2003  
//    Added support for FullFrame mode. 
//    
// ****************************************************************************

void 
VisWinQuery::Lineout(const Line *lineAtts)
{
    //
    //  Create the lineout actor, and set its properties.
    //

    avtLineoutActor_p lo = new avtLineoutActor;

    lo->SetDesignator(lineAtts->GetDesignator());
    double color[4];
    lineAtts->GetColor().GetRgba(color);
    lo->SetForegroundColor(color[0], color[1], color[2]);

    if (mediator.GetMode() == WINMODE_3D)
    {
        lo->SetMode3D(true);
    }
    else 
    {
        lo->SetMode3D(false);
    }
    lo->SetShowLabels(lineAtts->GetReflineLabels());

    //
    // Pull the lineout actors a little closer to the camera to make sure
    // there are no z-buffer errors.  Note that canvas issues are hidden
    // by GetCanvas routine.
    //
    float distance = 0.003;
    float z_foc, z_pos, z_proj;
    z_pos = mediator.GetCanvas()->GetActiveCamera()->GetPosition()[2];
    z_foc = mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint()[2];
    z_proj = distance*(z_pos - z_foc);

    const double *pt1 = lineAtts->GetPoint1();
    const double *pt2 = lineAtts->GetPoint2();
    attachmentPoint[0] = pt1[0];
    attachmentPoint[1] = pt1[1]; 
    attachmentPoint[2] = pt1[2] + z_proj;

    secondaryPoint[0] = pt2[0];
    secondaryPoint[1] = pt2[1]; 
    secondaryPoint[2] = pt2[2] + z_proj;

    lo->SetAttachmentPoint(attachmentPoint);
    lo->SetPoint2(secondaryPoint);

    if (mediator.GetFullFrameMode())
    {
        double scale;
        int type;

        mediator.GetScaleFactorAndType(scale, type);
        float vec[3] = { 1., 1., 1.};
        if (type == 0) // x_axis
        {
            vec[0] = scale; 
        }
        else // y_axis 
        {
            vec[1] = scale; 
        }
        lo->Translate(vec);
    }

    lo->Add(mediator.GetCanvas());

    //
    //  Save this for removal later.
    //
    lineOuts.push_back(lo);
}



// ****************************************************************************
//  Method: VisWinQuery::ClearLineouts
//
//  Purpose:
//    Clears all the reference lines from the renderer. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 10, 2001 
//
// ****************************************************************************
void 
VisWinQuery::ClearLineouts()
{
    if (lineOuts.empty())
    {
        return;
    }

    std::vector< avtLineoutActor_p >::iterator it;
    for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
    {
        (*it)->Remove();
    }
    lineOuts.clear();
}


// ****************************************************************************
//  Method: VisWinQuery::UpdateQuery
//
//  Purpose:
//    Update a specific lineout actor, based on the passed atts.
//
//  Arguments:
//    lineAtts  The attributes to be used for udpating the lineout actor. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 18, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 30 14:44:37 PDT 2002
//    Pull lineout a little closer to camera to avoid z-buffer issues with
//    other plots in the window.  Removed unnecessary calls to 
//    remove and re-add the lineout actor from the renderer.
//
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Add ability for reflines to display labels. 
//
//    Kathleen Bonnell, Fri Jun  6 15:17:45 PDT 2003  
//    Added support for FullFrame mode. 
//    
// ****************************************************************************
 
void
VisWinQuery::UpdateQuery(const Line *lineAtts)
{
    std::vector< avtLineoutActor_p >::iterator it;
    for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
    {
        if ((*it)->GetDesignator() == lineAtts->GetDesignator())
        {
            //
            // Pull the lineout actors a little closer to the camera to make sure
            // there are no z-buffer errors.  Note that canvas issues are hidden
            // by GetCanvas routine.
            //
            float distance = 0.003;
            float z_foc, z_pos, z_proj;
            z_pos = mediator.GetCanvas()->GetActiveCamera()->GetPosition()[2];
            z_foc = mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint()[2];
            z_proj = distance*(z_pos - z_foc);

            attachmentPoint[0] = lineAtts->GetPoint1()[0];
            attachmentPoint[1] = lineAtts->GetPoint1()[1];
            attachmentPoint[2] = lineAtts->GetPoint1()[2];

            secondaryPoint[0] = lineAtts->GetPoint2()[0];
            secondaryPoint[1] = lineAtts->GetPoint2()[1];
            secondaryPoint[2] = lineAtts->GetPoint2()[2];

            attachmentPoint[2] += z_proj;
            secondaryPoint[2]  += z_proj;
            (*it)->SetAttachmentPoint(attachmentPoint);
            (*it)->SetPoint2(secondaryPoint);

            (*it)->SetLineWidth(lineAtts->GetLineWidth());
            (*it)->SetLineStyle(lineAtts->GetLineStyle());
 
            double c[4];
            lineAtts->GetColor().GetRgba(c);
            (*it)->SetForegroundColor(c[0], c[1], c[2]);
            (*it)->SetShowLabels(lineAtts->GetReflineLabels());
            if (mediator.GetFullFrameMode())
            {
                double scale;
                int type;

                mediator.GetScaleFactorAndType(scale, type);
                float vec[3] = { 1., 1., 1.};
                if (type == 0) // x_axis
                {
                    vec[0] = scale; 
                }
                else // y_axis 
                {
                    vec[1] = scale; 
                }
                (*it)->Translate(vec);
            }
        }
    }
    //
    //  Issue a render call so changes take effect.
    //
    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinQuery::DeleteQuery
//
//  Purpose:
//    Delete a specific lineout actor, based on the passed atts.
//
//  Arguments:
//    lineAtts  The attributes that specify which actor should be deleted. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 18, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 15 15:38:43 PDT 2002 
//    Add break from loop when correct actor found.
//
// ****************************************************************************
 
void
VisWinQuery::DeleteQuery(const Line *lineAtts)
{
    std::vector< avtLineoutActor_p >::iterator it;
    for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
    {
        if ((*it)->GetDesignator() == lineAtts->GetDesignator())
        {
            (*it)->Remove();
            lineOuts.erase(it);
            break;
        }
    }
    //
    //  Issue a render call so changes take effect.
    //
    mediator.Render();
}


// ****************************************************************************
//  Method: VisWinQuery::FullFrameOn
//
//  Purpose:
//    Translates the queries so that they appear in the proper spot in
//    full-frame mode. 
//
//  Arguments:
//    scale    The axis scale factor.
//    type     The axis scale type.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 6, 2003 
//
// ****************************************************************************
 
void
VisWinQuery::FullFrameOn(const double scale, const int type)
{
    if (scale > 0.)
    {
        float vec[3] = { 1., 1., 1.};
        if (type == 0) // x_axis
            vec[0] = scale;
        else           //  y_axis
            vec[1] = scale;
        std::vector< avtPickActor_p >::iterator it;
        for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
        {
            (*it)->Translate(vec);
        }
        std::vector< avtLineoutActor_p >::iterator it2;
        for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
        {
            (*it2)->Translate(vec);
        }
    }
}


// ****************************************************************************
//  Method: VisWinQuery::FullFrameOff
//
//  Purpose:
//    Resets the positions of pick and lineout actors to their original
//    position. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 6, 2003 
//
// ****************************************************************************

void
VisWinQuery::FullFrameOff()
{
    float distance = 0.003;
    float foc[3], pos[3], projection[3];

    mediator.GetCanvas()->GetActiveCamera()->GetPosition(pos);
    mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint(foc);

    projection[0] = distance*(pos[0] - foc[0]);
    projection[1] = distance*(pos[1] - foc[1]);
    projection[2] = distance*(pos[2] - foc[2]);
    std::vector< avtPickActor_p >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        (*it)->ResetPosition(projection);
    }
    std::vector< avtLineoutActor_p >::iterator it2;
    for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
    {
        (*it2)->ResetPosition();
    }
}
