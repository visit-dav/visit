// ************************************************************************* //
//                                  avtActor.C                               //
// ************************************************************************* //

#include <avtActor.h>

#include <vtkRenderer.h>

#include <avtTransparencyActor.h>

#include <NoInputException.h>


// ****************************************************************************
//  Method: avtActor constructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Initialized new data member 'transparencyActor'.
//
//    Kathleen Bonnell, Fri Jul 12 16:10:49 PDT 2002 
//    Add support for a decorations drawable.
//
//    Mark C. Miller, Thu Dec 19 16:19:23 PST 2002
//    Added new data members, extRenderedImagesActor and extRenderedImageId
//    Added initialization for transparencyIndex;
// ****************************************************************************

avtActor::avtActor()
{
    behavior               = NULL;
    drawable               = NULL;
    decorations            = NULL;
    renderer               = NULL;
    transparencyActor      = NULL;
    transparencyIndex      = -1;
    extRenderedImagesActor = NULL;
}


// ****************************************************************************
//  Method: avtActor destructor
//
//  Programmer: Hank Childs
//  Creation:   June 25, 2003
//
// ****************************************************************************

avtActor::~avtActor()
{
    if (transparencyActor != NULL && transparencyIndex >= 0)
    {
        transparencyActor->RemoveInput(transparencyIndex);
    }
}


// ****************************************************************************
//  Method: avtActor::SetBehavior
//
//  Purpose:
//      Sets the behavior of the actor.
//
//  Arguments:
//      b      The behavior for this actor.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

void
avtActor::SetBehavior(avtBehavior_p b)
{
    behavior = b;
}


// ****************************************************************************
//  Method: avtActor::SetDrawable
//
//  Purpose:
//      Sets the drawable of the actor.
//
//  Arguments:
//      d      The drawable for this actor.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

void
avtActor::SetDrawable(avtDrawable_p d)
{
    drawable = d;
    if (renderer != NULL)
    {
        drawable->Add(renderer);
    }
}


// ****************************************************************************
//  Method: avtActor::SetDecorations
//
//  Purpose:
//      Sets the deocorations of the actor.
//
//  Arguments:
//      d      The decorations for this actor.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtActor::SetDecorations(avtDrawable_p d)
{
    decorations = d;
    if (*(decorations) != NULL && renderer != NULL)
    {
        decorations->Add(renderer);
    }
}



// ****************************************************************************
//  Method: avtActor::Add
//
//  Purpose:
//      Adds the actor to a renderer.
//
//  Arguments:
//      renderingCanvas   The canvas that the rendering takes place on.  This
//                        is for the drawable.
//      decorationCanvas  The canvas that decorations should be placed on.
//                        This is for the behavior.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  8 10:58:27 PST 2001
//    Added second renderer so 2D plots could work well.
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Kathleen Bonnell, Fri Jul 12 16:10:49 PDT 2002 
//    Add support for a decorations drawable.
//
// ****************************************************************************

