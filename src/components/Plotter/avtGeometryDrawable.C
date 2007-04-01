// ************************************************************************* //
//                           avtGeometryDrawable.C                           //
// ************************************************************************* //

#include <avtGeometryDrawable.h>

#include <vtkActor.h>
#include <vtkDataObjectCollection.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include <avtMapper.h>
#include <avtTransparencyActor.h>

#include <ColorAttribute.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtGeometryDrawable constructor
//
//  Arguments:
//      nA       The number of actors in a.
//      a        A list of actors.
//
//  Programmer:  Hank Childs
//  Creation:    December 21, 2000
// 
//  Modifications:
//    Kathleen Bonnell, Thu Mar 15 10:51:59 PST 2001
//    Added member actorsVisibility, used to store visibility of passed
//    actors, so that this visibility value can be restored to individual
//    actors in the VisbilityOn method. 
//
// ****************************************************************************

avtGeometryDrawable::avtGeometryDrawable(int nA, vtkActor **a)
{
    nActors = nA;
    actors  = new vtkActor*[nActors];
    actorsVisibility  = new int[nActors];
  
    for (int i = 0 ; i < nActors ; i++)
    {
        actors[i] = a[i];
        if (actors[i] != NULL)
        {
            actors[i]->Register(NULL);
            actorsVisibility[i] = actors[i]->GetVisibility();
        }
    }

    renderer = NULL;
    mapper = NULL;
}


// ****************************************************************************
//  Method: avtGeometryDrawable destructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
// ****************************************************************************

avtGeometryDrawable::~avtGeometryDrawable()
{
    //
    // If we are still attached to a renderer, remove ourselves.
    //
    if (renderer != NULL)
    {
        Remove(renderer);
    }

    if (actors != NULL)
    {
        for (int i = 0 ; i < nActors ; i++)
        {
            if (actors[i] != NULL)
            {
                actors[i]->Delete();
                actors[i] = NULL;
            }
        }
    }
    delete [] actors;
    delete [] actorsVisibility;
}


// ****************************************************************************
//  Method: avtGeometryDrawable::SetMapper
//
//  Purpose:
//      Tells the geometry drawable which mapper it comes from.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

void
avtGeometryDrawable::SetMapper(avtMapper *m)
{
    mapper = m;
}


// ****************************************************************************
//  Method: avtGeometryDrawable::Add
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
avtGeometryDrawable::Add(vtkRenderer *ren)
{
    renderer = ren;
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            ren->AddActor(actors[i]);
        }
    }
}


// ****************************************************************************
//  Method: avtGeometryDrawable::Remove
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
//  Modifications:
//
//    Hank Childs, Fri May  3 15:14:59 PDT 2002
//    Added a trick/hack to allow actors to be removed from a window without
//    affecting their display lists.
//
// ****************************************************************************

void
avtGeometryDrawable::Remove(vtkRenderer *ren)
{
    if (ren != renderer)
    {
        EXCEPTION0(ImproperUseException);
    }

    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            //
            // This is supposed to approximate the RemoveActor call of
            // vtkRenderer.  That call also tells the actor to release its
            // graphics resources, which does not work well for us, since
            // we remove the actors every time we add new plots (the viewer
            // does a ClearPlots) and also when the vis window re-orders the
            // actors.
            //
            // THIS IS A MAINTENANCE ISSUE.  This routine should be the same
            // as vtkRenderer::RemoveActor, but does not call
            // ReleaseGraphicsResources (which is actually called indirectly
            // through vtkViewport::RemoveProp).
            //
            //ren->RemoveActor(actors[i]);
            //
            ren->GetActors()->RemoveItem(actors[i]);
            actors[i]->RemoveConsumer(ren);
            ren->GetProps()->RemoveItem(actors[i]);
        }
    }
    renderer = NULL;
}


// ****************************************************************************
//  Method: avtGeometryDrawable::VisibilityOn
//
//  Purpose:
//      Turns the visibility of the drawable back on (this is the default).
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 15 10:51:59 PST 2001
//    Changed to set visibility of individual actors based on stored
//    visibility created in constructor.  
//
// ****************************************************************************

void
avtGeometryDrawable::VisibilityOn(void)
{
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->SetVisibility(actorsVisibility[i]);
        }
    }
}


// ****************************************************************************
//  Method: avtGeometryDrawable::VisibilityOff
//
//  Purpose:
//      Turns off the visibility of the drawable (for bounding box mode, etc).
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jul 22 13:17:35 PDT 2002
//    Give ourselves another chance to check out the actor's visibility.
//    This is because the transparency actor may have modified it.
//
// ****************************************************************************

void
avtGeometryDrawable::VisibilityOff(void)
{
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            actorsVisibility[i] = actors[i]->GetVisibility();
            if (actorsVisibility[i] != 0)
            {
                actors[i]->VisibilityOff();
            }
        }
    }
}


// ****************************************************************************
//  Method: avtGeometryDrawable::ShiftByVector
//
//  Purpose:
//      Shifts the drawable by a vector. 
//
//  Arguments:
//      vec    The vector to shift by.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
// ****************************************************************************

void
avtGeometryDrawable::ShiftByVector(const float vec[3])
{
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            // VTK is ridiculous -- needs const.
            float v[3];
            v[0] = vec[0];
            v[1] = vec[1];
            v[2] = vec[2];
            actors[i]->SetPosition(v);
        }
    }
}


