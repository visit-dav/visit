/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtStreamlineRenderer.C                          //
// ************************************************************************* //

#include "avtStreamlineRenderer.h"

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
#include <avtOpenGLStreamlineRenderer.h>
#include <avtMesaStreamlineRenderer.h>

#include <ImproperUseException.h>
#include <InvalidLimitsException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Constructor:  avtStreamlineRenderer::avtStreamlineRenderer
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

avtStreamlineRenderer::avtStreamlineRenderer()
{
    initialized = false;

    rendererImplementation = NULL;
    currentRendererIsValid = false;

    ambient_coeff = 0;
    spec_coeff = 0;
    spec_power = 0;
    spec_r = 0;
    spec_g = 0;
    spec_b = 0;
}

// ****************************************************************************
//  Destructor:  avtStreamlineRenderer::~avtStreamlineRenderer
//
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

avtStreamlineRenderer::~avtStreamlineRenderer()
{
    ReleaseGraphicsResources();
}

// ****************************************************************************
//  Method:  avtStreamlineRenderer::ReleaseGraphicsResources
//
//  Purpose:
//    Delete the renderer implementation.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************

void
avtStreamlineRenderer::ReleaseGraphicsResources()
{
    if (rendererImplementation)
    {
        VTKRen->GetRenderWindow()->MakeCurrent();
        delete rendererImplementation;
        rendererImplementation = NULL;
    }
}

// ****************************************************************************
//  Method: avtStreamlineRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer based on
//      whether we should use OpenGL or Mesa.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

avtStreamlineRenderer *
avtStreamlineRenderer::New()
{
    return new avtStreamlineRenderer;
}
 
// ****************************************************************************
//  Method:  avtStreamlineRenderer::Render
//
//  Purpose:
//    Set up things necessary to call the renderer implentation.  Make a new
//    implementation object if things have changed.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineRenderer::Render(vtkDataSet *ds)
{
    if (!currentRendererIsValid || !rendererImplementation)
    {
        if (rendererImplementation)
            delete rendererImplementation;

        if (avtCallback::GetNowinMode())
            rendererImplementation = new avtMesaStreamlineRenderer;
        else
            rendererImplementation = new avtOpenGLStreamlineRenderer;
        
        currentRendererIsValid = true;
        rendererImplementation->SetLevelsLUT(levelsLUT);
    }

    // Do other initialization
    if (!initialized)
        Initialize(ds);

    // get data set
    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION1(ImproperUseException,
                   "Inappropriate mesh type for Streamline Plot ");
    }
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
//  Method:  avtStreamlineRenderer::Initialize
//
//  Purpose:
//    Calculate some one-time stuff with respect to the data set
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineRenderer::Initialize(vtkDataSet *ds)
{
    initialized = true;
}


// ****************************************************************************
//  Method:  avtStreamlineRenderer::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtStreamlineRenderer::SetAtts(const AttributeGroup *a)
{
    atts = *(const StreamlineAttributes*)a;

    initialized = false;
}

// ****************************************************************************
//  Method:  avtStreamlineRenderer::GlobalSetAmbientCoefficient
//
//  Purpose:
//    We've got to track this ourselves.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************

void
avtStreamlineRenderer::GlobalSetAmbientCoefficient(float coeff)
{
    ambient_coeff = coeff;
}

// ****************************************************************************
//  Method:  avtStreamlineRenderer::GlobalLightingOn
//
//  Purpose:
//    Track when GlobalLightingOn is called.  See the comment below.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************

void
avtStreamlineRenderer::GlobalLightingOn()
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
//  Method:  avtStreamlineRenderer::GlobalLightingOff
//
//  Purpose:
//    Force ambient to 1.0.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************

void
avtStreamlineRenderer::GlobalLightingOff()
{
    ambient_coeff = 1.0;
}

// ****************************************************************************
//  Method:  avtStreamlineRenderer::SetSpecularProperties
//
//  Purpose:
//    Track specular properties.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************

void
avtStreamlineRenderer::SetSpecularProperties(bool flag, double coeff, 
                                             double power,
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
//  Method:  avtStreamlineRenderer::InvalidateColors
//
//  Purpose:
//    This lets the rendererImplementation know that something
//    has caused its colors to potentially be invalid.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************

void
avtStreamlineRenderer::InvalidateColors()
{
    if (rendererImplementation)
        rendererImplementation->InvalidateColors();
}

// ****************************************************************************
// Method: avtStreamlineRenderer::SetLevelsLUT
//
// Purpose: 
//   Sets a lookup table for the renderer to use.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// Modifications:
//   
// ****************************************************************************

void 
avtStreamlineRenderer::SetLevelsLUT(avtLookupTable *lut)
{
    levelsLUT = lut;

    if (rendererImplementation)
        rendererImplementation->SetLevelsLUT(lut);
}
