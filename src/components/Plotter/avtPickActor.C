#include <avtPickActor.h>

#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>
#include <vtkGlyphSource2D.h>

#include <DebugStream.h>

// ****************************************************************************
//  Constructor:  avtPickActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct  3 14:41:19 PDT 2002 
//    Disable lighting by setting ambient/diffuse coefficients.
//
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003  
//    Added glyph for NodePicking. 
//
// ****************************************************************************

avtPickActor:: avtPickActor()
{
    mode3D  = true;
    useGlyph  = false;
    attach[0] = attach[1] = attach[2] = 0.;

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

    letterActor = vtkFollower::New(); 
        letterActor->GetProperty()->SetColor(0., 0., 0.);
        letterActor->SetScale(0.5);
        letterActor->PickableOff();
        letterActor->GetProperty()->SetAmbient(1.);
        letterActor->GetProperty()->SetDiffuse(0.);

    glyphSource = vtkGlyphSource2D::New();
        glyphSource->SetGlyphTypeToSquare();
        glyphSource->CrossOff();
        glyphSource->FilledOn();
        glyphSource->SetScale(0.5);
    glyphMapper = vtkPolyDataMapper::New();
        glyphMapper->SetInput(glyphSource->GetOutput());
        glyphMapper->ScalarVisibilityOff();
    glyphActor = vtkFollower::New();
        glyphActor->SetMapper(glyphMapper);
        glyphActor->SetScale(0.5);
        glyphActor->PickableOff();
        glyphActor->GetProperty()->SetColor(0., 0., 0.);
        glyphActor->GetProperty()->SetAmbient(1.);
        glyphActor->GetProperty()->SetDiffuse(0.);


    renderer = NULL; 
}


// ****************************************************************************
//  Destructor:  avtPickActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Delete glyphActor, glyphMapper, glyphSource
//
// ****************************************************************************

