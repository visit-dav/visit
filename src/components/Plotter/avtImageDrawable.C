// ************************************************************************* //
//                            avtImageDrawable.C                             //
// ************************************************************************* //

#include <avtImageDrawable.h>

#include <vtkActor2D.h>
#include <vtkRenderer.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtImageDrawable constructor
//
//  Arguments:
//      img      The actor for this image.
//
//  Programmer:  Hank Childs
//  Creation:    December 27, 2000
//
// ****************************************************************************

avtImageDrawable::avtImageDrawable(vtkActor2D *img)
{
    image    = img;
    image->Register(NULL);
    renderer = NULL;
}


// ****************************************************************************
//  Method: avtImageDrawable destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

avtImageDrawable::~avtImageDrawable()
{
    if (renderer != NULL)
    {
        Remove(renderer);
    }
    if (image != NULL)
    {
        image->Delete();
        image = NULL;
    }
}


// ****************************************************************************
//  Method: avtImageDrawable::Add
//
//  Purpose:
//      Adds this drawable to a renderer.
//
//  Arguments:
//      ren     The renderer to add ourselves to.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
// ****************************************************************************

void
avtImageDrawable::Add(vtkRenderer *ren)
{
    renderer = ren;
    renderer->AddActor(image);
}


// ****************************************************************************
//  Method: avtImageDrawable::Remove
//
//  Purpose:
//      Removes this drawable from a renderer.
//
//  Arguments:
//      ren     The renderer to remove ourselves from.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
// ****************************************************************************

void
avtImageDrawable::Remove(vtkRenderer *ren)
{
    if (ren != renderer)
    {
        EXCEPTION0(ImproperUseException);
    }

    renderer->RemoveActor(image);
    renderer = NULL;
}


// ****************************************************************************
//  Method: avtImageDrawable::VisibilityOn
//
//  Purpose:
//      Turns the visibility of the drawable back on (this is the default).
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

void
avtImageDrawable::VisibilityOn(void)
{
    image->VisibilityOn();
}


// ****************************************************************************
//  Method: avtImageDrawable::VisibilityOff
//
//  Purpose:
//      Turns off the visibility of the drawable (for bounding box mode, etc).
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

void
avtImageDrawable::VisibilityOff(void)
{
    image->VisibilityOff();
}


// ****************************************************************************
//  Method: avtImageDrawable::ShiftByVector
//
//  Purpose:
//      Shifts the drawable by a vector.  This doesn't make sense for an image,
//      so do nothing.  This is defined only so this can be a concrete type.
//
//  Arguments:
//      <unnamed>    The vector to shift by.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
// ****************************************************************************

void
avtImageDrawable::ShiftByVector(const double [3])
{
    ;
}


// ****************************************************************************
//  Method: avtImageDrawable::ScaleByVector
//
//  Purpose:
//      Scales the drawable by a vector.  This doesn't make sense for an image,
//      so do nothing.  This is defined only so this can be a concrete type.
//
//  Arguments:
//      <unnamed>    The vector to scale by.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtImageDrawable::ScaleByVector(const double [3])
{
    ;
}


// ****************************************************************************
//  Method: avtImageDrawable::GetDataObject
//
//  Purpose:
//      Get the associated data object.  This is not implemented at this time.
//
//  Returns:    The image as a data object.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Sun May 26 18:37:23 PDT 2002
//    Changed interface to return an avtDataObject directly.  This is a more
//    general interface and allows us to play well with images (although this
//    functionality is not being used at the time of this change).
//
// ****************************************************************************

avtDataObject_p
avtImageDrawable::GetDataObject(void)
{
    return 0;
}


// ****************************************************************************
//  Method: avtImageDrawable::UpdateScaleFactor
//
//  Purpose:
//      Allows the drawable to recompute its scale.  This doesn't make sense 
//      for an image so do nothing.  This is defined only so this can be a 
//      concrete type.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtImageDrawable::UpdateScaleFactor()
{
    ;
}


// ****************************************************************************
//  Method: avtImageDrawable::TurnLightingOn
//
//  Purpose:
//      Allows the drawable to update its lighting coefficients.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtImageDrawable::TurnLightingOn()
{
    ;  // nothing for now
}


// ****************************************************************************
//  Method: avtImageDrawable::TurnLightingOff
//
//  Purpose:
//      Allows the drawable to update its lighting coefficients.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002
//
// ****************************************************************************

void
avtImageDrawable::TurnLightingOff()
{
    ;  // nothing for now
}


// ****************************************************************************
//  Method: avtImageDrawable::SetAmbientCoefficient
//
//  Purpose:
//      Allows the drawable to update its lighting coefficients.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtImageDrawable::SetAmbientCoefficient(const double)
{
    ;  // nothing for now
}


