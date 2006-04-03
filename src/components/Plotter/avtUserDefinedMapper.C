// ************************************************************************* //
//                           avtUserDefinedMapper.C                          //
// ************************************************************************* //

#include <avtUserDefinedMapper.h>

#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

#include <ColorAttribute.h>

#include <avtCallback.h>

#include <VisItException.h>


// ****************************************************************************
//  Method: avtUserDefinedMapper constructor
//
//  Arguments:
//      r       The custom-defined renderer.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
// ****************************************************************************

avtUserDefinedMapper::avtUserDefinedMapper(avtCustomRenderer_p r)
{
    renderer = r;
}


// ****************************************************************************
//  Method: avtUserDefinedMapper destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtUserDefinedMapper::~avtUserDefinedMapper()
{
    ;
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::GetDataRange
//
//  Arguments:
//    rmin      The minimum in the range.
//    rmax      The maximum in the range.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 4, 2001 
//
// ****************************************************************************

bool
avtUserDefinedMapper::GetDataRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmin = 1.;
        return false;
    }

    avtMapper::GetRange(rmin, rmax);
    return true;
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::GetCurrentDataRange
//
//  Arguments:
//    rmin      The minimum in the range.
//    rmax      The maximum in the range.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 4, 2001 
//
// ****************************************************************************

bool
avtUserDefinedMapper::GetCurrentDataRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmin = 1.;
        return false;
    }

    avtMapper::GetCurrentRange(rmin, rmax);
    return true;
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the user defined mapper to
//      determine its extents and set them.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 19 14:26:05 PST 2003
//    Removed test for min == max, no longer an issue with vtkLookupTables or 
//    vtkMappers.
// ****************************************************************************

