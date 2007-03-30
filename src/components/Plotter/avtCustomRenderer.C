// ************************************************************************* //
//                             avtCustomRenderer.C                           //
// ************************************************************************* //

#include <avtCustomRenderer.h>

// For NULL
#include <stdio.h>
#include <iostream.h>

#include <vtkRenderer.h>

#include <avtImageMapper.h>


// ****************************************************************************
//  Method: avtCustomRenderer constructor
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

avtCustomRenderer::avtCustomRenderer()
{
    overrideRenderCallback     = NULL;
    overrideRenderCallbackArgs = NULL;
    VTKRen = NULL;
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
avtCustomRenderer::SetRange(float min, float max)
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
avtCustomRenderer::GlobalSetAmbientCoefficient(const float)
{
    ;
}
