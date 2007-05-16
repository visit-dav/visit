/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//    Kathleen Bonnell, Wed Jun 25 14:30:39 PDT 2003 
//    Removed un-needed members 'type', attachmentPoint, secondaryPoint.
//
// ****************************************************************************

VisWinQuery::VisWinQuery(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    hidden = false;
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
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************

void
VisWinQuery::StartBoundingBox(void)
{
    //
    // Hide all of the pick point letters. 
    //
    std::vector< PickEntry >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        it->pickActor->Hide();
    }
    std::vector< LineEntry >::iterator it2;
    for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
    {
        it2->lineActor->Hide();
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
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************

void
VisWinQuery::EndBoundingBox(void)
{
    //
    // Unhide all of the pick points. 
    //
    std::vector< PickEntry >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        it->pickActor->UnHide();
    }
    std::vector< LineEntry >::iterator it2;
    for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
    {
        it2->lineActor->UnHide();
    }
    hidden = false;
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
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************

void
VisWinQuery::SetForegroundColor(double fr, double fg, double fb)
{
    //
    // Set the color for the pick points.
    //
    std::vector< PickEntry >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        it->pickActor->SetForegroundColor(fr, fg, fb);
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
//    Set scale for lineout actors, as they now have labels. 
//
//    Kathleen Bonnell, Fri Feb 20 12:37:26 PST 2004 
//    Shift and translate pickPoints (in 2D or Curve mode). 
//
//    Kathleen Bonnell, Tue Jun  8 17:42:59 PDT 2004 
//    For picks, use LetterPosition instead of AttachmentPoint to determine
//    scale.
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
//    Kathleen Bonnell, Fri Jun 11 14:49:39 PDT 2004 
//    Add back in the line that computes and sets the Scale for a pick.
//
// ****************************************************************************

void
VisWinQuery::UpdateView()
{
    if (!pickPoints.empty() && !hidden)
    {
        std::vector< PickEntry >::iterator it;

        double transVec[3];
        double shiftVec[3];
        CreateShiftVector(shiftVec, CalculateShiftDistance());
        if (mediator.GetFullFrameMode())
        {
            double scale;
            int type;
            mediator.GetScaleFactorAndType(scale, type);
            shiftVec[2] /= scale;
            CreateTranslationVector(transVec);
        }

        for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
        {
            const double *pos = it->pickActor->GetLetterPosition();
            it->pickActor->SetScale(mediator.ComputeVectorTextScaleFactor(pos));
            if (mediator.GetMode() != WINMODE_3D)
            {
                it->pickActor->Shift(shiftVec);
                if (mediator.GetFullFrameMode())
                {
                    it->pickActor->Translate(transVec);
                }
            }
            it->pickActor->UpdateView();
        }
    }
    if (!lineOuts.empty() && !hidden)
    {
        std::vector< LineEntry >::iterator it;
        for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
        {
            const double *pos = it->lineActor->GetAttachmentPoint();
            it->lineActor->SetScale(mediator.ComputeVectorTextScaleFactor(pos));
            it->lineActor->UpdateView();
        }
    }
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
//    Kathleen Bonnell, Wed Jun 25 14:30:39 PDT 2003 
//    Removed QueryType. 
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Modified to use VisualCueInfo arguments
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Removed 'else' from test for point or line. Added call
//    to RecalculateRenderOrder
// ****************************************************************************

void
VisWinQuery::QueryIsValid(const VisualCueInfo *vqPoint, const VisualCueInfo *vqLine)
{
    if (vqPoint != NULL)
    {
        Pick(vqPoint);
    }
    if (vqLine != NULL)
    {
        Lineout(vqLine);
    }
    mediator.RecalculateRenderOrder();
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
//    Kathleen Bonnell, Tue Jun 24 18:26:44 PDT 2003 
//    To resolve z-buffer issues in 2d, make sure the z-coord of the
//    attachment point is moved towards the camera. 
//    
//    Kathleen Bonnell, Fri Jun 27 16:39:52 PDT 2003   
//    Set pickActor's attachment point directly, rather than through 
//    (now defunct) member attachmentPoint. Turn on glyph if NodePick.
//    
//    Kathleen Bonnell, Mon Jul  7 16:46:12 PDT 2003  
//    Un-scale the projection if in FullFrame mode. 
//    
//    Kathleen Bonnell, Tue Dec  2 17:43:08 PST 2003 
//    Use the glyph for curve picks, too. 
//    
//    Kathleen Bonnell, Fri Feb 20 12:37:26 PST 2004 
//    Modified code to more accurately calculate a shift vector and distance
//    for 2d points. 
//    
//    Kathleen Bonnell, Tue Jun  1 15:26:10 PDT 2004 
//    Account for expansion of pick types. 
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Modified to use VisualCueInfo arguments
//
//    Brad Whitlock, Tue Jun 29 10:53:44 PDT 2004
//    Fixed so it works with MSVC++ 6.0 again.
//
//    Kathleen Bonnell, Mon Jun 27 15:19:14 PDT 2005 
//    Ensure that the attachment point's z-value for 2D is larger than
//    the maximum amount that any plot has been shifted in z. (otherwise
//    pick letters may not be visible).
//
// ****************************************************************************

void 
VisWinQuery::Pick(const VisualCueInfo *vq)
{
    double pt[3];
    vq->GetPointD(0,pt);

    avtPickActor_p pp = new avtPickActor;
    double distance = CalculateShiftDistance();
    if (mediator.GetMode() == WINMODE_3D)
    {
        pp->SetMode3D(true);
        pp->SetAttachmentPoint(pt[0], pt[1], pt[2]);
    }
    else
    {
        pp->SetMode3D(false);
        double maxShift = mediator.GetMaxPlotZShift();
        if (maxShift > distance)
            distance += maxShift;
        pp->SetAttachmentPoint(pt[0], pt[1], distance);
    }

    pp->UseGlyph(vq->GetGlyphType() != "");
    
    pp->SetDesignator(vq->GetLabel().c_str());

    double fg[3];
    mediator.GetForegroundColor(fg);
    pp->SetForegroundColor(fg);

    //
    // Pull the pick actors a little closer to the camera to make sure
    // there are no z-buffer errors.  Note that canvas issues are hidden
    // by GetCanvas routine.
    //

    double shiftVec[3];
    CreateShiftVector(shiftVec, distance);

    if (mediator.GetFullFrameMode())
    {
        double scale;
        int type;

        mediator.GetScaleFactorAndType(scale, type);
        // Un-scale the projection so that pick letters don't go off the
        // screen.  
        shiftVec[2] /= scale;
        pp->Shift(shiftVec);

        double transVec[3];
        CreateTranslationVector(scale, type, transVec);
        pp->Translate(transVec);
    }
    else
    {
        pp->Shift(shiftVec);
    }


    //
    //  Add the pickpoint to the renderer. 
    //
    pp->Add(mediator.GetCanvas());


    //
    //  Save this for removal later.
    //
    PickEntry tmp;
    tmp.pickActor = pp;
    tmp.vqInfo = *vq;
    pickPoints.push_back(tmp);


    //
    //  Draw the label. 
    //
    mediator.Render();
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
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
//    Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004 
//    Added arg to specify which type of pick points to remove (all, 3d or 2d). 
//
// ****************************************************************************
void 
VisWinQuery::ClearPickPoints(int which)
{ 
    // which == 0, all
    // which == 1, 3d
    // which == 2, 2d
    if (pickPoints.empty())
    {
        return;
    }
    std::vector< PickEntry >::iterator it;
    for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
    {
        if (which == 0) // all
            it->pickActor->Remove();
        else if (which == 1 &&  it->pickActor->GetMode3D()) // 3D
            it->pickActor->Remove();
        else if (which == 2 && !it->pickActor->GetMode3D()) // 2D
            it->pickActor->Remove();
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
//    Kathleen Bonnell, Fri Jun 27 16:39:52 PDT 2003   
//    Set lineoutActor's attachment and secondary points directly, rather
//    than through (now defunct) members attachmentPoint and secondaryPoint. 
//    
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Modified to use VisualCueInfo argument
//
//    Brad Whitlock, Tue Jun 29 10:55:15 PDT 2004
//    I fixed the code so it builds with MSVC++ 6.0 again.
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added code to set lo's line width and line style
//
//    Kathleen Bonnell, Tue May 15 13:46:22 PDT 2007
//    Only project the z-coords of a 2d lineout. 
//
// ****************************************************************************

void 
VisWinQuery::Lineout(const VisualCueInfo *vq)
{
    //
    //  Create the lineout actor, and set its properties.
    //

    avtLineoutActor_p lo = new avtLineoutActor;

    lo->SetDesignator(vq->GetLabel());
    double color[4];
    vq->GetColor().GetRgba(color);
    lo->SetForegroundColor(color[0], color[1], color[2]);

    double pt1[3];
    double pt2[3];
    vq->GetPointD(0,pt1);
    vq->GetPointD(1,pt2);
    if (mediator.GetMode() == WINMODE_3D)
    {
        lo->SetMode3D(true);
    }
    else 
    {
        lo->SetMode3D(false);
        //
        // Pull the lineout actors a little closer to the camera to make sure
        // there are no z-buffer errors.  Note that canvas issues are hidden
        // by GetCanvas routine.
        //
        double distance = 0.003;
        double z_foc, z_pos, z_proj;
        z_pos = mediator.GetCanvas()->GetActiveCamera()->GetPosition()[2];
        z_foc = mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint()[2];
        z_proj = distance*(z_pos - z_foc);
        pt1[2] += z_proj;
        pt2[2] += z_proj;
    }

    lo->SetAttachmentPoint(pt1[0], pt1[1], pt1[2]);
    lo->SetPoint2(pt2[0], pt2[1], pt2[2]);

    lo->SetShowLabels(vq->GetShowLabel());
    lo->SetLineWidth(vq->GetLineWidth());
    lo->SetLineStyle(vq->GetLineStyle());

    if (mediator.GetFullFrameMode())
    {
        double transVec[3];
        CreateTranslationVector(transVec);
        lo->Translate(transVec);
    }

    lo->Add(mediator.GetCanvas());

    //
    //  Save this for removal later.
    //
    LineEntry tmp;
    tmp.lineActor = lo;
    tmp.vqInfo = *vq;
    lineOuts.push_back(tmp);
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
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************
void 
VisWinQuery::ClearLineouts()
{
    if (lineOuts.empty())
    {
        return;
    }

    std::vector< LineEntry >::iterator it;
    for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
    {
        it->lineActor->Remove();
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
//    Kathleen Bonnell, Fri Jun 27 16:39:52 PDT 2003   
//    Set lineoutActor's attachment and secondary points directly, rather
//    than through (now defunct) members attachmentPoint and secondaryPoint. 
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Added code to store visual cue info back into the cache
//    
// ****************************************************************************
 
void
VisWinQuery::UpdateQuery(const VisualCueInfo *vq)
{
    std::vector< LineEntry >::iterator it;
    for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
    {
        if (it->lineActor->GetDesignator() == vq->GetLabel())
        {
            //
            // Pull the lineout actors a little closer to the camera to make sure
            // there are no z-buffer errors.  Note that canvas issues are hidden
            // by GetCanvas routine.
            //
            double distance = 0.003;
            double z_foc, z_pos, z_proj;
            z_pos = mediator.GetCanvas()->GetActiveCamera()->GetPosition()[2];
            z_foc = mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint()[2];
            z_proj = distance*(z_pos - z_foc);

            double pt1[3];
            double pt2[3];
            vq->GetPointD(0,pt1);
            vq->GetPointD(1,pt2);

            it->lineActor->SetAttachmentPoint(pt1[0], pt1[1], pt1[2] + z_proj);
            it->lineActor->SetPoint2(pt2[0], pt2[1], pt2[2] + z_proj);

            it->lineActor->SetLineWidth(vq->GetLineWidth());
            it->lineActor->SetLineStyle(vq->GetLineStyle());
 
            double c[4];
            vq->GetColor().GetRgba(c);
            it->lineActor->SetForegroundColor(c[0], c[1], c[2]);
            it->lineActor->SetShowLabels(vq->GetShowLabel());
            if (mediator.GetFullFrameMode())
            {
                double transVec[3];
                CreateTranslationVector(transVec);
                it->lineActor->Translate(transVec);
            }

            //
            // Finally, update the vqInfo associated with this lineActor in
            // the list of lineOuts
            //
            it->vqInfo = *vq;
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
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************
 
void
VisWinQuery::DeleteQuery(const VisualCueInfo *vq)
{
    std::vector< LineEntry >::iterator it;
    for (it = lineOuts.begin() ; it != lineOuts.end() ; it++)
    {
        if (it->lineActor->GetDesignator() == vq->GetLabel())
        {
            it->lineActor->Remove();
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
//  Modifications:
//    Kathleen Bonnell, Fri Feb 20 12:37:26 PST 2004
//    Call new method 'CreateTranslationVector'.
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************
 
void
VisWinQuery::FullFrameOn(const double scale, const int type)
{
    if (scale > 0. && (!pickPoints.empty() || !lineOuts.empty()))
    {
        double vec[3]; 
        CreateTranslationVector(scale, type, vec);
        std::vector< PickEntry >::iterator it;
        for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
        {
            it->pickActor->Translate(vec);
        }
        std::vector< LineEntry >::iterator it2;
        for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
        {
            it2->lineActor->Translate(vec);
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
//  Modifications:
//    Kathleen Bonnell, Fri Feb 20 12:37:26 PST 2004
//    Use new methods 'CalculateShiftDistance' & 'CreateShiftVector'.
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************

void
VisWinQuery::FullFrameOff()
{
    if (!pickPoints.empty())
    {
        double distance = CalculateShiftDistance();
        double shiftVec[3];
        CreateShiftVector(shiftVec, distance);
        std::vector< PickEntry >::iterator it;
        for (it = pickPoints.begin() ; it != pickPoints.end() ; it++)
        {
            it->pickActor->ResetPosition(shiftVec);
        }
    }
    if (!lineOuts.empty())
    {
        std::vector< LineEntry >::iterator it2;
        for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
        {
            it2->lineActor->ResetPosition();
        }
    }
}


// ****************************************************************************
//  Method: VisWinQuery::ReAddToWindow
//
//  Purpose:
//    Removes and re-adds lineouts to the RenderWindow so that they appear
//    in front of plots. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 8, 2003 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Modified to use PickEntry and LineEntry vectors
//
// ****************************************************************************

void
VisWinQuery::ReAddToWindow()
{
    if (lineOuts.empty())
        return;

    std::vector< LineEntry >::iterator it2;
    for (it2 = lineOuts.begin() ; it2 != lineOuts.end() ; it2++)
    {
        it2->lineActor->Remove();
        it2->lineActor->Add(mediator.GetCanvas());
    }
}


// ****************************************************************************
//  Method: VisWinQuery::CreateTranslationVector
//
//  Purpose:
//    Creates a vector to be used in translating a query. 
//
//  Arguments: 
//    vec       A place to store the translation vector. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 20, 2004 
//
// ****************************************************************************

void
VisWinQuery::CreateTranslationVector(double vec[3])
{
    double scale;
    int type;
    mediator.GetScaleFactorAndType(scale, type);
    CreateTranslationVector(scale, type, vec);
}


// ****************************************************************************
//  Method: VisWinQuery::CreateTranslationVector
//
//  Purpose:
//    Creates a vector to be used in translating a query. 
//
//  Arguments:
//     s        The scale factor.
//     t        The scale type.
//     vec      A place to store the translation vector.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 20, 2004 
//
// ****************************************************************************

void
VisWinQuery::CreateTranslationVector(const double s, const int t, double vec[3])
{
    if (t == 0) // x_axis
    {
        vec[0] = s; 
        vec[1] = 1;
        vec[2] = 1; 
    }
    else // y_axis 
    {
        vec[0] = 1;
        vec[1] = s; 
        vec[2] = 1;
    }
}


// ****************************************************************************
//  Method: VisWinQuery::CreateShiftVector
//
//  Purpose:
//    Creates a vector to be used in shifting a query. 
//
//  Arguments:
//     vec      A place to store the translation vector.
//     dist     The distance along the vector to shift.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 20, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  8 17:42:59 PDT 2004
//    Handle FullFrame mode differently than non-FullFrame.
//
// ****************************************************************************

void
VisWinQuery::CreateShiftVector(double vec[3], const double distance)
{
    if (mediator.GetMode() == WINMODE_3D)
    {
        double pos[3], foc[3];
        mediator.GetCanvas()->GetActiveCamera()->GetPosition(pos);
        mediator.GetCanvas()->GetActiveCamera()->GetFocalPoint(foc);
        vec[0] = distance*(pos[0] - foc[0]);
        vec[1] = distance*(pos[1] - foc[1]);
        vec[2] = distance*(pos[2] - foc[2]);
    }
    else 
    {
        if (mediator.GetFullFrameMode())
        {
            vec[0] = 0;
            vec[1] = distance;
            vec[2] = 0;
        }
        else
        {
            vec[0] = distance;
            vec[1] = distance;
            vec[2] = 0;
        }
    }
}


// ****************************************************************************
//  Method: VisWinQuery::CalculateShiftDistance
//
//  Purpose:
//    Calculates how far along the shift vector a query should be shifted.
//
//  Returns:
//    The shift distance. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 20, 2004 
//
// ****************************************************************************

double
VisWinQuery::CalculateShiftDistance()
{
    double distance = 0.003;

    if (mediator.GetMode() == WINMODE_3D)
    {
        return distance;
    }

    vtkRenderer *ren = mediator.GetCanvas();
    int *size = ren->GetSize();
    double wp1[4] = {0., 0., 0., 0.};
    double wp2[4] = {0., 0., 0., 0.};
    double x = 0., y = 0., z = 0.;
    ren->SetDisplayPoint(x, y, z);
    ren->DisplayToWorld();
    ren->GetWorldPoint(wp1);
    if (wp1[3])
    {
        wp1[0] /= wp1[3];
        wp1[1] /= wp1[3];
    }
    x = (double) size[0];
    y = (double) size[1];
    z = 0.;
    ren->SetDisplayPoint(x, y, z);
    ren->DisplayToWorld();
    ren->GetWorldPoint(wp2);
    if (wp2[3])
    {
        wp2[0] /= wp2[3];
        wp2[1] /= wp2[3];
    }
    double xDiff = fabs(wp2[0] - wp1[0]);
    double yDiff = fabs(wp2[1] - wp1[1]);
    double minDiff  = (xDiff < yDiff ? xDiff : yDiff); 
    if (minDiff *distance < distance)
        distance *= minDiff ;

    return distance;
}


// ****************************************************************************
//  Method: VisWinQuery::GetVisualCues
//
//  Purpose:
//    Returns a vector of visual cues of the specified type currently in the
//    VisWindow
//
//  Programmer: Mark C. Miller 
//  Creation:   June 7, 2004 
//
// ****************************************************************************
void
VisWinQuery::GetVisualCues(const VisualCueInfo::CueType cueType,
    std::vector<const VisualCueInfo*>& cues) const
{
    int i;

    if ((cueType == VisualCueInfo::PickPoint) ||
        (cueType == VisualCueInfo::Unknown))
    {
        for (i = 0; i < pickPoints.size(); i++)
            cues.push_back(&(pickPoints[i].vqInfo));
    }

    if ((cueType == VisualCueInfo::RefLine) ||
        (cueType == VisualCueInfo::Unknown))
    {
        for (i = 0; i < lineOuts.size(); i++)
            cues.push_back(&(lineOuts[i].vqInfo));
    }
}


// ****************************************************************************
//  Method: VisWinQuery::Start2DMode
//
//  Purpose:
//    Clears the pick points that are not appropriate for 2D mode. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

void             
VisWinQuery::Start2DMode()
{
    // Remove 3d pick actors if they exists
    ClearPickPoints(1);
}

// ****************************************************************************
//  Method: VisWinQuery::StartCurveMode
//
//  Purpose:
//    Clears the pick points that are not appropriate for Curve mode. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

void             
VisWinQuery::StartCurveMode()
{
    // Remove 3d pick actors if they exists
    ClearPickPoints(1);
}


// ****************************************************************************
//  Method: VisWinQuery::Start3DMode
//
//  Purpose:
//    Clears the pick points that are not appropriate for 3D mode. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

void             
VisWinQuery::Start3DMode()
{
    // Remove 2d pick actors if they exists
    ClearPickPoints(2);
}
