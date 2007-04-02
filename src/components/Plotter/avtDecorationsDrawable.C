// ************************************************************************* //
//                        avtDecorationsDrawable.C                           //
// ************************************************************************* //

#include <avtDecorationsDrawable.h>

#include <vtkDataObjectCollection.h>
#include <vtkRenderer.h>

#include <avtDecorationsMapper.h>
#include <avtTransparencyActor.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtDecorationsDrawable constructor
//
//  Arguments:
//      alist    A list of actors.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    July 12, 2002
// 
// ****************************************************************************

avtDecorationsDrawable::avtDecorationsDrawable(std::vector<avtLabelActor_p> & alist)
{
    actors  = alist;
    renderer = NULL;
    mapper = NULL;
}

// ****************************************************************************
//  Method: avtDecorationsDrawable destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

avtDecorationsDrawable::~avtDecorationsDrawable()
{
    //
    // If we are still attached to a renderer, remove ourselves.
    //
    if (renderer != NULL)
    {
        Remove(renderer);
    }

    if (!actors.empty())
    {
        actors.clear();
    }
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::SetMapper
//
//  Purpose:
//      Tells the decorations drawable which mapper it comes from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::SetMapper(avtDecorationsMapper *m)
{
    mapper = m;
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::Add
//
//  Purpose:
//      Adds this drawable to a renderer.
//
//  Arguments:
//      ren     The renderer to add ourselves to.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsDrawable::Add(vtkRenderer *ren)
{
    renderer = ren;
    for (int i = 0 ; i < actors.size() ; i++)
    {
        actors[i]->Add(ren); 
    }
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::Remove
//
//  Purpose:
//      Removes this drawable from a renderer.
//
//  Arguments:
//      ren     The renderer to remove ourselves from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsDrawable::Remove(vtkRenderer *ren)
{
    if (ren != renderer)
    {
        EXCEPTION0(ImproperUseException);
    }

    for (int i = 0 ; i < actors.size() ; i++)
    {
        actors[i]->Remove();
    }
    renderer = NULL;
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::VisibilityOn
//
//  Purpose:
//      Turns the visibility of the drawable back on (this is the default).
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::VisibilityOn(void)
{
    for (int i = 0 ; i < actors.size() ; i++)
    {
        actors[i]->UnHide();
    }
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::VisibilityOff
//
//  Purpose:
//      Turns off the visibility of the drawable (for bounding box mode, etc).
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002
//
// ****************************************************************************

void
avtDecorationsDrawable::VisibilityOff(void)
{
    for (int i = 0 ; i < actors.size() ; i++)
    {
        actors[i]->Hide();
    }
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::ShiftByVector
//
//  Purpose:
//      Shifts the drawable by a vector. 
//
//  Arguments:
//      vec    The vector to shift by.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::ShiftByVector(const double vec[3])
{
    // nothing here for now
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::ScaleByVector
//
//  Purpose:
//      Scales the drawable by a vector. 
//
//  Arguments:
//      vec    The vector to scale by.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::ScaleByVector(const double vec[3])
{
    //
    // Okay, this is weird, we're told to "scale" but we're telling
    // the actors to shift.  This class was created with curve plots
    // in mind.  When the curve line gets scaled, we don't want the
    // label decorations to get scaled, too.  Rather we want to
    // change their position according to the way the curve line 
    // was scaled.
    //
    for (int i = 0 ; i < actors.size(); i++)
    {
        actors[i]->Shift(vec);
    }
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::GetDataObject
//
//  Purpose:
//      Gets the datasets associated with this decorations drawable.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2002 
//
// ****************************************************************************

avtDataObject_p
avtDecorationsDrawable::GetDataObject(void)
{
    // not certain it makes sense to get the decoration's input, which is
    // the same as some plot's input, so always return NULL for now.
    return NULL;
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::UpdateScaleFactor
//
//  Purpose:
//      Allows the drawable to re-compute its scale. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 19, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::UpdateScaleFactor()
{
    if (actors.size() > 0) 
    {
        //
        //  As this class is currently only used to set labels on curve
        //  plots, which exist in 2D, the scale factor for 1 actor should
        //  suffice for all.  Compute once, then use that result to set
        //  the scale for all the other actors.
        //
        double scale = actors[0]->ComputeScaleFactor();
        for (int i = 1 ; i < actors.size(); i++)
        {
            actors[i]->SetScale(scale);
        }
    }
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::TurnLightingOn
//
//  Purpose:
//      Allows the drawable to update its lighting coefficients.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::TurnLightingOn()
{
   ; // Nothing for now.
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::TurnLightingOff
//
//  Purpose:
//      Allows the drawable to update its lighting coefficients.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::TurnLightingOff()
{
   ; // Nothing for now.
}


// ****************************************************************************
//  Method: avtDecorationsDrawable::SetAmbientCoefficient
//
//  Purpose:
//      Allows the drawable to update its lighting coefficients.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtDecorationsDrawable::SetAmbientCoefficient(const double)
{
   ; // Nothing for now.
}