avtPickActor::~avtPickActor()
{
    Remove();
    if (glyphActor != NULL)
    {
        glyphActor->Delete();
        glyphActor = NULL;
    }
    if (glyphMapper != NULL)
    {
        glyphMapper->Delete();
        glyphMapper = NULL;
    }
    if (glyphSource != NULL)
    {
        glyphSource->Delete();
        glyphSource = NULL;
    }
    if (letterActor != NULL)
    {
        letterActor->Delete();
        letterActor = NULL;
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
//  Method:  avtPickActor::Add
//
//  Purpose:  Add actors to the renderer, and renderer to this class.
//
//  Arguments:
//    ren     The renderer to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Add glyphActor to renderer when required.
//
// ****************************************************************************

void 
avtPickActor::Add(vtkRenderer *ren)
{
    renderer = ren;
    letterActor->SetCamera(renderer->GetActiveCamera());
    renderer->AddActor(letterActor);
    renderer->AddActor(lineActor);
    if (useGlyph) 
    {
        glyphActor->SetCamera(renderer->GetActiveCamera());
        renderer->AddActor(glyphActor);
    }
}


// ****************************************************************************
//  Method:  avtPickActor::Remove
//
//  Purpose:  Remove actors from the renderer, and renderer from this class.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Remove glyphActor from renderer when required.
//
// ****************************************************************************

void 
avtPickActor::Remove()
{
    if (renderer != NULL)
    {
        if (useGlyph)
            renderer->RemoveActor(glyphActor);
        renderer->RemoveActor(lineActor);
        renderer->RemoveActor(letterActor);
        renderer = NULL;
    }
}


// ****************************************************************************
//  Method:  avtPickActor::Shift
//
//  Purpose:  Shift the pickLetter towards the camera to avoid it being
//            obscured by the plot. 
//
//  Arguments:
//    vec     The vector to use for shifting.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Change glyph's position in 3d.
//
//    Kathleen Bonnell, Tue Jun  8 17:42:59 PDT 2004 
//    For 2d, use correct vec components. 
//
// ****************************************************************************

void 
avtPickActor::Shift(const float vec[3])
{
    float newPos[3], newGlyphPos[3], shiftFactor ; 
    if (mode3D)
    {
        shiftFactor = 3.;     // completely arbitrary shift factor!!!
        newPos[0] = attach[0] + vec[0] *shiftFactor;
        newPos[1] = attach[1] + vec[1] *shiftFactor;
        newPos[2] = attach[2] + vec[2] *shiftFactor;
        newGlyphPos[0] = attach[0] + vec[0] *0.25;
        newGlyphPos[1] = attach[1] + vec[1] *0.25;
        newGlyphPos[2] = attach[2] + vec[2] *0.25;
    
        glyphActor->SetPosition(newGlyphPos[0], newGlyphPos[1], newGlyphPos[2]);
    
    }
    else
    {
        shiftFactor = 10.;    // completely arbitrary shift factor!!!
        newPos[0] = attach[0] + vec[0] *shiftFactor;
        newPos[1] = attach[1] + vec[1] *shiftFactor;
        newPos[2] = attach[2];
    }
    lineSource->SetPoint2(newPos[0], newPos[1], newPos[2]);
    letterActor->SetPosition(newPos[0], newPos[1], newPos[2]);
}


// ****************************************************************************
//  Method:  avtPickActor::SetMode3D
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
avtPickActor::SetMode3D(const bool mode)
{
    mode3D = mode;
}

// ****************************************************************************
//  Method:  avtPickActor::SetAttachmentPoint
//
//  Purpose: Set actors' postion.
//
//  Arguments:
//    x, y , z   The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 25 15:12:24 PDT 2003 
//    Changed argument from float[3] to 3 floats, set position for glyphActor.
//
// ****************************************************************************

void 
avtPickActor::SetAttachmentPoint(float x, float y, float z)
{
    attach[0] = x;
    attach[1] = y;
    attach[2] = z;
    lineSource->SetPoint1(x, y, z);
    letterActor->SetPosition(x, y, z);
    glyphActor->SetPosition(x, y, z);
}


// ****************************************************************************
//  Method:  avtPickActor::SetScale
//
//  Purpose:  Set the scale for letterActor.
//
//  Arguments:
//    s       The scale factor to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Set scale for glyphActor.
//
// ****************************************************************************

void
avtPickActor::SetScale(float s)
{
    letterActor->SetScale(s);
    glyphActor->SetScale(s);
}


// ****************************************************************************
//  Method:  avtPickActor::SetDesignator
//
//  Purpose:  Sets the designator for a pick.  In this instance it
//            is text for letterActor.
//
//  Arguments:
//    l       The designator string.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
// ****************************************************************************

void avtPickActor::SetDesignator(const char *l)
{
    vtkVectorText *vecText = vtkVectorText::New();
    vecText->SetText(l);

    vtkPolyDataMapper *pickMapper = vtkPolyDataMapper::New();
    pickMapper->SetInput(vecText->GetOutput());

    letterActor->SetMapper(pickMapper);

    pickMapper->Delete();
    vecText->Delete();
}


// ****************************************************************************
//  Method:  avtPickActor::SetForegroundColor
//
//  Purpose:  Set the actors' color.
//
//  Arguments:
//    fg         The rgb components of the foreground color.
//   
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Set color for glyphActor.
//
// ****************************************************************************

void avtPickActor::SetForegroundColor(float fg[3])
{
    letterActor->GetProperty()->SetColor(fg);
    lineActor->GetProperty()->SetColor(fg);
    glyphActor->GetProperty()->SetColor(fg);
}

// ****************************************************************************
//  Method:  avtPickActor::SetForegroundColor
//
//  Purpose: Set the actors' color.
//
//  Arguments:
//    r      The red component of the foreground color.
//    g      The green component of the foreground color.
//    b      The blue component of the foreground color.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Set color for glyphActor.
//
// ****************************************************************************

void 
avtPickActor::SetForegroundColor(float r, float g, float b)
{
    letterActor->GetProperty()->SetColor(r, g, b);
    lineActor->GetProperty()->SetColor(r, g, b);
    glyphActor->GetProperty()->SetColor(r, g, b);
}


// ****************************************************************************
//  Method:  avtPickActor::Hide
//
//  Purpose: Make the actors invisible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Hide glyphActor.
//
// ****************************************************************************

void 
avtPickActor::Hide()
{
    letterActor->VisibilityOff();
    lineActor->VisibilityOff();
    glyphActor->VisibilityOff();
}


// ****************************************************************************
//  Method:  avtPickActor::UnHide
//
//  Purpose:  Make the actors visible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Unhide glyphActor.
//
// ****************************************************************************

void 
avtPickActor::UnHide()
{
    letterActor->VisibilityOn();
    lineActor->VisibilityOn();
    glyphActor->VisibilityOn();
}


// ****************************************************************************
//  Method:  avtPickActor::UpdateView
//
//  Purpose:   Peform whatever updates are necessary when the view changes.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed May  8 13:50:44 PDT 2002
//    Removed call to ComputeScaleFactor.  Now handled by VisWinQuery.
//
// ****************************************************************************

void 
avtPickActor::UpdateView()
{
    // nothing for now. 
}


// ****************************************************************************
//  Method:  avtPickActor::Translate
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
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Translate the glyphActor.
//
//    Kathleen Bonnell, Fri Feb 20 12:34:39 PST 2004 
//    Handle 3D lineSource's Pt1 differently than 2D. 
//
//    Kathleen Bonnell, Fri Feb 20 12:34:39 PST 2004 
//    Handle 2D glyphActor's position differently than 3D. 
//
// ****************************************************************************

void 
avtPickActor::Translate(const float vec[3])
{
    float *newPos = letterActor->GetPosition();
    newPos[0] *= vec[0];
    newPos[1] *= vec[1]; 
    newPos[2] *= vec[2];

    newPos = lineSource->GetPoint1();
    if (mode3D)
    {
        newPos[0] *= vec[0];
        newPos[1] *= vec[1]; 
        newPos[2] *= vec[2];
    }
    else
    {
        newPos[0] = attach[0] * vec[0];
        newPos[1] = attach[1] * vec[1]; 
        newPos[2] = attach[2] * vec[2];
    }

    newPos = lineSource->GetPoint2();
    newPos[0] *= vec[0];
    newPos[1] *= vec[1]; 
    newPos[2] *= vec[2];

    newPos = glyphActor->GetPosition();
    if (mode3D)
    {
        newPos[0] *= vec[0];
        newPos[1] *= vec[1]; 
        newPos[2] *= vec[2];
    }
    else
    {
        newPos[0] = attach[0] * vec[0];
        newPos[1] = attach[1] * vec[1]; 
        newPos[2] = attach[2] * vec[2];
    }

    lineSource->Modified();
}


// ****************************************************************************
//  Method:  avtPickActor::ResetPosition
//
//  Purpose:   Resets the actor's position to the original location, including
//             Shifting towards the camera. 
//
//  Arguments:
//    vec      The vector to use in Shifting the actor. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 6, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 16:57:45 PDT 2003 
//    Reset glyphActor's position.
//
// ****************************************************************************

void 
avtPickActor::ResetPosition(const float vec[3])
{
    lineSource->SetPoint1(attach[0], attach[1], attach[2]);
    letterActor->SetPosition(attach[0], attach[1], attach[2]);
    glyphActor->SetPosition(attach[0], attach[1], attach[2]);
    Shift(vec);
}

// ****************************************************************************
//  Method:  avtPickActor::GetLetterPosition
//
//  Purpose:   Retrieve the position of the letterActor. 
//             Useful for determining scale factor.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 8, 2004
//
//  Modifications:
//
// ****************************************************************************

const float * 
avtPickActor::GetLetterPosition()
{
    return letterActor->GetPosition();
}