void
avtUserDefinedMapper::CustomizeMappers(void)
{
    if (renderer->OperatesOnScalars())
    {
        double mmin = 0.;
        double mmax = 0.;
        GetRange(mmin, mmax);
    
        //
        // Now tell the renderer what its variable range should be.
        //
        renderer->SetRange(mmin, mmax);
    }
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::CreateMapper
//
//  Purpose:
//      Creates a mapper that will act as a bridge from vtk to the user's
//      custom defined renderer.
//
//  Returns:     The mapper.
//
//  Programmer:  Hank Childs
//  Creation:    March 26, 2001
//
// ****************************************************************************

vtkDataSetMapper *
avtUserDefinedMapper::CreateMapper(void)
{
    vtkUserDefinedMapperBridge *rv = vtkUserDefinedMapperBridge::New();
    rv->SetRenderer(renderer);

    return rv;
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::GlobalLightingOn
//
//  Purpose:
//      Tells the renderer that global lighting has been turned on. 
//      Allows the renderer to update lighting coefficients if necessary. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 13, 2002 
//
// ****************************************************************************

void 
avtUserDefinedMapper::GlobalLightingOn()
{
    renderer->GlobalLightingOn();
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::GlobalLightingOff
//
//  Purpose:
//      Tells the renderer that global lighting has been turned off. 
//      Allows the renderer to update lighting coefficients if necessary. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 13, 2002 
//
// ****************************************************************************

void 
avtUserDefinedMapper::GlobalLightingOff()
{
    renderer->GlobalLightingOff();
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::GlobalSetAmbientCoefficient
//
//  Purpose:
//      Tells the renderer that global ambient lighting coefficient has
//      been set.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 13, 2002 
//
// ****************************************************************************

void 
avtUserDefinedMapper::GlobalSetAmbientCoefficient(const double amb)
{
    renderer->GlobalSetAmbientCoefficient(amb);
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::SetSpecularProperties
//
//  Purpose:
//    Sends the specular properties to the renderer. 
//
//  Arguments:
//    flag  :  true to enable specular, false otherwise
//    coeff :  the new specular coefficient
//    power :  the new specular power
//    color :  the new specular color
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtUserDefinedMapper::SetSpecularProperties(bool flag, double coeff, 
                                           double power,
                                           const ColorAttribute &color)
{
    renderer->SetSpecularProperties(flag, coeff, power, color);
}


// ****************************************************************************
//  Method: avtUserDefinedMapper::SetSurfaceRepresentation
//
//  Purpose:
//    Sends the surface representation to the renderer. 
//
//  Arguments:
//    rep  :  the new representation 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtUserDefinedMapper::SetSurfaceRepresentation(int rep)
{
    renderer->SetSurfaceRepresentation(rep);
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::New
//
//  Purpose:
//      Meets the VTK convention of instantiating objects through a static
//      function, since the constructors and destructors are protected for
//      reference counting reasons.
//
//  Returns:     A new instance of vtkUserDefinedMapperBridge
//
//  Programmer:  Hank Childs
//  Creation:    March 26, 2001
//
// ****************************************************************************

vtkUserDefinedMapperBridge *
vtkUserDefinedMapperBridge::New(void)
{
    //
    // First try to create the object from the vtkObjectFactory
    //
    vtkObject *ret = NULL;
    ret = vtkObjectFactory::CreateInstance("vtkUserDefinedMapperBridge");
    if (ret == NULL)
    {
        ret = new vtkUserDefinedMapperBridge;
    }

    return (vtkUserDefinedMapperBridge *) ret;
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge constructor
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
// ****************************************************************************

vtkUserDefinedMapperBridge::vtkUserDefinedMapperBridge()
{
    ren = NULL;
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::SetRenderer
//
//  Purpose:
//      Registers the renderer that the bridge should call asked to map data.
//
//  Arguments:
//      r       The renderer.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
// ****************************************************************************

void 
vtkUserDefinedMapperBridge::SetRenderer(avtCustomRenderer_p r)
{
    ren = r;
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::Render
//
//  Purpose:
//      Called when a mapper should makes its rendering calls.  This re-routes
//      the call to the avtCustomRenderer and sets some information with the
//      renderer.
//
//  Arguments:
//      r          The vtkRenderer.
//      <unnamed>  The actor associated with the mapper.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Mar  1 09:59:45 PST 2006
//    Added some exception handling.  Not a good idea to let an exception
//    be thrown up through VTK's GL code.
//
// ****************************************************************************

void 
vtkUserDefinedMapperBridge::Render(vtkRenderer *r, vtkActor *)
{
    //
    // Retrieve the view information and set it with the avt renderer.
    //
    avtViewInfo view;
    vtkCamera *cam = r->GetActiveCamera();
    view.SetViewFromCamera(cam);
    ren->SetView(view);

    //
    // Ask the avt renderer to draw this dataset.
    //
    vtkDataSet *input = this->GetInput();
    ren->SetVTKRenderer(r);
    TRY
    {
        ren->Execute(input);
    }
    CATCH2(VisItException, ve)
    {
        //
        // We do not want to throw an exception here.  This call has a bunch
        // of VTK/GL code above it and throwing an exception will leave it
        // in a very bad state.  So, log the exception in avtCallback and let
        // the VisWindow look for it after the render is finished.
        //
        avtCallback::SetRenderingException(ve.Message());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::ImmediateModeRenderingOn
//
//  Purpose:
//    Tells the renderer to turn on immediate mode rendering.   
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 4, 2003 
//
// ****************************************************************************

void
vtkUserDefinedMapperBridge::ImmediateModeRenderingOn()
{
    ren->ImmediateModeRenderingOn();
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::ImmediateModeRenderingOff
//
//  Purpose:
//    Tells the renderer to turn off immediate mode rendering.   
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 4, 2003 
//
// ****************************************************************************

void
vtkUserDefinedMapperBridge::ImmediateModeRenderingOff()
{
    ren->ImmediateModeRenderingOff();
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::SetImmediateModeRendering
//
//  Purpose:
//    Tells the renderer to turn on/off  immediate mode rendering based on
//    the passed mode.
//
//  Arguments:
//    mode      Whether immediate mode rendering should be on or off. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 4, 2003 
//
// ****************************************************************************

void
vtkUserDefinedMapperBridge::SetImmediateModeRendering(int mode)
{
    if (mode)
        ren->ImmediateModeRenderingOn();
    else
        ren->ImmediateModeRenderingOff();
}


// ****************************************************************************
//  Method: vtkUserDefinedMapperBridge::GetImmediateModeRendering
//
//  Purpose:
//    Retrieve the value of immediat mode rendering from the renderer.
//
//  Returns:
//    True if immediate mode rendering is on, false otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 4, 2003 
//
// ****************************************************************************

int
vtkUserDefinedMapperBridge::GetImmediateModeRendering()
{
    return ren->GetImmediateModeRendering();
}