// ****************************************************************************
//  Method: avtGeometryDrawable::ScaleByVector
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
avtGeometryDrawable::ScaleByVector(const float vec[3])
{
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            // VTK is ridiculous -- needs const.
            float v[3];
            v[0] = vec[0];
            v[1] = vec[1];
            v[2] = vec[2];
            actors[i]->SetScale(v);
        }
    }
}


// ****************************************************************************
//  Method: avtGeometryDrawable::GetDataObject
//
//  Purpose:
//      Gets the datasets associated with this geometry drawable.
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
avtGeometryDrawable::GetDataObject(void)
{
    if (mapper != NULL)
    {
        return mapper->GetInput();
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtGeometryDrawable::SetTransparencyActor
//
//  Purpose:
//      Sets the mapper's transparency actor.
//
//  Arguments:
//      act      The transparency actor.
//
//  Returns:     The index for this actor with the transparency actor.
//
//  Programmer:  Hank Childs
//  Creation:    July 7, 2002
//
// ****************************************************************************

int
avtGeometryDrawable::SetTransparencyActor(avtTransparencyActor *act)
{
    int rv = -1;
    if (mapper != NULL)
    {
        rv = mapper->SetTransparencyActor(act);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtGeometryDrawable::UpdateScaleFactor
//
//  Purpose:
//      Allow the actors to update their scale. 
//
//  Programmer:  Kathleen Bonnell
//  Creation:    July 19, 2002
//
// ****************************************************************************

void
avtGeometryDrawable::UpdateScaleFactor()
{
    ;
}


// ****************************************************************************
//  Method: avtGeometryDrawable::TurnLightingOn
//
//  Purpose:
//      Allow the mapper to update the lighting coefficients.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 13, 2002
//
// ****************************************************************************

void
avtGeometryDrawable::TurnLightingOn()
{
   mapper->GlobalLightingOn();
}


// ****************************************************************************
//  Method: avtGeometryDrawable::TurnLightingOff
//
//  Purpose:
//      Allow the mapper to update the lighting coefficients.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 13, 2002
//
// ****************************************************************************

void
avtGeometryDrawable::TurnLightingOff()
{
   mapper->GlobalLightingOff();
}

// ****************************************************************************
//  Method: avtGeometryDrawable::SetAmbientCoefficient
//
//  Purpose:
//      Allow the mapper to update the ambient lighting coefficient. 
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 13, 2002
//
// ****************************************************************************

void
avtGeometryDrawable::SetAmbientCoefficient(const float amb)
{
   mapper->GlobalSetAmbientCoefficient(amb);
}

// ****************************************************************************
//  Method: avtGeometryDrawable::SetSurfaceRepresentation
//
//  Purpose:
//      Sets the drawable's surface representation.
//
//  Arguments:
//      rep : The new surface representation.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 23 15:58:48 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Sat Oct 19 15:07:04 PDT 2002 
//    Disable lighting for Wireframe and Points representation.
//
// ****************************************************************************

void
avtGeometryDrawable::SetSurfaceRepresentation(int rep)
{
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            vtkProperty *prop = actors[i]->GetProperty();
            if(prop != NULL)
            {
                int actorRep = prop->GetRepresentation();
                if(rep == 0 && actorRep != VTK_SURFACE)
                {
                    prop->SetRepresentation(VTK_SURFACE);
                    if (mapper->GetLighting())
                    {
                        prop->SetAmbient(mapper->GetGlobalAmbientCoefficient());
                        prop->SetDiffuse(1.);
                    }
                }
                else if(rep == 1 && actorRep != VTK_WIREFRAME)
                {
                    prop->SetRepresentation(VTK_WIREFRAME);
                    prop->SetAmbient(1.);
                    prop->SetDiffuse(0.);
                }
                else if(rep == 2 && actorRep != VTK_POINTS)
                {
                    prop->SetRepresentation(VTK_POINTS);
                    prop->SetAmbient(1.);
                    prop->SetDiffuse(0.);
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: avtGeometryDrawable::SetSpecularProperties
//
//  Purpose:
//      Sets the drawable's surface representation.
//
//  Arguments:
//      flag  :  true to enable specular, false otherwise
//      coeff :  the new specular coefficient
//      power :  the new specular power
//      color :  the new specular color
//
//  Programmer: Jeremy Meredith
//  Creation:   November 14, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtGeometryDrawable::SetSpecularProperties(bool flag, float coeff, float power,
                                           const ColorAttribute &color)
{
    for (int i = 0 ; i < nActors ; i++)
    {
        if (actors[i] != NULL)
        {
            vtkProperty *prop = actors[i]->GetProperty();
            if(prop != NULL)
            {
                prop->SetSpecular(flag ? coeff : 0);
                prop->SetSpecularPower(power);
                int r = color.Red();
                int g = color.Green();
                int b = color.Blue();
                prop->SetSpecularColor(float(r)/255.,
                                       float(g)/255.,
                                       float(b)/255.);
            }
        }
    }
}

// ****************************************************************************
// Method: avtGeometryDrawable::SetImmediateModeRendering
//
// Purpose: 
//   Sets the rendering mode for the mappers.
//
// Arguments:
//   val : The new immediate rendering mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 16:47:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtGeometryDrawable::SetImmediateModeRendering(bool val)
{
    if(mapper->GetImmediateModeRendering() != val)
        mapper->SetImmediateModeRendering(val);
}

