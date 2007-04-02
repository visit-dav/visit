/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                     avtSurfaceAndWireframeRenderer.C                      //
// ************************************************************************* //

#include "avtSurfaceAndWireframeRenderer.h"

#include <vtkCellData.h>
#include <vtkGeometryFilter.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <avtCallback.h>
#include <avtOpenGLSurfaceAndWireframeRenderer.h>
#include <avtMesaSurfaceAndWireframeRenderer.h>

#include <ImproperUseException.h>

#include <DebugStream.h>
#include <Init.h>
#define ScalRen ((!strcmp(Init::GetComponentName(), "engine")) ? 1 : 0)

// ****************************************************************************
//  Constructor:  avtSurfaceAndWireframeRenderer  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 22 15:06:29 PDT 2001
//    Intialize new members drawVerts, drawLines, drawStrips, drawPolys.
//
//    Kathleen Bonnell, Wed Sep 26 17:47:41 PDT 2001 
//    Added distinction between surface-associated primitives and 
//    edge-associated primitives. 
//
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002 
//    Initialize new members immediateModeRendering and inputNum.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Initialize new members lastAmbient, lastDiffuse.
//
//    Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002 
//    Initialize new member lutColorsChanged.
//
//    Kathleen Bonnell, Thu Aug  7 08:29:31 PDT 2003 
//    Removed immediateModeRendering. 
//
//    Kathleen Bonnell, Tue Aug 26 14:45:31 PDT 2003 
//    Initialize lastWindowSize. 
//
//    Kathleen Bonnell, Thu Sep  2 16:24:49 PDT 2004 
//    Added globalAmbientCoeff and canApplyGlobalRep. 
//
// ****************************************************************************

avtSurfaceAndWireframeRenderer::avtSurfaceAndWireframeRenderer()
{
    input  = NULL;
    LUT    = NULL;
    Colors = NULL;

    prop   = vtkProperty::New();

    resolveTopology  = false;
    scalarVisibility = true;
    scalarRange      = false;
   
    // if range isn't set by user, it won't be used. 
    range[0] = range[1] = 0.; 

    drawSurfaceVerts = drawSurfaceLines = true;
    drawSurfaceStrips = drawSurfacePolys = true;
    canApplyGlobalRep = true;

    drawEdgeLines = drawEdgeStrips = drawEdgePolys = true;

    inputNum = 0;
    ignoreLighting = false;
    lutColorsChanged = false;
    lastWindowSize[0] = lastWindowSize[1] = -1;
    globalAmbientCoeff = 0.;
}


// ****************************************************************************
//  Destructor:  avtSurfaceAndWireframeRenderer  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Eric Brugger, Thu Aug 30 16:27:32 PDT 2001
//    Delete the input and properties to fix a memory leak.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    This renderer does not own the lut, should not delete it.
//
//    Kathleen Bonnell, Fri Sep 28 9:45:49 PDT 2001
//    Colors never instantiaed with New(), don't use Delete() 
//
//    Kathleen Bonnell, Tue Jan 11 16:06:33 PST 2005 
//    Fix memory leak -- use UnRegister with Colors. 
//
// ****************************************************************************

avtSurfaceAndWireframeRenderer::~avtSurfaceAndWireframeRenderer()
{
    inputs.clear();
    if (input != NULL)
    {
        input->Delete();
        input = NULL;
    }
    if (Colors)
    {
        Colors->UnRegister(NULL); 
        Colors = NULL;
    }
    if (prop != NULL)
    {
        prop->Delete();
        prop = NULL;
    }
}


// ****************************************************************************
//  Method: avtSurfaceAndWireframeRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer based on
//      whether we should use OpenGL or Mesa.
//
//  Returns:    A renderer that is of type derived from this class.
//
//  Programmer: Hank Childs
//  Creation:   April 23, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Dec  3 20:37:07 PST 2005
//    Change test for whether or not we are doing software rendering.
//
// ****************************************************************************

avtSurfaceAndWireframeRenderer *
avtSurfaceAndWireframeRenderer::New(void)
{
    if (avtCallback::GetSoftwareRendering())
    {
        return new avtMesaSurfaceAndWireframeRenderer;
    }
    else
    {
        return new avtOpenGLSurfaceAndWireframeRenderer;
    }
}


// ****************************************************************************
//  Method:  avtSurfaceAndWireframeRenderer::Draw
//
//  Purpose: return whether or not the surface primitives (polygons) should
//  be drawn
//
//  Programmer:  Mark C. Miller
//  Creation:    November 22, 2004 
//
// ****************************************************************************

