#include <avtLabelActor.h>

#include <vtkFollower.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>


// ****************************************************************************
//  Constructor:  avtLabelActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

avtLabelActor:: avtLabelActor()
{
    attach[0] = attach[1] = attach[2] = 0.;

    labelActor = vtkFollower::New(); 
        labelActor->GetProperty()->SetColor(0., 0., 0.);
        labelActor->SetScale(0.5);
        labelActor->PickableOff();

    renderer = NULL; 
}


// ****************************************************************************
//  Destructor:  avtLabelActor
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

avtLabelActor::~avtLabelActor()
{
    Remove();
    if (labelActor != NULL)
    {
        labelActor->Delete();
        labelActor = NULL;
    }
}


// ****************************************************************************
//  Method:  avtLabelActor::Add
//
//  Purpose:  Add actors to the renderer, and renderer to this class.
//
//  Arguments:
//    ren     The renderer to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::Add(vtkRenderer *ren)
{
    renderer = ren;
    labelActor->SetCamera(renderer->GetActiveCamera());
    renderer->AddActor(labelActor);
}


// ****************************************************************************
//  Method:  avtLabelActor::Remove
//
//  Purpose:  Remove actors from the renderer, and renderer from this class.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::Remove()
{
    if (renderer != NULL)
    {
        renderer->RemoveActor(labelActor);
        renderer = NULL;
    }
}


// ****************************************************************************
//  Method:  avtLabelActor::Shift
//
//  Purpose:  Shift the actors by the specified vector.
//
//  Arguments:
//    vec     The vector to use for shifting.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::Shift(const float vec[3])
{
    float newPos[3];
    newPos[0] = attach[0] * vec[0];
    newPos[1] = attach[1] * vec[1];
    newPos[2] = attach[2] * vec[2] ;
    labelActor->SetPosition(newPos[0], newPos[1], newPos[2]);
}

// ****************************************************************************
//  Method:  avtLabelActor::SetAttachmentPoint
//
//  Purpose: Set actors' postion.
//
//  Arguments:
//    pos    The attachment point in 3d world coordinates.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void 
avtLabelActor::SetAttachmentPoint(const float pos[3])
{
    attach[0] = pos[0];
    attach[1] = pos[1];
    attach[2] = pos[2];
    labelActor->SetPosition(pos[0], pos[1], pos[2]);
}


// ****************************************************************************
//  Method:  avtLabelActor::SetScale
//
//  Purpose:  Set the scale for labelActor.
//
//  Arguments:
//    s       The scale factor to use.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void
avtLabelActor::SetScale(float s)
{
    labelActor->SetScale(s);
}


// ****************************************************************************
//  Method:  avtLabelActor::SetDesignator
//
//  Purpose:  Sets the designator text for this actor. 
//
//  Arguments:
//    l       The designator string.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::SetDesignator(const char *l)
{
    vtkVectorText *vecText = vtkVectorText::New();
    vecText->SetText(l);

    vtkPolyDataMapper *labelMapper = vtkPolyDataMapper::New();
    labelMapper->SetInput(vecText->GetOutput());

    labelActor->SetMapper(labelMapper);

    labelMapper->Delete();
    vecText->Delete();
}


// ****************************************************************************
//  Method:  avtLabelActor::SetForegroundColor
//
//  Purpose:  Set the actors' color.
//
//  Arguments:
//    fg         The rgb components of the foreground color.
//   
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::SetForegroundColor(float fg[3])
{
    labelActor->GetProperty()->SetColor(fg);
}

// ****************************************************************************
//  Method:  avtLabelActor::SetForegroundColor
//
//  Purpose: Set the actors' color.
//
//  Arguments:
//    r      The red component of the foreground color.
//    g      The green component of the foreground color.
//    b      The blue component of the foreground color.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::SetForegroundColor(float r, float g, float b)
{
    labelActor->GetProperty()->SetColor(r, g, b);
}


// ****************************************************************************
//  Method:  avtLabelActor::Hide
//
//  Purpose: Make the actors invisible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::Hide()
{
    labelActor->VisibilityOff();
}


// ****************************************************************************
//  Method:  avtLabelActor::UnHide
//
//  Purpose:  Make the actors visible.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
// ****************************************************************************

void avtLabelActor::UnHide()
{
    labelActor->VisibilityOn();
}


// ****************************************************************************
//  Method:  avtLabelActor::ComputeScaleFactor
//
//  Purpose:  Computes the scale for this actor to use, based on the
//            current settings in the renderer, and the actors position. 
//
//  Notes:    This code was duplicated from /components/VisWindow/Colleagues/
//            VisWinRendering.  If this code is modified in any significant way
//            (change in heuristics, etc.) the code in VisWinRendering should 
//            also be modified.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 19, 2002 
//
// ****************************************************************************

float 
avtLabelActor::ComputeScaleFactor()
{
    if (renderer == NULL)
    {
        return -1.;
    }
    float pos[3];
    labelActor->GetPosition(pos);
 
    float newX = pos[0], newY = pos[1], newZ = pos[2];
    renderer->WorldToView(newX, newY, newZ);
    renderer->ViewToNormalizedViewport(newX, newY, newZ);
    renderer->NormalizedViewportToViewport(newX, newY);
    renderer->ViewportToNormalizedDisplay(newX, newY);

    //
    //  Assuming NormalizedDisplay coordinates run from 0 .. 1 in both
    //  x and y directions, then the normalized dispaly creates a square, whose
    //  diagonal length is sqrt(2) or 1.4142134624.  
    // 
    const float displayDiag = 1.4142135624; 

    // 
    //  We want to find a position P2, that creates a diagonal with the 
    //  original point that is 1/20th of the display diagonal.
    //

    const float target = displayDiag * 0.0125;

    //
    //  Since we are dealing with a right-isosceles-triangle the new position
    //  will be offset in both x and y directions by target * cos(45); 
    //
  
    const float cos45 = 0.7604059656;

    float offset = target * cos45;

    newX += offset;
    newY += offset;

    //
    // Now convert our new position from NormalizedDisplay to World Coordinates.
    //
 
    renderer->NormalizedDisplayToViewport(newX, newY);
    renderer->ViewportToNormalizedViewport(newX, newY);
    renderer->NormalizedViewportToView(newX, newY, newZ);
    renderer->ViewToWorld(newX, newY, newZ);

    //
    // Experimental results, using vtkVectorText and vtkPolyDataMapper, 
    // smallest 'diagonal size' is from the letter 'r' at 0.917883
    // largest  'diagonal size' is from the letter 'W' at 1.78107
    // thus, the average diagonal size of vtkVectorText is:  1.3494765
    // (for alpha text only, in world coordinats, with scale factor of 1.0).  
    // THIS MAY BE A DISPLAY-DEPENDENT RESULT! 
    // 

    const float avgTextDiag = 1.3494765;

    //
    //  Calculate our scale factor, by determining the new target and using
    //  the avgTextDiag to normalize the results across all the pick letters.
    //

    float dxsqr = (newX - pos[0]) * (newX - pos[0]);
    float dysqr = (newY - pos[1]) * (newY - pos[1]);
    float dzsqr = (newZ - pos[2]) * (newZ - pos[2]);
    float worldTarget = sqrt(dxsqr + dysqr + dzsqr); 

    float scale = worldTarget / avgTextDiag;
    SetScale(scale);
    return scale;
}


