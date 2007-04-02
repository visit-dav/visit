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

#include <avtLineoutActor.h>

#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>
#include <LineAttributes.h>

#include <DebugStream.h>

// ****************************************************************************
//  Constructor:  avtLineoutActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002  
//    Initialize new members: hidden, pt2, labelActor1, labelActor1, 
//    showLabels.
//
//    Kathleen Bonnell, Thu Oct  3 14:41:19 PDT 2002  
//    Disable lighting by setting ambient/diffuse coefficients for the actors. 
// 
// ****************************************************************************

avtLineoutActor:: avtLineoutActor()
{
    designator = " ";
    mode3D  = true;
    hidden  = false;
    attach[0] = attach[1] = attach[2] = 0.;
    pt2[0] = pt2[1] = pt2[2] = 0.;

    //
    //  Set up the pipeline for the line actor.
    //
    lineSource  = vtkLineSource::New();
        lineSource->SetResolution(1);   // only need one line segment here
    lineMapper  = vtkPolyDataMapper::New();
        lineMapper->SetInput(lineSource->GetOutput());
    lineActor   = vtkActor::New();
        lineActor->SetMapper(lineMapper); 
        lineActor->PickableOff(); 
        lineActor->GetProperty()->SetColor(0., 0., 0.);
        lineActor->GetProperty()->SetAmbient(1.);
        lineActor->GetProperty()->SetDiffuse(0.);

    labelActor1 = vtkFollower::New();
        labelActor1->GetProperty()->SetColor(0., 0., 0.);
        labelActor1->GetProperty()->SetAmbient(1.);
        labelActor1->GetProperty()->SetDiffuse(0.);
        labelActor1->SetScale(0.5);
        labelActor1->VisibilityOff();
    labelActor2 = vtkFollower::New();
        labelActor2->GetProperty()->SetColor(0., 0., 0.);
        labelActor2->GetProperty()->SetAmbient(1.);
        labelActor2->GetProperty()->SetDiffuse(0.);
        labelActor2->SetScale(0.5);
        labelActor2->PickableOff();
        labelActor2->VisibilityOff();

    renderer = NULL; 
    showLabels = false;
}


// ****************************************************************************
//  Destructor:  avtLineoutActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002  
//    Delete new members: labelActor1, labelActor1.
//
// ****************************************************************************

avtLineoutActor::~avtLineoutActor()
{
    Remove();
    if (labelActor1 != NULL)
    {
        labelActor1->Delete();
        labelActor1 = NULL;
    }
    if (labelActor2 != NULL)
    {
        labelActor2->Delete();
        labelActor2 = NULL;
    }
    if (lineActor != NULL)
    {
        lineActor->Delete();
        lineActor = NULL;
    }
    if (lineMapper != NULL)
    {
        lineMapper->Delete();
        lineMapper = NULL;
    }
    if (lineSource != NULL)
    {
        lineSource->Delete();
        lineSource = NULL;
    }
}



// ****************************************************************************
//  Method:  avtLineoutActor::Add
//
//  Purpose:  Add actors to the renderer, and renderer to this class.
//
//  Arguments:
//    ren     The renderer to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Add the label actors to the renderer. Set camera. 
// ****************************************************************************

void 
avtLineoutActor::Add(vtkRenderer *ren)
{
    renderer = ren;
    labelActor1->SetCamera(renderer->GetActiveCamera());
    labelActor2->SetCamera(renderer->GetActiveCamera());
    renderer->AddActor(labelActor1);
    renderer->AddActor(labelActor2);
    renderer->AddActor(lineActor);
}


// ****************************************************************************
//  Method:  avtLineoutActor::Remove
//
//  Purpose:  Remove actors from the renderer, and renderer from this class.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Remove label actors.
//
// ****************************************************************************

void 
avtLineoutActor::Remove()
{
    if (renderer != NULL)
    {
        renderer->RemoveActor(lineActor);
        renderer->RemoveActor(labelActor2);
        renderer->RemoveActor(labelActor1);
        renderer = NULL;
    }
}


// ****************************************************************************
//  Method:  avtLineoutActor::Shift
//
//  Purpose:  Shift the lineout towards the camera to avoid it being
//            obscured by the plot. 
//
//  Arguments:
//    vec     The vector to use for shifting.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
// ****************************************************************************

void 
avtLineoutActor::Shift(const double vec[3])
{
}