bool
avtSurfaceAndWireframeRenderer::ShouldDrawSurface()
{
    int rep = prop->GetRepresentation();

    return (!(rep == VTK_WIREFRAME && prop->GetEdgeVisibility()));
}

// ****************************************************************************
//  Method:  avtSurfaceAndWireframeRenderer::Draw
//
//  Purpose:
//    Call the necessary helper draw methods to draw the primitives. 
//    
//  Notes:  
//    This routine was taken mostly from vtkOpenGLPolyDataMapper,
//    with a few modifications for workability within this framework.
//
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep  2 16:24:49 PDT 2004
//    Allow DrawEdges to be called, even if Representation is points.
//
// ****************************************************************************

void 
avtSurfaceAndWireframeRenderer::Draw()
{
    // if the primitives are invisible then get out of here 
    if (prop->GetOpacity() <= 0.0)
    {
        return;
    }

    //
    //  No need to draw the surface if we want wireframe mode AND
    //  we want the edges to be drawn, too.
    // 
    if (ShouldDrawSurface())
    {
        DrawSurface();
    }

    if (prop->GetEdgeVisibility())
    {
        DrawEdges();
    }
}


// ****************************************************************************
//  Method:  avtSurfaceAndWireframeRenderer::Render
//
//  Purpose:
//    Render one dataset.
//
//  Arguments:
//    ds         the data set to render
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Eric Brugger, Thu Aug 30 16:27:32 PDT 2001
//    Add code to free the old dataset.  Bump the reference count on the
//    input vtkDataSet so that it is reference counted properly.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    Added logic so that various draw functions will only be
//    called if the primitives are present in the input.
//
//    Kathleen Bonnell, Wed Sep 26 17:47:41 PDT 2001 
//    Added distinction between surface-associated primitives and 
//    edge-associated primitives. 
//
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002 
//    Keep track of input ds, and update flags that keep track of 
//    certain modifications. 
//    
//    Kathleen Bonnell, Tue Nov 26 15:39:16 PST 2002 
//    Removed unused members lastAmbient, lastDiffuse. 
//    
//    Kathleen Bonnell, Thu Aug  7 08:35:53 PDT 2003 
//    Added early return if there are no points in the dataset.
//    
//    Kathleen Bonnell, Fri Aug 22 12:40:55 PDT 2003 
//    Set lutColorsChanged to false. 
//    
//    Kathleen Bonnell, Tue Aug 26 14:45:31 PDT 2003
//    If window size has changed and we are in Scalable Rendering mode,
//    ReleaseGraphicsResources.   Set lastWindowSize and setupModified.
//    
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::Render(vtkDataSet *ds)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return;
    }

    for (inputNum = 0; inputNum < inputs.size(); inputNum++)
    {
        if (inputs[inputNum] == ds) break;
    }
    if (inputNum >= inputs.size())
    {
        inputs.push_back(ds);
        surfaceModified.push_back(true);
        edgesModified.push_back(true);
        setupModified.push_back(true);
        propMTime.push_back(0);
        lastRep.push_back(-1);
        lastInterp.push_back(-1);
        LEC temp;
        temp.r = temp.g = temp.b = -1;
        lastEdgeColor.push_back(temp);
    }

    //
    // For efficiency: if input type is vtkPolyData, there's no 
    // need to pass it through the geometry filter.
    //
  
    if (input != NULL)
    {
        input->Delete();
    }

    if (ds->GetDataObjectType() != VTK_POLY_DATA) 
    {
        vtkGeometryFilter *gf = vtkGeometryFilter::New();
        gf->SetInput(ds);
        input = vtkPolyData::New();
        gf->SetOutput(input);
        gf->Update();
    }
    else 
    {
        input = (vtkPolyData *)ds;
        input->Register(NULL);
    }

    int *curSize = VTKRen->GetRenderWindow()->GetSize();
    if (ScalRen && ((curSize[0] != lastWindowSize[0]) ||
        (curSize[0] != lastWindowSize[0])))
    {
        // Changing size of the render window in scalable rendering
        // (offscreen) mode causes the rendering Context to be destroyed.
        // All display list ids stored will be invalid, so allow
        // derived types to be notified. 
        ReleaseGraphicsResources();
    } 

    SetColors();
    SetupGraphicsLibrary();
    //
    // why draw primitives that aren't present?
    //
    drawSurfaceVerts  &= (input->GetNumberOfVerts()  != 0);
    drawSurfaceLines  &= (input->GetNumberOfLines()  != 0);
    drawSurfaceStrips &= (input->GetNumberOfStrips() != 0);
    drawSurfacePolys  &= (input->GetNumberOfPolys()  != 0);
    drawEdgeLines     &= (input->GetNumberOfLines()  != 0);
    drawEdgeStrips    &= (input->GetNumberOfStrips() != 0);
    drawEdgePolys     &= (input->GetNumberOfPolys()  != 0);
    Draw(); 

    //
    // Keep track of certain modifications.
    // 
    surfaceModified[inputNum] = false;
    edgesModified[inputNum] = false;
    setupModified[inputNum] = false;
    propMTime[inputNum] = prop->GetMTime();
    lastRep[inputNum] = prop->GetRepresentation();
    lastInterp[inputNum] = prop->GetInterpolation();
    lastEdgeColor[inputNum].r = prop->GetEdgeColor()[0];
    lastEdgeColor[inputNum].g = prop->GetEdgeColor()[1];
    lastEdgeColor[inputNum].b = prop->GetEdgeColor()[2];
    lutColorsChanged = false;
    lastWindowSize[0] = curSize[0];
    lastWindowSize[1] = curSize[1];
}


