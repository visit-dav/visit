/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtSurfaceMapper.C                                //
// ************************************************************************* //

#include <avtSurfaceMapper.h>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>

#include <avtTransparencyActor.h>
#include <ImproperUseException.h>
#include <NoInputException.h>

#include <DebugStream.h>

using std::string;
using std::vector;



// ****************************************************************************
//  Method: avtSurfaceMapper constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtSurfaceMapper::avtSurfaceMapper()
{
    edgeVis = false;
    scalarVis = true;
    lineWidth = 1;
    edgeColor[0] = edgeColor[1] = edgeColor[2] = 0.;
    surfaceColor[0] = surfaceColor[1] = surfaceColor[2] = 1.;
    canApplyGlobalRep = false;
    ignoreLighting = false;
    wireMode = false;
    lut = NULL;
    scalarRange[0] = scalarRange[1] = 0.; 
}


// ****************************************************************************
//  Method: avtSurfaceMapper destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

avtSurfaceMapper::~avtSurfaceMapper()
{
}


// ****************************************************************************
//  Method: avtSurfaceMapper::CustomizeMappers
//
//  Purpose:
//     Sets up vtkMappers/vtkActors differently depending upon whether
//     they are the polys (opaque surface) or the lines (the mesh).
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::CustomizeMappers()
{
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] == NULL)
            continue;


        vtkProperty *prop = actors[i]->GetProperty();
        if (wireMode)
        {
            mappers[i]->ScalarVisibilityOff();
            prop->SetColor(edgeColor);
            prop->SetRepresentationToWireframe();
        }
        else 
        {
            mappers[i]->SetScalarVisibility(scalarVis);
            if (scalarVis && lut != NULL)
            {
                lut->SetRange(scalarRange);
                mappers[i]->SetLookupTable(lut);
                mappers[i]->SetScalarRange(scalarRange);
            }
            prop->SetColor(surfaceColor);
            prop->SetRepresentationToSurface();
            if (edgeVis)
            {
                mappers[i]->SetResolveCoincidentTopologyToPolygonOffset();
                mappers[i]->SetResolveCoincidentTopologyLineOffsetParameters(1., 1.);
                prop->EdgeVisibilityOn();
                prop->SetEdgeColor(edgeColor);
                prop->SetLineWidth(lineWidth);
            }
            else
            {
                mappers[i]->SetResolveCoincidentTopologyToOff();
                prop->EdgeVisibilityOff();
            }
        }
        if (ignoreLighting)
        {
            prop->SetAmbient(1.0);
            prop->SetDiffuse(0.0);
        }
        else 
        {
            prop->SetAmbient(0.0);
            prop->SetDiffuse(1.0);
        }
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetEdgeVisibility
//
//  Purpose:
//      Toggles edge visibility.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSurfaceMapper::SetEdgeVisibility(bool val)
{
    if (edgeVis != val)
    {
        edgeVis = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] == NULL)
                continue;
            actors[i]->GetProperty()->SetEdgeVisibility(edgeVis);
            if (edgeVis)
            {
                mappers[i]->SetResolveCoincidentTopologyToPolygonOffset();
                mappers[i]->SetResolveCoincidentTopologyLineOffsetParameters(1., 1.);
            }
            else
                mappers[i]->SetResolveCoincidentTopologyToOff();
        }
    }
}


// ****************************************************************************
//  Method: ColorsAreDifferent
//
//  Purpose:
//     Helper method for comparing rgb colors.
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