void
avtActor::Add(vtkRenderer *renderingCanvas, vtkRenderer *decorationCanvas)
{
    if (*behavior == NULL || *drawable == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    behavior->Add(decorationCanvas);
    drawable->Add(renderingCanvas);
    if (*(decorations) != NULL)
    {
        decorations->Add(renderingCanvas);
    }

    renderer = renderingCanvas;

    if (transparencyActor != NULL && transparencyIndex >=0)
    {
        transparencyActor->TurnOnInput(transparencyIndex);
    }
}


// ****************************************************************************
//  Method: avtActor::Remove
//
//  Purpose:
//      Removes the actor from a renderer.
//
//  Arguments:
//      renderingCanvas   The canvas that the rendering takes place on.  This
//                        is for the drawable.
//      decorationCanvas  The canvas that decorations should be placed on.
//                        This is for the behavior.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  8 10:58:27 PST 2001
//    Added second renderer so 2D plots could work well.
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Kathleen Bonnell, Fri Jul 12 16:10:49 PDT 2002 
//    Add support for a decorations drawable.
//
// ****************************************************************************

void
avtActor::Remove(vtkRenderer *renderingCanvas, vtkRenderer *decorationCanvas)
{
    if (*behavior == NULL || *drawable == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    behavior->Remove(decorationCanvas);
    drawable->Remove(renderingCanvas);
    if (*(decorations) != NULL)
    {
        decorations->Remove(renderingCanvas);
    }
    renderer = NULL;

    if (transparencyActor != NULL && transparencyIndex >= 0)
    {
        transparencyActor->TurnOffInput(transparencyIndex);
    }
}


// ****************************************************************************
//  Method: avtActor::GetOriginalBounds
//
//  Purpose:
//      Gets the original bounds of an actor.
//
//  Arguments:
//      bounds    A place to put the bounds.
//
//  Programmer:   Hank Childs
//  Creation:     December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jul 15 09:07:25 PDT 2002
//    Renamed to GetOriginalBounds (from GetBounds).
//
// ****************************************************************************

void
avtActor::GetOriginalBounds(float bounds[6])
{
    if (*behavior == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    behavior->GetOriginalBounds(bounds);
}


// ****************************************************************************
//  Method: avtActor::GetActualBounds
//
//  Purpose:
//      Gets the actual bounds of an actor.
//
//  Arguments:
//      bounds    A place to put the bounds.
//
//  Programmer:   Hank Childs
//  Creation:     July 15, 2002
//
// ****************************************************************************

void
avtActor::GetActualBounds(float bounds[6])
{
    if (*behavior == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    behavior->GetActualBounds(bounds);
}


// ****************************************************************************
//  Method: avtActor::GetDimension
//
//  Purpose:
//      Gets the dimension of the plot.
//
//  Returns:    the spatial dimension of the plot
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

int
avtActor::GetDimension(void)
{
    if (*behavior == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return behavior->GetDimension();
}


// ****************************************************************************
//  Method: avtActor::GetRenderOrder
//
//  Purpose:
//      Gets the render order of the plot.
//
//  Returns:    the render order of the plot
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 3, 2001 
//
// ****************************************************************************

int
avtActor::GetRenderOrder(void)
{
    if (*behavior == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return behavior->GetRenderOrder();
}


// ****************************************************************************
//  Method: avtActor::GetLegend
//
//  Purpose:
//      Gets the legend from the behavior.
//
//  Returns:     A legend, NULL is possible if there is no legend.
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2000
//
// ****************************************************************************

avtLegend_p
avtActor::GetLegend(void)
{
    if (*behavior == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return behavior->GetLegend();
}


// ****************************************************************************
//  Method: avtActor::VisibilityOff
//
//  Purpose:
//      Has a drawable remove itself from the scene for something along the
//      lines of bounding box mode.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Hank Childs, Thu Jul 11 16:07:53 PDT 2002
//    Use the concept of 'visibility' for the transparency actor, rather
//    than turning it on and off.
//
// ****************************************************************************

void
avtActor::VisibilityOff(void)
{
    if (*drawable == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    drawable->VisibilityOff();

    if (transparencyActor != NULL && transparencyIndex >= 0)
    {
        transparencyActor->SetVisibility(transparencyIndex, false);
    }
}


// ****************************************************************************
//  Method: avtActor::VisibilityOn
//
//  Purpose:
//      Has a drawable add itself back to the scene after something along the
//      lines of bounding box mode.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Add support for transparency.
//
//    Hank Childs, Thu Jul 11 16:07:53 PDT 2002
//    Use the concept of 'visibility' for the transparency actor, rather
//    than turning it on and off.
//
// ****************************************************************************

void
avtActor::VisibilityOn(void)
{
    if (*drawable == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    drawable->VisibilityOn();

    if (transparencyActor != NULL && transparencyIndex >= 0)
    {
        transparencyActor->SetVisibility(transparencyIndex, true);
    }
}


// ****************************************************************************
//  Method: avtActor::ShiftByVector
//
//  Purpose:
//      Shifts the actor by a vector.
//
//  Arguments:
//      vec     The vector to shift by.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
// ****************************************************************************

void
avtActor::ShiftByVector(const float vec[3])
{
    //
    // Not all plots should be shifted by the same amount.  The behavior has
    // a factor (presumably between 0 and 1) that will determine by what
    // amount to shift.
    //
    float f = behavior->GetShiftFactor();

    float v[3];
    v[0] = vec[0]*f;
    v[1] = vec[1]*f;
    v[2] = vec[2]*f;

    drawable->ShiftByVector(v);
}


// ****************************************************************************
//  Method: avtActor::ScaleByVector
//
//  Purpose:
//      Scale the actor component-wise by a vector.
//
//  Arguments:
//      vec     The vector to scale by.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 16 16:40:47 PDT 2003
//    Scale the transparency actor.
//
// ****************************************************************************

void
avtActor::ScaleByVector(const float vec[3])
{
    drawable->ScaleByVector(vec);
    if (*decorations != NULL)
    {
        decorations->ScaleByVector(vec);
    }
    if (transparencyActor != NULL && transparencyIndex >= 0)
    {
        transparencyActor->ScaleByVector(vec);
    }
}



// ****************************************************************************
//  Method: avtActor::GetDataExtents
//
//  Purpose:
//      Gets the data extents of an actor.
//
//  Arguments:
//      dmin    A place to put minimum value. 
//      dmax    A place to put maximum value. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     May 7, 2002 
//
// ****************************************************************************

void
avtActor::GetDataExtents(float &dmin, float &dmax) 
{
    if (*behavior == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    behavior->GetDataExtents(dmin, dmax);
}


// ****************************************************************************
//  Method: avtActor::GetDataObject
//
//  Purpose:
//      Gets the data objects added to this actor.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Sun May 26 18:37:23 PDT 2002
//    Changed interface to return an avtDataObject directly.  This is a more
//    general interface and allows us to not modify the tree structure for an
//    avtDataset.
//
// ****************************************************************************

avtDataObject_p
avtActor::GetDataObject(void)
{
    return drawable->GetDataObject();
}


// ****************************************************************************
//  Method: avtActor::SetTransparencyActor
//
//  Purpose:
//      Sets the transparency actor that this actor must notify of changes in
//      status.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2002
//
// ****************************************************************************

void
avtActor::SetTransparencyActor(avtTransparencyActor *actor)
{
    transparencyActor = actor;
    transparencyIndex = drawable->SetTransparencyActor(transparencyActor);
}


// ****************************************************************************
//  Method: avtActor::SetExternallyRenderedImagesActor
//
//  Purpose:
//      Sets the ERI actor to which this actor must defer rendering requests
//
//  Programmer: Mark C. Miller
//  Creation:   January 9, 2003 
//
// ****************************************************************************

void
avtActor::SetExternallyRenderedImagesActor(
   avtExternallyRenderedImagesActor *actor)
{
    extRenderedImagesActor = actor;
    drawable->SetExternallyRenderedImagesActor(actor);
}


// ****************************************************************************
//  Method: avtActor::UpdateScaleFactor
//
//  Purpose:
//      Tells the decorations to update the scale factor. 
//
//  Programmer: Kathleen Bonenll 
//  Creation:   July 19, 2002
//
// ****************************************************************************

void
avtActor::UpdateScaleFactor()
{
    if (*(decorations) != NULL)
    {
        decorations->UpdateScaleFactor();
    }
}


// ****************************************************************************
//  Method: avtActor::TurnLightingOn
//
//  Purpose:
//      Tells the drawable to update the lighting coefficients. 
//
//  Programmer: Kathleen Bonenll 
//  Creation:   August 13, 2002
//
// ****************************************************************************

void
avtActor::TurnLightingOn()
{
    if (*(drawable) != NULL)
    {
        drawable->TurnLightingOn();
    }
}


// ****************************************************************************
//  Method: avtActor::TurnLightingOff
//
//  Purpose:
//      Tells the drawable to update the lighting coefficients. 
//
//  Programmer: Kathleen Bonenll 
//  Creation:   August 13, 2002
//
// ****************************************************************************

void
avtActor::TurnLightingOff()
{
    if (*(drawable) != NULL)
    {
        drawable->TurnLightingOff();
    }
}


// ****************************************************************************
//  Method: avtActor::SetAmbientCoefficient
//
//  Purpose:
//      Tells the drawable to update the ambient lighting coefficient. 
//
//  Programmer: Kathleen Bonenll 
//  Creation:   August 13, 2002
//
// ****************************************************************************

void
avtActor::SetAmbientCoefficient(const float amb)
{
    if (*(drawable) != NULL)
    {
        drawable->SetAmbientCoefficient(amb);
    }
}

// ****************************************************************************
//  Method: avtActor::SetSurfaceRepresentation
//
//  Purpose:
//      Tells the drawable to set its surface representation. 
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 23 15:54:15 PST 2002
//
// ****************************************************************************

void
avtActor::SetSurfaceRepresentation(int rep)
{
    if (*(drawable) != NULL)
    {
        drawable->SetSurfaceRepresentation(rep);
    }
}

// ****************************************************************************
//  Method: avtActor::SetImmediateModeRendering
//
//  Purpose:
//      Tells the drawable adn the decorations to set their immediate mode
//      rendering flags. 
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 23 15:54:15 PST 2002
//
// ****************************************************************************

void
avtActor::SetImmediateModeRendering(bool val)
{
    if (*(drawable) != NULL)
    {
        drawable->SetImmediateModeRendering(val);
    }

    if (*(decorations) != NULL)
    {
        decorations->SetImmediateModeRendering(val);
    }
}