// ****************************************************************************
//  Method: avtSurfaceAndWireframeRenderer::SetColors 
//
//  Purpose:
//    Creates the vtkUnsignedCharArray that will be used for coloring 
//    primitives, if necessary. 
//    
//  Notes:  
//    This routine was taken mostly from vtkMapper::GetColors
//    with a few modifications so it would work here. 
//
//  Programmer:  Kathleen Bonnell  (thanks to Kitware)
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    Throw an exception if the user has not specified a lut instead
//    of creating a default one.
//
//    Kathleen Bonnell, Fri Sep 28 9:45:49 PDT 2001
//    Colors never instantiated with New(), don't use Delete(). 
//    
//    Kathleen Bonnell, Mon Oct 29 15:27:41 PST 2001 
//    Match VTK 4.0 API by retrieving point/cell data as vtkDataArray.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    GetActiveScalars() no longer part of VTK 4.0 api.
//
//    Kathleen Bonnell, Tue Jan 11 16:06:33 PST 2005 
//    Fix memory leak -- use Register and UnRegister with Colors. 
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SetColors()
{
    // make sure we have an input
    if (input == NULL)
    {
        return ;
    }

    if (Colors)
    {
        Colors->UnRegister(NULL);
        Colors = NULL;
    }
   
    vtkDataArray *scalars = input->GetPointData()->GetScalars();
    if (!scalars)
    {
        scalars = input->GetCellData()->GetScalars();
    }
  
    // do we have any scalars? and do we want to use them?
    if (scalars && scalarVisibility)
    {
        // if the scalars have a lookup table use it instead
        if (scalars->GetLookupTable())
        {
            SetLookupTable(scalars->GetLookupTable());
        }
        else
        {
            // make sure we have a lookup table
            if (LUT == NULL)
            {
                EXCEPTION0(ImproperUseException);
                
            }
            if (scalarRange)
            {
                LUT->SetRange(range);
            }
            else 
            {
                LUT->SetRange(scalars->GetRange());
            }
            
        }

        Colors = LUT->MapScalars(scalars, VTK_COLOR_MODE_DEFAULT, 0);
        Colors->Register(NULL);
        Colors->Delete();
    }
    else //scalars not present or we don't want to use them.
    {
        //
        //  Instead of coloring by scalar values, the primitives
        //  will be colored by a constant color:  either the color 
        //  value stored via vtkProperty::SetColor, or the value
        //  stored via vtkProperty::SetEdgeColor. 
        //
        //Colors = NULL;
    }
}


// ****************************************************************************
//  Method: avtSurfaceAndWireframeRenderer::SetProperty 
//
//  Purpose:
//    Sets the vtkProperty for this renderer. vtkProperty has
//    a lot of information about how the primitives should be drawn.
//    
//  Arguments:
//    p          The vtkProperty for this renderer to use.
//   
//  Programmer:  Kathleen Bonnell  
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Eric Brugger, Thu Aug 30 16:27:32 PDT 2001
//    Bump the reference count for the property so that it is reference
//    counted properly.
//
// ****************************************************************************

void 
avtSurfaceAndWireframeRenderer::SetProperty(vtkProperty *p)
{
    if (prop != p)
    {
        if (prop != NULL)
        {
            prop->Delete();
        }
        prop = p;
        prop->Register(NULL);
    }
}


