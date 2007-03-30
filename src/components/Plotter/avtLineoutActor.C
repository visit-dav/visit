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
avtLineoutActor::Shift(const float vec[3])
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
//    pos    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set position for first label actor.
//
// ****************************************************************************

void 
avtLineoutActor::SetAttachmentPoint(const float pos[3])
{
    attach[0] = pos[0];
    attach[1] = pos[1];
    attach[2] = pos[2];
    lineSource->SetPoint1(pos[0], pos[1], pos[2]);
    labelActor1->SetPosition(pos[0], pos[1], pos[2]);
}


// ****************************************************************************
//  Method:  avtLineoutActor::SetAttachmentPoint
//
//  Purpose: Set actors' postion.
//
//  Arguments:
//    pos    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 18, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set position for first label actor.
//
// ****************************************************************************

void 
avtLineoutActor::SetAttachmentPoint(const double pos[3])
{
    attach[0] = (float)pos[0];
    attach[1] = (float)pos[1];
    attach[2] = (float)pos[2];
    lineSource->SetPoint1(attach[0], attach[1], attach[2]);
    labelActor1->SetPosition(attach[0], attach[1], attach[2]);
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
// ****************************************************************************

void 
avtLineoutActor::SetPoint2(const float pos[3])
{
    pt2[0] = pos[0];
    pt2[1] = pos[1];
    pt2[2] = pos[2];
    lineSource->SetPoint2(pos[0], pos[1], pos[2]);
    labelActor2->SetPosition(pos[0], pos[1], pos[2]);
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
//  Creation:    June 18, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:07:40 PDT 2002 
//    Set position for second label actor.
//
// ****************************************************************************

void 
avtLineoutActor::SetPoint2(const double pos[3])
{
    pt2[0] = (float)pos[0];
    pt2[1] = (float)pos[1];
    pt2[2] = (float)pos[2];
    lineSource->SetPoint2((float)pos[0], (float)pos[1], (float)pos[2]);
    labelActor2->SetPosition((float)pos[0], (float)pos[1], (float)pos[2]);
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

void avtLineoutActor::SetForegroundColor(float fg[3])
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

void avtLineoutActor::SetForegroundColor(float r, float g, float b)
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
avtLineoutActor::SetScale(float s)
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
avtLineoutActor::Translate(const float vec[3])
{
    float *newPos = labelActor1->GetPosition();
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

