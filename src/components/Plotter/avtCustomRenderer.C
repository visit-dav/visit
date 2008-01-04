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
//                             avtCustomRenderer.C                           //
// ************************************************************************* //

#include <avtCustomRenderer.h>

// For NULL
#include <stdio.h>
#include <visitstream.h>

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
    immediateModeRendering = false;
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
// Method: avtCustomRenderer::ImmediateModeRenderingOn
//
// Purpose:
//   Turns on immediate rendering mode. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 4, 2003 
//
// ****************************************************************************

void
avtCustomRenderer::ImmediateModeRenderingOn()
{
    immediateModeRendering = true;    
}


// ****************************************************************************
// Method: avtCustomRenderer::SetImmediateModeRendering
//
// Purpose:
//   Turns on/off immediate rendering mode based on the passed value.
//
// Arguments:
//   mode      The new value for immediate mode rendering. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 4, 2003 
//
// ****************************************************************************

void
avtCustomRenderer::SetImmediateModeRendering(bool mode)
{
    immediateModeRendering = mode;
}


// ****************************************************************************
// Method: avtCustomRenderer::GetImmediateModeRendering
//
// Purpose:
//   Returns the value of immediateModeRendering. 
//
// Returns:
//   True if immediateModeRendering is on, false otherwise. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 4, 2003 
//
// ****************************************************************************

bool
avtCustomRenderer::GetImmediateModeRendering()
{
    return immediateModeRendering;
}


// ****************************************************************************
// Method: avtCustomRenderer::ImmediateModeRenderingOff
//
// Purpose:
//   Turns off immediate rendering mode. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 4, 2003
//
// ****************************************************************************

void
avtCustomRenderer::ImmediateModeRenderingOff()
{
    immediateModeRendering = false;    
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

