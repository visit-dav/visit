#include <avtPickActor.h>

#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>

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
// ****************************************************************************

avtPickActor:: avtPickActor()
{
    mode3D  = true;
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

    renderer = NULL; 
}


// ****************************************************************************
//  Destructor:  avtPickActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
// ****************************************************************************

avtPickActor::~avtPickActor()
{
    Remove();
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
// ****************************************************************************

void 
avtPickActor::Add(vtkRenderer *ren)
{
    renderer = ren;
    letterActor->SetCamera(renderer->GetActiveCamera());
    renderer->AddActor(letterActor);
    renderer->AddActor(lineActor);
}


// ****************************************************************************
//  Method:  avtPickActor::Remove
//
//  Purpose:  Remove actors from the renderer, and renderer from this class.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
// ****************************************************************************

void 
avtPickActor::Remove()
{
    if (renderer != NULL)
    {
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
// ****************************************************************************

void 
avtPickActor::Shift(const float vec[3])
{
    float newPos[3], shiftFactor ; 
    if (mode3D)
    {
        shiftFactor = 3.;     // completely arbitrary shift factor!!!
        newPos[0] = attach[0] + vec[0] *shiftFactor;
        newPos[1] = attach[1] + vec[1] *shiftFactor;
        newPos[2] = attach[2] + vec[2] *shiftFactor;
    }
    else
    {
        shiftFactor = 10.;    // completely arbitrary shift factor!!!
        newPos[0] = attach[0] + vec[2] *shiftFactor;
        newPos[1] = attach[1] + vec[2] *shiftFactor;
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
//    pos    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
// ****************************************************************************

void 
avtPickActor::SetAttachmentPoint(const float pos[3])
{
    attach[0] = pos[0];
    attach[1] = pos[1];
    attach[2] = pos[2];
    lineSource->SetPoint1(pos[0], pos[1], pos[2]);
    letterActor->SetPosition(pos[0], pos[1], pos[2]);
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
// ****************************************************************************

void
avtPickActor::SetScale(float s)
{
    letterActor->SetScale(s);
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
// ****************************************************************************

void avtPickActor::SetForegroundColor(float fg[3])
{
    letterActor->GetProperty()->SetColor(fg);
    lineActor->GetProperty()->SetColor(fg);
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
// ****************************************************************************

void 
avtPickActor::SetForegroundColor(float r, float g, float b)
{
    letterActor->GetProperty()->SetColor(r, g, b);
    lineActor->GetProperty()->SetColor(r, g, b);
}


// ****************************************************************************
//  Method:  avtPickActor::Hide
//
//  Purpose: Make the actors invisible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
// ****************************************************************************

void 
avtPickActor::Hide()
{
    letterActor->VisibilityOff();
    lineActor->VisibilityOff();
}


// ****************************************************************************
//  Method:  avtPickActor::UnHide
//
//  Purpose:  Make the actors visible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 22, 2002 
//
// ****************************************************************************

void 
avtPickActor::UnHide()
{
    letterActor->VisibilityOn();
    lineActor->VisibilityOn();
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
//
// ****************************************************************************

void 
avtPickActor::ResetPosition(const float vec[3])
{
    lineSource->SetPoint1(attach[0], attach[1], attach[2]);
    letterActor->SetPosition(attach[0], attach[1], attach[2]);
    Shift(vec);
}
