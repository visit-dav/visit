// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtCustomRenderer.C                           //
// ************************************************************************* //

#include <avtCustomRenderer.h>

// For NULL
#include <stdio.h>
#include <visitstream.h>

#include <vtkActor.h>
#include <vtkRenderer.h>

#include <avtImageMapper.h>
#include <ColorAttribute.h>


// ****************************************************************************
//  Method: avtCustomRenderer constructor
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug  4 11:14:22 PDT 2003
//    Initialize imemdiateModeRendering.
//
// ****************************************************************************

avtCustomRenderer::avtCustomRenderer()
{
    overrideRenderCallback     = NULL;
    overrideRenderCallbackArgs = NULL;
    VTKRen = NULL;
    VTKActor = NULL;
}


// ****************************************************************************
//  Method: avtCustomRenderer destructor
//
//  Programmer: Hank Childs
//  Creation:   May 5, 2003
//
// ****************************************************************************

avtCustomRenderer::~avtCustomRenderer()
{
    if (VTKRen != NULL)
        VTKRen->Delete();
}


// ****************************************************************************
//  Method: avtCustomRenderer::SetVTKRenderer
//
//  Programmer: Hank Childs
//  Creation:   May 5, 2003
//
// ****************************************************************************

void
avtCustomRenderer::SetVTKRenderer(vtkRenderer *r)
{
    if (VTKRen != NULL)
        VTKRen->Delete();
    VTKRen = r;
    if (VTKRen != NULL)
        VTKRen->Register(NULL);
}


// ****************************************************************************
//  Method: avtCustomRenderer::SetVTKActor
//
//  Programmer: Carson Brownlee
//  Creation:   July 24, 2012
//
// ****************************************************************************

void
avtCustomRenderer::SetVTKActor(vtkActor *a)
{
    VTKActor = a;
}

void
avtCustomRenderer::SetAlternateDisplay(void *)
{
    // nothing.
}

// ****************************************************************************
//  Method: avtCustomRenderer::SetView
//
//  Purpose:
//      Sets the view.
//
//  Arguments:
//      v       The new view.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
// ****************************************************************************

void
avtCustomRenderer::SetView(avtViewInfo &v)
{
    view = v;
}


// ****************************************************************************
//  Method: avtCustomRenderer::SetRange
//
//  Purpose:
//      Sets the range that we should use for the variable.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2001
//
// ****************************************************************************

void
avtCustomRenderer::SetRange(double min, double max)
{
    varmin = min;
    varmax = max;
}


// ****************************************************************************
//  Method: avtCustomRenderer::RegisterOverrideRenderCallback
//
//  Purpose:
//      Registers a callback that will allow rendering to be overridden when
//      the callback deems appropriate.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtCustomRenderer::RegisterOverrideRenderCallback(OverrideRenderCallback orc,
                                                  void *orcArgs)
{
    overrideRenderCallback     = orc;
    overrideRenderCallbackArgs = orcArgs;
}


// ****************************************************************************
//  Method: avtCustomRenderer::Execute
//
//  Purpose:
//      Do a render.  Possibly override the Render method of the derived type
//      with a callback.
//
//  Arguments:
//      input    The input dataset.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtCustomRenderer::Execute(vtkDataSet *input)
{
    avtDataObject_p image = NULL;
    if (overrideRenderCallback != NULL)
    {
        overrideRenderCallback(overrideRenderCallbackArgs, image);
    }

    if (*image != NULL)
    {
        avtImageMapper mapper;
        mapper.SetInput(image);
        mapper.Draw(VTKRen);
    }
    else
    {
        //
        // Go ahead and do a normal render.
        //
        Render(input);
    }
}


// ****************************************************************************
//  Method: avtCustomRenderer::GlobalLightingOn
//
//  Purpose:
//      Turn on lighting.  This is a stub for derived types. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtCustomRenderer::GlobalLightingOn()
{
    ;
}


// ****************************************************************************
//  Method: avtCustomRenderer::GlobalLightingOff
//
//  Purpose:
//      Turn off lighting.  This is a stub for derived types. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtCustomRenderer::GlobalLightingOff()
{
    ;
}


// ****************************************************************************
//  Method: avtCustomRenderer::GlobalLightingOff
//
//  Purpose:
//     Set the ambient lighting coefficient to the specified value.
//     This is a stub for derived types. 
//
//  Arguments:
//     <un-named>  The new ambient lighting coefficient.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
// ****************************************************************************

void
avtCustomRenderer::GlobalSetAmbientCoefficient(const double)
{
    ;
}


// ****************************************************************************
//  Method: avtCustomRenderer::SetSpecularProperties
//
//  Purpose:
//    Sets the specified specular properties. 
//    This is a stub for derived types. 
//
//  Arguments:
//    <un-named> :  true to enable specular, false otherwise
//    <un-named> :  the new specular coefficient
//    <un-named> :  the new specular power
//    <un-named> :  the new specular color
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004
//
// ****************************************************************************

void
avtCustomRenderer::SetSpecularProperties(bool , double , double ,
                                         const ColorAttribute &)
{
    ;
}

// ****************************************************************************
//  Method: avtCustomRenderer::SetSurfaceRepresentation
//
//  Purpose:
//     Set the surface representation to the specified value.
//     This is a stub for derived types. 
//
//  Arguments:
//     <un-named>  The new surface representation.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//
// ****************************************************************************

void
avtCustomRenderer::SetSurfaceRepresentation(int rep)
{
    ;
}


// ****************************************************************************
//  Method: avtCustomRenderer::SetBoundingBox
//
//  Purpose:
//      Sets the bounding box of the input.
//
//  Programmer: Hank Childs
//  Creation:   September 30, 2010
//
// ****************************************************************************

void
avtCustomRenderer::SetBoundingBox(const double *b)
{
    for (int i = 0 ; i < 6 ; i++)
        bbox[i] = b[i];
}