// ****************************************************************************
//  Method:  avtSurfaceAndWireframeRenderer::SetLookupTable 
//
//  Purpose:
//    Specify a lookup table to be used for converting scalars to colors. 
//
//  Arguments:  
//    lut        The lookup table for this renderer to use. 
//   
//  Programmer:  Kathleen Bonnell  
//  Creation:    August 16, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void 
avtSurfaceAndWireframeRenderer::SetLookupTable(vtkLookupTable *lut)
{
    if (LUT != lut)
    {
        LUT = lut; 
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = edgesModified[i] = true;
    }
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::ResolveTopololgyOn
//
// Purpose:
//   Turns on the resolution of coincident topology (via polygon offset). 
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::ResolveTopologyOn()
{
    if (!resolveTopology) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = edgesModified[i] = true;
    }
    resolveTopology = true;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::ResolveTopololgyOff
//
// Purpose:
//   Turns off the resolution of coincident topology (via polygon offset). 
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::ResolveTopologyOff()
{
    if (resolveTopology) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = edgesModified[i] = true;
    }
    resolveTopology = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::ScalarVisibilityOff
//
// Purpose:
//   Turns off scalar visibility, which allow the surface to be colored
//   by the input's scalar values.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::ScalarVisibilityOff()
{
    if (scalarVisibility) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = edgesModified[i] = true;
    }
    scalarVisibility = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::ScalarVisibilityOn
//
// Purpose:
//   Turns on scalar visibility, which allow the surface to be colored
//   by the input's scalar values.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::ScalarVisibilityOn()
{
    if (!scalarVisibility) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = edgesModified[i] = true;
    }
    scalarVisibility = true;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfaceVertsOn
//
// Purpose:
//   Turns on the drawing of vertices for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfaceVertsOn()
{
    if (!drawSurfaceVerts) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfaceVerts = true;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfaceVertsOff
//
// Purpose:
//   Turns off the drawing of vertices for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfaceVertsOff()
{
    if (drawSurfaceVerts) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfaceVerts = false;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfaceLinesOn
//
// Purpose:
//   Turns on the drawing of lines for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfaceLinesOn()
{
    if (!drawSurfaceLines) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfaceLines = true;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfaceLinesOff
//
// Purpose:
//   Turns off the drawing of lines for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfaceLinesOff()
{
    if (drawSurfaceLines) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfaceLines = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfaceStripsOn
//
// Purpose:
//   Turns on the drawing of tri-strips for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfaceStripsOn()
{
    if (!drawSurfaceStrips) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfaceStrips = true;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfaceStripsOff
//
// Purpose:
//   Turns off the drawing of tri-strips for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfaceStripsOff()
{
    if (drawSurfaceStrips) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfaceStrips = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfacePolysOn
//
// Purpose:
//   Turns on the drawing of polygons for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfacePolysOn()
{
    if (!drawSurfacePolys) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfacePolys = true;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SurfacePolysOff
//
// Purpose:
//   Turns off the drawing of polygons for surface mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SurfacePolysOff()
{
    if (drawSurfacePolys) 
    {
        for (int i = 0; i < inputs.size(); i++)
            surfaceModified[i] = true;
    }
    drawSurfacePolys = false;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::EdgeLinesOn
//
// Purpose:
//   Turns on the drawing of lines for edges mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::EdgeLinesOn()
{
    if (!drawEdgeLines) 
    {
        for (int i = 0; i < inputs.size(); i++)
            edgesModified[i] = true;
    }
    drawEdgeLines = true;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::EdgeLinesOff
//
// Purpose:
//   Turns off the drawing of lines for edges mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::EdgeLinesOff()
{
    if (drawEdgeLines) 
    {
        for (int i = 0; i < inputs.size(); i++)
            edgesModified[i] = true;
    }
    drawEdgeLines = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::EdgeStripsOn
//
// Purpose:
//   Turns on the drawing of tri-strips for edges mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::EdgeStripsOn()
{
    if (!drawEdgeStrips) 
    {
        for (int i = 0; i < inputs.size(); i++)
            edgesModified[i] = true;
    }
    drawEdgeStrips = true;    
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::EdgeStripsOff
//
// Purpose:
//   Turns off the drawing of tri-strips for edges mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::EdgeStripsOff()
{
    if (drawEdgeStrips) 
    {
        for (int i = 0; i < inputs.size(); i++)
            edgesModified[i] = true;
    }
    drawEdgeStrips = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::EdgePolysOn
//
// Purpose:
//   Turns on the drawing of polygons for edges mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::EdgePolysOn()
{
    if (!drawEdgePolys) 
    {
        for (int i = 0; i < inputs.size(); i++)
            edgesModified[i] = true;
    }
    drawEdgePolys = true;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::EdgePolysOff
//
// Purpose:
//   Turns off the drawing of polygons for edges mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 22, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 13 10:26:49 PDT 2002   
//    Keep track of modifications.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::EdgePolysOff()
{
    if (drawEdgePolys) 
    {
        for (int i = 0; i < inputs.size(); i++)
            edgesModified[i] = true;
    }
    drawEdgePolys = false;    
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::SetScalarRange
//
// Purpose:
//   Sets the scalar range for the LUT to use when creating colors.
//
// Arguments:
//   min       The range minimum value.
//   max       The range maximum value. 
//
// Programmer: Kathleen Bonnell 
// Creation:   Aug 21, 2001 
//
// Modifications:
//   Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//   Tweak the range if min == max.  vtkLookupTables' don't like constant range.
//
//   Kathleen Bonnell, Wed Mar 19 14:26:05 PST 2003 
//   Removed the tweak for range if min == max.  No longer is a problem. 
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SetScalarRange(double min, double max)
{
    scalarRange = true;    
    range[0] = min; 
    range[1] = max; 
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::GlobalLightingOn
//
// Purpose:
//   Sets the ambient and diffuse lighting coefficients such that 
//   lighting is "off". 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 13, 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Aug 14 12:33:59 PDT 2002 
//   Allow the lighting coefficents to be ignored.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::GlobalLightingOn()
{
    if (!ignoreLighting)
    {
        prop->SetAmbient(0.0); 
        prop->SetDiffuse(1.0); 
    }
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::GlobalLightingOff
//
// Purpose:
//   Sets the ambient and diffuse lighting coefficients such that 
//   lighting is "off". 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 13, 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Aug 14 12:33:59 PDT 2002 
//   Allow the lighting coefficents to be ignored.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::GlobalLightingOff()
{
    if (!ignoreLighting)
    {
        prop->SetAmbient(1.0); 
        prop->SetDiffuse(0.0); 
    }
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::GlobalSetAmbientCoefficient
//
// Purpose:
//   Sets the ambient lighting coefficient to the specified value. 
//
// Arguments:
//   amb   The ambient coefficient.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 13, 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Aug 14 12:33:59 PDT 2002 
//   Allow the lighting coefficents to be ignored.
//
//   Kathleen Bonnell, Thu Sep  2 16:24:49 PDT 2004 
//   Set internal globalAmbientCoeff. 
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::GlobalSetAmbientCoefficient(const double amb)
{
    globalAmbientCoeff = amb;
    if (!ignoreLighting)
    {
       prop->SetAmbient(amb);
    }
}


// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::IgnoreLighting
//
// Purpose:
//   Sets the ignoreLighting flag to the specified value. 
//
// Arguments:
//   val       The value. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 14, 2002
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::IgnoreLighting(const bool val)
{
    ignoreLighting = val; 
}

// ****************************************************************************
// Method: avtSurfaceAndWireframeRenderer::LUTColorsChanged
//
// Purpose:
//   Sets the lutColorsChanged flag to the specified value. 
//
// Arguments:
//   val       The value. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 26, 2002
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::LUTColorsChanged(const bool val)
{
    lutColorsChanged = val; 
}


// ****************************************************************************
//  Method: avtSurfaceAndWireframeRenderer::SetSpecularProperties
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
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//
//  Modifications:
//
//    Hank Childs, Fri Oct 29 10:30:22 PDT 2004
//    Do not turn on specular lighting if lighting is off.
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SetSpecularProperties(bool flag, double coeff, 
    double power, const ColorAttribute &color)
{
    if (ignoreLighting)
    {
        if (prop != NULL)
            prop->SetSpecular(0);
        return;
    }
    if (prop != NULL && (drawSurfaceStrips || drawSurfacePolys ||
        prop->GetRepresentation() == VTK_SURFACE))
    {
        prop->SetSpecular(flag ? coeff : 0);
        prop->SetSpecularPower(power);
        int r = color.Red();
        int g = color.Green();
        int b = color.Blue();
        prop->SetSpecularColor(double(r)/255.,
                               double(g)/255.,
                               double(b)/255.);
    }
}


// ****************************************************************************
//  Method: avtSurfaceAndWireframeRenderer::SetSurfaceRepresentation
//
//  Purpose:
//      Sets the property's surface representation.
//
//  Arguments:
//      rep : The new surface representation.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtSurfaceAndWireframeRenderer::SetSurfaceRepresentation(int rep)
{
    if(prop != NULL) 
    {
        int actorRep = prop->GetRepresentation();
        if(rep == 0 && actorRep != VTK_SURFACE && canApplyGlobalRep &&
           (drawSurfaceStrips || drawSurfacePolys)) 
        {
            prop->SetRepresentation(VTK_SURFACE);
            if (!ignoreLighting)
            {
                prop->SetAmbient(globalAmbientCoeff);
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