bool
ColorsAreDifferent(double a[3], double b[3])
{
   return ((a[0] != b[0]) ||
           (a[1] != b[1]) ||
           (a[2] != b[2]));
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetEdgeColor
//
//  Purpose:
//     Sets color for edges.
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::SetEdgeColor(double rgb[3])
{
    if (ColorsAreDifferent(edgeColor, rgb))
    {
        edgeColor[0] = rgb[0];
        edgeColor[1] = rgb[1];
        edgeColor[2] = rgb[2];
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL)
            {
                actors[i]->GetProperty()->SetEdgeColor(rgb);
            }
        }
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetLineWidth
//
//  Purpose:
//     Sets the line width.
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::SetLineWidth(int lw)
{
    if (lineWidth != lw)
    {
        lineWidth = lw;
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL)
            {
                actors[i]->GetProperty()->SetLineWidth(lw);
            }
        }
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetSurfaceColor
//
//  Purpose:
//     Sets color to be used for the opaque surface.
//
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::SetSurfaceColor(double rgb[3])
{
    if (ColorsAreDifferent(surfaceColor, rgb))
    {
        surfaceColor[0] = rgb[0];
        surfaceColor[1] = rgb[1];
        surfaceColor[2] = rgb[2];
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL)
            {
                actors[i]->GetProperty()->SetColor(rgb);
            }
        }
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::CanApplyGlobalRepresentation
//
//  Purpose:
//
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::CanApplyGlobalRepresentation(bool val)
{
    canApplyGlobalRep = val;
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetRepresentation
//
//  Purpose:
//    toggles wireframe mode.
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//    Kathleen Biagas, Fri May 25 12:09:23 PDT 2018
//    Ensure lighting settings are correct for the representation mode.
//
// ****************************************************************************

void
avtSurfaceMapper::SetRepresentation(bool val)
{
    if (wireMode != val)
    {
        wireMode = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] == NULL)
                continue;

            if (wireMode)
            {
                mappers[i]->ScalarVisibilityOff();
                vtkProperty *prop = actors[i]->GetProperty();
                prop->SetColor(edgeColor);
                prop->SetRepresentationToWireframe();
                prop->SetAmbient(1.);
                prop->SetDiffuse(0.);
            }
            else
            {
                mappers[i]->SetScalarVisibility(scalarVis);
                vtkProperty *prop = actors[i]->GetProperty();
                prop->SetColor(surfaceColor);
                prop->SetRepresentationToSurface();
                if (GetLighting())
                {
                    prop->SetAmbient(GetGlobalAmbientCoefficient());
                    prop->SetDiffuse(1.);
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetSurfaceRepresentation
//
//  Purpose:
//     Global setting of representation: surface, wireframe or points.
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::SetSurfaceRepresentation(int rep)
{
    if (canApplyGlobalRep)
    {
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] == NULL)
                continue;

            vtkProperty *prop = actors[i]->GetProperty();
            int actorRep = prop->GetRepresentation();
            if(rep == 0 && actorRep != VTK_SURFACE)
            {
                prop->SetRepresentation(VTK_SURFACE);
                if (GetLighting())
                {
                    prop->SetAmbient(GetGlobalAmbientCoefficient());
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


// ****************************************************************************
//  Method: avtSurfaceMapper::SetIgnoreLighting
//
//  Purpose:
//    Sets the ignoreLighting flag.  Sets ambient/diffuse as appropriate.
//
// 
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
// ****************************************************************************

void
avtSurfaceMapper::SetIgnoreLighting(bool val)
{
    if (ignoreLighting != val)
    {
        ignoreLighting = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] == NULL)
                continue;
            vtkProperty *prop = actors[i]->GetProperty();

            if (ignoreLighting)
            {
                prop->SetAmbient(1.0);
                prop->SetDiffuse(0.0);
            }
            else 
            {
                prop->SetAmbient(0.0);
                prop->SetDiffuse(1.0);
            }
        }
    }
}

// ****************************************************************************
//  Method: avtSurfaceMapper::SetLookupTable
//
//  Purpose:
//    Causes the mapper to use the specified lookup table. 
//
//  Arguments:
//    lut    The new value for the lookup table. 
//
//  Notes:
//    This method must be called after the call to CustomizeMappers, otherwise
//    the default LUT set there overrides this setup.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 18, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtSurfaceMapper::SetLookupTable(vtkLookupTable *_lut)
{
    if (lut != _lut)
    {
        lut = _lut;
        lut->SetRange(scalarRange);
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->SetLookupTable(lut);
                mappers[i]->SetScalarRange(scalarRange);
            }
        }

        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetScalarVisibility
//
//  Purpose:
//      Toggles scalar visibility.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSurfaceMapper::SetScalarVisibility(bool val)
{
    if (scalarVis != val)
    {
        scalarVis = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->SetScalarVisibility(scalarVis);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::SetScalarRange
//
//  Purpose:
//      Sets the scalar range used by the mappers and LUT.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSurfaceMapper::SetScalarRange(double smin, double smax)
{
    scalarRange[0] = smin;
    scalarRange[1] = smax;
    if (lut != NULL)
    {
       lut->SetRange(scalarRange); 
       for (int i = 0; i < nMappers; ++i)
           if (mappers[i] != NULL)
               mappers[i]->SetScalarRange(scalarRange);
    }
}


// ****************************************************************************
//  Method: avtSurfaceMapper::GetDataRange
//
//  Arguments:
//    rmin      The minimum in the range.
//    rmax      The maximum in the range.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 18, 2016 
//
//  Modifications:
//
// ****************************************************************************

bool
avtSurfaceMapper::GetDataRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmax = 1.;
        return false;
    }

    avtMapper::GetRange(rmin, rmax);
    return true;
}


// ****************************************************************************
//  Method: avtSurfaceMapper::GetCurrentDataRange
//
//  Arguments:
//    rmin      The minimum in the range.
//    rmax      The maximum in the range.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 18, 2016 
//
//  Modifications:
//
// ****************************************************************************

bool
avtSurfaceMapper::GetCurrentDataRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmax = 1.;
        return false;
    }

    avtMapper::GetCurrentRange(rmin, rmax);
    return true;
}


// ****************************************************************************
//  Method: avtSurfaceMapper::NotifyTransparencyActor
//
//  Purpose:
//    Informs the transparency actor that its input has changed.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSurfaceMapper::NotifyTransparencyActor()
{
    if (transparencyActor != NULL)
    {
        transparencyActor->InputWasModified(transparencyIndex);
        transparencyActor->InvalidateTransparencyCache();
    }
}



