// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtHistogramMapper.C                           //
// ************************************************************************* //

#include <avtHistogramMapper.h>
#include <avtTransparencyActor.h>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>


// ****************************************************************************
//  Method: avtHistogramMapper constructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtHistogramMapper::avtHistogramMapper() 
{
    edgeVis = true;
    edgeColor[0]    = edgeColor[1]    = edgeColor[2]    = 0.; // black
    surfaceColor[0] = surfaceColor[1] = surfaceColor[2] = 1.; // white
    opacity = 1.0;
    lineWidth = 1.0;
}


// ****************************************************************************
//  Method: avtHistogramMapper destructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtHistogramMapper::~avtHistogramMapper()
{
}


// ****************************************************************************
//  Method: avtHistogramMapper::SetSurfaceRepresentation
//
//  Purpose:
//      Sets the drawable's surface representation.
//
//  Arguments:
//      rep : The new surface representation.
//
//  Programmer: Kathleen Biagas 
//  Creation:   May 11, 2016
//
// ****************************************************************************

void
avtHistogramMapper::SetSurfaceRepresentation(int globalRep)
{
  // This is global rep, don't allow it to set anything for now.
}


// ****************************************************************************
//  Method: avtHistogramMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the surface and wireframe
//      mapper to make any calls take effect that were made before the base
//      class set up the vtk mappers.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::CustomizeMappers()
{
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] == NULL)
            continue;

        mappers[i]->SetScalarVisibility(false);
        vtkProperty *prop = actors[i]->GetProperty();
        prop->SetColor(surfaceColor);
        prop->SetOpacity(opacity);
        prop->SetAmbient(1.);
        prop->SetDiffuse(0.);
        prop->SetLineWidth(lineWidth);

        if (edgeVis)
        {
            prop->EdgeVisibilityOn();
            prop->SetEdgeColor(edgeColor);
        }
        else 
        {
            prop->EdgeVisibilityOff();
        }
    }
}


// ****************************************************************************
//  Method: avtHistogramMapper::SetEdgeVisibility
//
//  Purpose:
//      Toggles edge visibility.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::SetEdgeVisibility(bool val)
{
    if (edgeVis != val)
    {
        edgeVis = val;
        CustomizeMappers();
    }
}


// ****************************************************************************
//  Method: avtHistogramMapper::SetColor
//
//  Purpose:
//      Sets the color for the surface.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::SetColor(double rgb[3])
{
    if (surfaceColor[0] != rgb[0] ||
        surfaceColor[1] != rgb[1] ||
        surfaceColor[2] != rgb[2] ) 
    {
        surfaceColor[0] = rgb[0];
        surfaceColor[1] = rgb[1];
        surfaceColor[2] = rgb[2];
        CustomizeMappers();
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtHistogramMapper::SetEdgeColor
//
//  Purpose:
//      Sets the color for the edges.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::SetEdgeColor(double rgb[3])
{
    if (edgeColor[0] != rgb[0] ||
        edgeColor[1] != rgb[1] ||
        edgeColor[2] != rgb[2] ) 
    {
        edgeColor[0] = rgb[0];
        edgeColor[1] = rgb[1];
        edgeColor[2] = rgb[2];
        CustomizeMappers();
        NotifyTransparencyActor();
    }
}

// ****************************************************************************
//  Method: avtHistogramMapper::SetOpacity
//
//  Purpose:
//      Sets the opacity.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::SetOpacity(double val)
{
    if (opacity != val)
    {
        opacity = val;
        CustomizeMappers();
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtHistogramMapper::SetLineWidth
//
//  Purpose:
//      Sets the lineWidth.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::SetLineWidth(int lw)
{
    if (lineWidth != lw)
    {
        lineWidth = lw;
        CustomizeMappers();
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtHistogramMapper::NotifyTransparencyActor
//
//  Purpose:
//    Informs the transparency actor that its input has changed.
//
//  Programmer: Kathleen Biagas
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtHistogramMapper::NotifyTransparencyActor()
{
    if (transparencyActor != NULL)
        transparencyActor->InputWasModified(transparencyIndex);
}