// ****************************************************************************
//  Method:  avtLineoutActor::SetMode3D
//
//  Purpose: Set mode (3d on or off). 
//
//  Arguments:
//    mode   True if 3d mode is on. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 26, 2002 
//
// ****************************************************************************

void 
avtLineoutActor::SetMode3D(const bool mode)
{
    mode3D = mode;
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetAttachmentPoint
//
//  Purpose: Set actors' postion.
//
//  Arguments:
//    x, y, z    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set position for first label actor.
//
//    Kathleen Bonnell, Wed Jun 25 15:12:24 PDT 2003  
//    Changed argument from float[3] to 3 floats.
//
// ****************************************************************************

void 
avtLineoutActor::SetAttachmentPoint(double x, double y, double z)
{
    attach[0] = x; 
    attach[1] = y; 
    attach[2] = z; 
    lineSource->SetPoint1(x, y, z);
    labelActor1->SetPosition(x, y, z);
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetPoint2
//
//  Purpose: Set actors' postion.
//
//  Arguments:
//    pos    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set position for second label actor.
//
//    Kathleen Bonnell, Wed Jun 25 15:12:24 PDT 2003  
//    Changed argument from float[3] to 3 floats. 
//
// ****************************************************************************

void 
avtLineoutActor::SetPoint2(double x, double y, double z)
{
    pt2[0] = x;
    pt2[1] = y;
    pt2[2] = z;
    lineSource->SetPoint2(x, y, z);
    labelActor2->SetPosition(x, y, z);
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetDesignator
//
//  Purpose:  Sets the designator for a pick.  In this instance it
//            is text for labelActor.
//
//  Arguments:
//    l       The designator string.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002 
//    Made the designator a string, and set it.
//
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Create vector text for the labels and set their mappers. 
//
// ****************************************************************************

void avtLineoutActor::SetDesignator(const std::string &designator_)
{
    designator = designator_; 
    vtkVectorText *vecText = vtkVectorText::New();
    vecText->SetText(designator.c_str());

    vtkPolyDataMapper *pdmapper = vtkPolyDataMapper::New();
    pdmapper->SetInput(vecText->GetOutput());

    labelActor1->SetMapper(pdmapper);
    labelActor2->SetMapper(pdmapper);

    pdmapper->Delete();
    vecText->Delete();
}


// ****************************************************************************
//  Method:  avtLineoutActor::GetDesignator
//
//  Purpose:  Gets the designator for a pick.  
//
//  Returns:  The designator string.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2002 
//
// ****************************************************************************

const std::string 
avtLineoutActor::GetDesignator() const
{
    return designator; 
}


// ****************************************************************************
//  Method:  avtLineoutActor::GetDesignator
//
//  Purpose:  Gets the designator for a pick.  
//
//  Returns:  The designator string.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2002 
//
// ****************************************************************************

std::string 
avtLineoutActor::GetDesignator() 
{
    return designator; 
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetForegroundColor
//
//  Purpose:  Set the actors' color.
//
//  Arguments:
//    fg         The rgb components of the foreground color.
//   
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set color for label actors.
//
// ****************************************************************************

void avtLineoutActor::SetForegroundColor(double fg[3])
{
    lineActor->GetProperty()->SetColor(fg);
    labelActor1->GetProperty()->SetColor(fg);
    labelActor2->GetProperty()->SetColor(fg);
}

// ****************************************************************************
//  Method:  avtLineoutActor::SetForegroundColor
//
//  Purpose: Set the actors' color.
//
//  Arguments:
//    r      The red component of the foreground color.
//    g      The green component of the foreground color.
//    b      The blue component of the foreground color.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set color for label actors.
//
// ****************************************************************************

void avtLineoutActor::SetForegroundColor(double r, double g, double b)
{
    lineActor->GetProperty()->SetColor(r, g, b);
    labelActor1->GetProperty()->SetColor(r, g, b);
    labelActor2->GetProperty()->SetColor(r, g, b);
}


// ****************************************************************************
//  Method:  avtLineoutActor::Hide
//
//  Purpose: Make the actors invisible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Turn off labelactor visibility.
// ****************************************************************************

void avtLineoutActor::Hide()
{
    hidden = true;
    lineActor->VisibilityOff();
    labelActor1->VisibilityOff();
    labelActor2->VisibilityOff();
}


// ****************************************************************************
//  Method:  avtLineoutActor::UnHide
//
//  Purpose:  Make the actors visible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Turn on labelActor visibility only if they should be shown.
// ****************************************************************************

void avtLineoutActor::UnHide()
{
    hidden = false;
    lineActor->VisibilityOn();
    if (showLabels)
    {
      labelActor1->VisibilityOn();
      labelActor2->VisibilityOn();
    }
}


// ****************************************************************************
//  Method:  avtLineoutActor::UpdateView
//
//  Purpose:   Peform whatever updates are necessary when the view changes.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
// ****************************************************************************

void avtLineoutActor::UpdateView()
{
    // nothing for now.
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetLineStyle
//
//  Purpose: Set the actors' line style.
//
//  Arguments:
//    ls     The line style to use (in VisIts's gui's format).
//
//  Programmer:  Kathleen Bonnell
//  Creation:    June 18, 2002 
//
// ****************************************************************************
 
void avtLineoutActor::SetLineStyle(int ls)
{
    //
    //  Need to convert from gui int to LineStyle to gl line stipple pattern.
    //
    lineActor->GetProperty()->SetLineStipplePattern(
        LineStyle2StipplePattern(Int2LineStyle(ls)));
}
 
 
// ****************************************************************************
//  Method:  avtLineoutActor::SetLineWidth
//
//  Purpose: Set the actors' line width.
//
//  Arguments:
//    lw     The line style to use (in VisIts's gui's format).
//
//  Programmer:  Kathleen Bonnell
//  Creation:    June 18, 2002 
//
// ****************************************************************************
 
void avtLineoutActor::SetLineWidth(int lw)
{ 
    //
    //  Need to convert from gui int to LineWidth to gl int.
    //
    lineActor->GetProperty()->SetLineWidth(LineWidth2Int(Int2LineWidth(lw)));
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetScale
//
//  Purpose:  Set the scale for label actors.
//
//  Arguments:
//    s       The scale factor to use.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    October 01, 2002 
//
// ****************************************************************************
 
void
avtLineoutActor::SetScale(double s)
{
    labelActor1->SetScale(s);
    labelActor2->SetScale(s);
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetShowLabels
//
//  Purpose: Set visibility of labels (on or off). 
//
//  Arguments:
//    mode   True if labels should be visible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 01, 2002 
//
// ****************************************************************************

void 
avtLineoutActor::SetShowLabels(const bool mode)
{
    if (showLabels != mode)
    {
        showLabels = mode; 
        if (showLabels && !hidden)
        {
            labelActor1->VisibilityOn();
            labelActor2->VisibilityOn();
        }
        else 
        {
            labelActor1->VisibilityOff();
            labelActor2->VisibilityOff();
        }
    }
}


// ****************************************************************************
//  Method:  avtLineoutActor::Translate
//
//  Purpose:   Translates the actor by the given vector. 
//
//  Arguments:
//    vec      The vector to use in translating the actor.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 6, 2003
//
//  Modifications:
//
// ****************************************************************************

void 
avtLineoutActor::Translate(const double vec[3])
{
    double *newPos = labelActor1->GetPosition();
    newPos[0] *= vec[0];
    newPos[1] *= vec[1]; 
    newPos[2] *= vec[2];

    newPos = labelActor2->GetPosition();
    newPos[0] *= vec[0];
    newPos[1] *= vec[1]; 
    newPos[2] *= vec[2];

    newPos = lineSource->GetPoint1();
    newPos[0] *= vec[0];
    newPos[1] *= vec[1]; 
    newPos[2] *= vec[2];

    newPos = lineSource->GetPoint2();
    newPos[0] *= vec[0];
    newPos[1] *= vec[1]; 
    newPos[2] *= vec[2];

    lineSource->Modified();
}


// ****************************************************************************
//  Method:  avtLineoutActor::ResetPosition
//
//  Purpose:   Resets the actor's position to the original location. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 6, 2003
//
//  Modifications:
//
// ****************************************************************************

void 
avtLineoutActor::ResetPosition()
{
    labelActor1->SetPosition(attach[0], attach[1], attach[2]);
    labelActor2->SetPosition(pt2[0], pt2[1], pt2[2]);

    lineSource->SetPoint1(attach[0], attach[1], attach[2]);
    lineSource->SetPoint2(pt2[0], pt2[1], pt2[2]);
    lineSource->Modified();
}

