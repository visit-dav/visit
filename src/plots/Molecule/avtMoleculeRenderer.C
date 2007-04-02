// ************************************************************************* //
//                            avtMoleculeRenderer.C                          //
// ************************************************************************* //

#include "avtMoleculeRenderer.h"

#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <math.h>
#include <float.h>

#include <avtCallback.h>
#include <avtOpenGLMoleculeRenderer.h>
#include <avtMesaMoleculeRenderer.h>

#include <ImproperUseException.h>
#include <InvalidLimitsException.h>


// ****************************************************************************
//  Constructor:  avtMoleculeRenderer::avtMoleculeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//
// ****************************************************************************
avtMoleculeRenderer::avtMoleculeRenderer()
{
    initialized = false;
    primaryVariable = "";

    rendererImplementation = NULL;
    currentRendererIsValid = false;

    ambient_coeff = 0;
    spec_coeff = 0;
}

// ****************************************************************************
//  Destructor:  avtMoleculeRenderer::~avtMoleculeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//
// ****************************************************************************
avtMoleculeRenderer::~avtMoleculeRenderer()
{
    ReleaseGraphicsResources();
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::ReleaseGraphicsResources
//
//  Purpose:
//    Delete the renderer implementation.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeRenderer::ReleaseGraphicsResources()
{
    if (rendererImplementation)
    {
        VTKRen->GetRenderWindow()->MakeCurrent();
        delete rendererImplementation;
        rendererImplementation = NULL;
    }
}

// ****************************************************************************
//  Method: avtMoleculeRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer based on
//      whether we should use OpenGL or Mesa.
//
//  Returns:    A renderer that is of type derived from this class.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//
// ****************************************************************************
avtMoleculeRenderer *
avtMoleculeRenderer::New(void)
{
    return new avtMoleculeRenderer;
}
 
// ****************************************************************************
//  Method:  avtMoleculeRenderer::Render
//
//  Purpose:
//    Set up things necessary to call the renderer implentation.  Make a new
//    implementation object if things have changed.
//
//  Arguments:
//    ds         the dataset to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Brad Whitlock, Fri Apr 7 11:36:04 PDT 2006
//    Pass window size to the renderer implementation.
//
// ****************************************************************************

void
avtMoleculeRenderer::Render(vtkDataSet *ds)
{
    if (!currentRendererIsValid || !rendererImplementation)
    {
        if (rendererImplementation)
            delete rendererImplementation;

        if (avtCallback::GetNowinMode())
        {
            rendererImplementation = new avtMesaMoleculeRenderer;
        }
        else
        { 
            rendererImplementation = new avtOpenGLMoleculeRenderer;
        }
        currentRendererIsValid = true;

        rendererImplementation->SetLevelsLUT(levelsLUT);
    }

    // Do other initialization
    if (!initialized)
    {
        Initialize(ds);
    }

    // get data set
    vtkPolyData *polydata = (vtkPolyData*)ds;


    int winsize[2];
    winsize[0] = VTKRen->GetVTKWindow()->GetSize()[0];
    winsize[1] = VTKRen->GetVTKWindow()->GetSize()[1];

    rendererImplementation->Render(polydata,
                                   atts,
                                   immediateModeRendering,
                                   varmin, varmax,
                                   ambient_coeff,
                                   spec_coeff, spec_power,
                                   spec_r, spec_g, spec_b, winsize);
}


// ****************************************************************************
//  Method:  avtMoleculeRenderer::Initialize
//
//  Purpose:
//    Calculate some one-time stuff with respect to the data set
//
//  Arguments:
//    ds      : the data set
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//
// ****************************************************************************
void
avtMoleculeRenderer::Initialize(vtkDataSet *ds)
{
    // get data set
    vtkPolyData  *polydata = (vtkPolyData*)ds;

    initialized = true;
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//
// ****************************************************************************
void
avtMoleculeRenderer::SetAtts(const AttributeGroup *a)
{
    const MoleculeAttributes *newAtts = (const MoleculeAttributes*)a;
    atts = *(const MoleculeAttributes*)a;

    initialized = false;
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::GlobalSetAmbientCoefficient
//
//  Purpose:
//    We've got to track this ourselves.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeRenderer::GlobalSetAmbientCoefficient(float coeff)
{
    ambient_coeff = coeff;
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::GlobalLightingOn
//
//  Purpose:
//    Track when GlobalLightingOn is called.  See the comment below.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeRenderer::GlobalLightingOn()
{
    //
    // Stupid hack -- when you turn off the ambient lighting, 
    // it never sets the global ambient coefficient to zero.
    // However, it will repeatedly call GlobalLightingOn (followed by
    // calling GlobalSetAmbientCoefficient when it's necessary).  Thus,
    // if we set it to zero here, it will turn it off if there is no
    // ambient light, and it will get set to the true ambient coeff
    // if there is an ambient light when GlobalSetAmbientCoefficient
    // gets called later.
    //
    // This emulates the reason the avtSurfaceAndWireframeRenderer works.
    //
    ambient_coeff = 0.0;
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::GlobalLightingOff
//
//  Purpose:
//    Force ambient to 1.0.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeRenderer::GlobalLightingOff()
{
    ambient_coeff = 1.0;
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::SetSpecularProperties
//
//  Purpose:
//    Track specular properties.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeRenderer::SetSpecularProperties(bool flag, float coeff, 
                                           float power,
                                           const ColorAttribute &color)
{
    spec_coeff = flag ? coeff : 0;
    spec_power = power;

    int r = color.Red();
    int g = color.Green();
    int b = color.Blue();
    spec_r = float(color.Red())/255.;
    spec_g = float(color.Green())/255.;
    spec_b = float(color.Blue())/255.;
}

// ****************************************************************************
//  Method:  avtMoleculeRenderer::InvalidateColors
//
//  Purpose:
//    This lets the rendererImplementation know that something
//    has caused its colors to potentially be invalid.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtMoleculeRenderer::InvalidateColors()
{
    if (rendererImplementation)
        rendererImplementation->InvalidateColors();
}

// ****************************************************************************
// Method: avtMoleculeRenderer::SetLevelsLUT
//
// Purpose: 
//   Sets a lookup table for the renderer to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 29 12:02:11 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void 
avtMoleculeRenderer::SetLevelsLUT(avtLookupTable *lut)
{
    levelsLUT = lut;

    if (rendererImplementation)
        rendererImplementation->SetLevelsLUT(lut);
}
