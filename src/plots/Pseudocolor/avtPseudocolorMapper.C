// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtPseudocolorMapper.C                              //
// ************************************************************************* //

#include <avtPseudocolorMapper.h>

#include <avtTransparencyActor.h>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPointGlyphMapper.h>
#include <vtkProperty.h>
#include <vtkVertexFilter.h>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtPseudocolorMapper constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   Autust 24, 2016
//
//  Modifications:
//    Kathleen Biagas, Tue Nov  5 11:58:15 PST 2019
//    Added variables for handling points.
//
// ****************************************************************************

avtPseudocolorMapper::avtPseudocolorMapper() : avtVariableMapper()
{
    drawSurface   = true;

    drawWireframe = false;
    wireframeColor[0] = wireframeColor[1] = wireframeColor[2] = 0.;

    drawPoints    = false;
    pointsColor[0] = pointsColor[1] = pointsColor[2] = 0.;
    glyphType = Point;
    colorByScalar = true;
    scale = 0.2;
    scalingVarName = "";
    coloringVarName = "";
    pointSize = 2;
}


// ****************************************************************************
//  Method: avtPseudocolorMapper destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
// ****************************************************************************

avtPseudocolorMapper::~avtPseudocolorMapper()
{
}


bool
LabelIsFromPCPlot(string &l)
{
    return (!l.empty() && l.size() > 3 && l.compare(0, 3, string("pc_")) == 0);
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::CreateActorMapperPairs
//
//  Purpose:  Creates the appropriate mapper type based on current settings
//            and sets up the input->mapper->actor pipeline.
//
//            For points, use vtkPointGlyphMapper, otherwise vtkDataSetMapper.
//
//  Arguments:
//    children  The input datasets.
//
//  Programmer: Kathleen Biagas
//  Creation:   November 5, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::CreateActorMapperPairs(vtkDataSet **children)
{
    int topoDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    int nDS = nMappers;
    nMappers = 0;
    if (!labels.empty())
    {
        nMappers = nDS;
    }
    else if (topoDim != 0)
    {
        if (drawSurface || drawWireframe)
            nMappers += nDS;
        if (drawPoints)
            nMappers += nDS;
    }
    else
    {
        nMappers = nDS;
    }

    mappers  = new vtkDataSetMapper*[nMappers];
    actors   = new vtkActor*[nMappers];

    int mi = 0;
    vector<int> usedDS;
    if (topoDim != 0 && (drawSurface || drawWireframe))
    {
        for (int i = 0; i < nDS; ++i)
        {
            if (children[i] == NULL || children[i]->GetNumberOfCells() <= 0)
            {
                usedDS.push_back(i);
                mappers[mi] = NULL;
                actors[mi]  = NULL;
                ++mi;
                continue;
            }
            if (labels.empty() ||  !LabelIsFromPCPlot(labels[i]) ||
                (labels[i].compare(0, 10, string("pc_points_")) != 0))
            {
                usedDS.push_back(i);
                mappers[mi] = vtkDataSetMapper::New();
                mappers[mi]->SetInputData(children[i]);
                actors[mi]  = vtkActor::New();
                actors[mi]->SetMapper(mappers[mi]);
                ++mi;
            }
        }
    }

    if ((mi < nMappers))
    {
        for (int i = 0; i < nDS; ++i)
        {
            if (std::find(usedDS.begin(), usedDS.end(), i) != usedDS.end() && !drawPoints)
                continue;

            if (children[i] == NULL || children[i]->GetNumberOfCells() <= 0)
            {
                mappers[mi] = NULL;
                actors[mi]  = NULL;
                ++mi;
                continue;
            }
            bool pointsLabel = (!labels.empty() && LabelIsFromPCPlot(labels[i]) &&
                              (labels[i].compare(0, 10, string("pc_points_")) == 0));
            mappers[mi] = (vtkDataSetMapper*)vtkPointGlyphMapper::New();
            if (topoDim == 0 || pointsLabel)
            {
                mappers[mi]->SetInputData(children[i]);
            }
            else
            {
                vtkNew<vtkVertexFilter> vertexFilter;
                vertexFilter->SetInputData(children[i]);
                vertexFilter->VertexAtPointsOn();
                vertexFilter->Update();
                mappers[mi]->SetInputData(vertexFilter->GetOutput());
            }
            actors[mi]  = vtkActor::New();
            actors[mi]->SetMapper(mappers[mi]);
            ++mi;
        }
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::CustomizeMappers
//
//  Purpose:
//    Adds our flags to the vtk mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//    Kathleen Biagas, Tue Nov  5 12:16:41 PST 2019
//    Call CustomizeMappersInternal.
//
// ****************************************************************************

void
avtPseudocolorMapper::CustomizeMappers()
{
    avtVariableMapper::CustomizeMappers();
    CustomizeMappersInternal();
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::CustomizeMappersInternal
//
//  Purpose:
//    Adds our flags to the vtk mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   November 5, 2019
//
// ****************************************************************************

void
avtPseudocolorMapper::CustomizeMappersInternal(bool invalidateTransparency)
{
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] == NULL)
            continue;

        vtkProperty *prop = actors[i]->GetProperty();
        prop->SetPointSize(pointSize);

        if (mappers[i]->IsA("vtkPointGlyphMapper"))
        {
            vtkPointGlyphMapper *pgm = (vtkPointGlyphMapper*)mappers[i];
            pgm->SetSpatialDimension(
                GetInput()->GetInfo().GetAttributes().GetSpatialDimension());
            pgm->SetLookupTable(lut);
            pgm->SetGlyphType(glyphType);
            if (drawPoints)
               pgm->ColorByScalarOff();
            else
               pgm->ColorByScalarOn(coloringVarName);
            prop->SetColor(pointsColor);
            if(glyphType == Point)
            {
                prop->SetRepresentationToPoints();
            }
            else
            {
                prop->SetRepresentationToSurface();
            }
            if (dataScaling)
            {
                // need to call ScaleByVar rather than DataScalingOn, because
                // scalingVarDim may not yet have been set correctly.
                ScaleByVar(scalingVarName);
            }
            else
            {
                DataScalingOff();
            }

            SetScale(scale);
        }
        else
        {
            if (drawSurface)
            {
                mappers[i]->ScalarVisibilityOn();
                prop->SetRepresentationToSurface();
                if (drawWireframe)
                {
                    if (labels.empty() || !LabelIsFromPCPlot(labels[i]) ||
                        (labels[i].compare(0, 8, string("pc_surf_")) == 0))
                    {
                        prop->EdgeVisibilityOn();
                        prop->SetEdgeColor(wireframeColor);
                    }
                }
            }
            else if (drawWireframe)
            {
                mappers[i]->ScalarVisibilityOff();
                prop->SetRepresentationToWireframe();
                prop->SetColor(wireframeColor);
                prop->EdgeVisibilityOff();
            }
        }
    }

    if(invalidateTransparency && opacity < 1.0 && transparencyActor != NULL)
    {
        InvalidateTransparencyCache();
        transparencyActor->InputWasModified(transparencyIndex);
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetDrawSurface
//
//  Purpose:
//     Toggles the surface representation mode
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//   Kathleen Biagas, Tue Nov  5 12:04:16 PST 2019
//   Call CustomizeMappersInternal to ensure dependent settings are correct.
//
// ****************************************************************************

void
avtPseudocolorMapper::SetDrawSurface(bool val)
{
    if (drawSurface != val)
    {
        drawSurface = val;
        CustomizeMappersInternal(drawWireframe);
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetDrawWireframe
//
//  Purpose:
//     Toggles the Wireframe representation mode
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//   Kathleen Biagas, Tue Nov  5 12:04:16 PST 2019
//   Call CustomizeMappersInternal to ensure dependent settings are correct.
//
// ****************************************************************************

void
avtPseudocolorMapper::SetDrawWireframe(bool val)
{
    if (drawWireframe != val)
    {
        drawWireframe = val;
        CustomizeMappersInternal(drawSurface);
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetDrawPoints
//
//  Purpose:
//     Toggles the Points representation mode
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//   Kathleen Biagas, Tue Nov  5 12:04:16 PST 2019
//   Call CustomizeMappersInternal to ensure dependent settings are correct.
//
// ****************************************************************************

void
avtPseudocolorMapper::SetDrawPoints(bool val)
{
    if (drawPoints != val)
    {
        drawPoints = val;
        CustomizeMappersInternal();
    }
}


// ****************************************************************************
//  Method: ColorsAreDifferent
//
//  Purpose:
//     Helper method for comparing rgb colors.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
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
//  Method: avtPseudocolorMapper::SetWireframeColor
//
//  Purpose:
//     Sets color to be used for the wirefame mode
//
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//   Kathleen Biagas, Tue Nov  5 12:04:16 PST 2019
//   Call CustomizeMappersInternal to ensure dependent settings are correct.
//
// ****************************************************************************

void
avtPseudocolorMapper::SetWireframeColor(double rgb[3])
{
    if (ColorsAreDifferent(wireframeColor, rgb))
    {
        wireframeColor[0] = rgb[0];
        wireframeColor[1] = rgb[1];
        wireframeColor[2] = rgb[2];
        CustomizeMappersInternal();
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetPointsColor
//
//  Purpose:
//     Sets color to be used for the wirefame mode
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//   Kathleen Biagas, Tue Nov  5 12:04:16 PST 2019
//   Call CustomizeMappersInternal to ensure dependent settings are correct.
//
// ****************************************************************************

void
avtPseudocolorMapper::SetPointsColor(double rgb[3])
{
    if (ColorsAreDifferent(pointsColor, rgb))
    {
        pointsColor[0] = rgb[0];
        pointsColor[1] = rgb[1];
        pointsColor[2] = rgb[2];
        CustomizeMappersInternal();
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::ColorByScalarOn
//
//  Purpose:
//    Tells the mapper to color by the given scalar.
//
//  Arguments:
//    sn          The name of the scalar var.
//
//  Programmer:   Kathleen Biagas
//  Creation:     November 5, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::ColorByScalarOn(const string &sn)
{
    colorByScalar = true;
    coloringVarName = sn;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper") &&
            !drawPoints)
                ((vtkPointGlyphMapper*)mappers[i])->ColorByScalarOn(sn);
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::ColorByScalarOff
//
//  Purpose:
//    Tells the mapper to color all of the points the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Biagas
//  Creation:     November 5, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::ColorByScalarOff()
{
    colorByScalar = false;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
            ((vtkPointGlyphMapper*)mappers[i])->ColorByScalarOff();
    }
}


// ****************************************************************************
//  Method: avtPointMapper::ScaleByVar
//
//  Purpose:
//    Turns on data scaling for the glyph portion of this mapper.
//
//  Arguments:
//    sname     The name of the variable to be used for scaling.
//
//  Programmer: Kathleen Biagas
//  Creation:   November 5, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::ScaleByVar(const std::string &sname)
{
    if (sname == "" || sname == "\0")
    {
        DataScalingOff();
        return;
    }
    scalingVarName = sname;
    int scalingVarDim = 1;
    if (*(GetInput()) != NULL &&
        GetInput()->GetInfo().GetAttributes().ValidVariable(sname.c_str()))
    {
        scalingVarDim = GetInput()->GetInfo().GetAttributes().
                        GetVariableDimension(sname.c_str());
    }
    DataScalingOn(scalingVarName, scalingVarDim);
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::DataScalingOff
//
//  Purpose:
//
//  Programmer: Kathleen Biagas
//  Creation:   November 5, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::DataScalingOff(void)
{
    dataScaling = false;
    scalingVarName = "";

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
            ((vtkPointGlyphMapper*)mappers[i])->DataScalingOff();
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::DataScalingOn
//
//  Purpose:
//    Turns on the appropriate type of data scaling based on the dimension
//    of the variable to be used in scaling.
//
//  Arguments:
//    sname     The name of the scalars to be used for scaling the glyphs.
//    varDim    The dimension of the var to be used for scaling.
//
//  Programmer: Kathleen Biagas
//  Creation:   November 5, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::DataScalingOn(const string &sname, int varDim)
{
    dataScaling = true;
    scalingVarName = sname;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
            ((vtkPointGlyphMapper*)mappers[i])->DataScalingOn(sname, varDim);
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetScale
//
//  Purpose:
//    Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Kathleen Biagas
//  Creation:    November 5, 2019
//
// ****************************************************************************

void
avtPseudocolorMapper::SetScale(double s)
{
    scale = s;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
            ((vtkPointGlyphMapper*)mappers[i])->SetScale(scale);
    }
}


// ****************************************************************************
// Method: avtPseudocolorMapper::SetGlyphType
//
// Purpose:
//   This method sets the point glyph type.
//
// Programmer: Kathleen Biagas
// Creation:   November 5, 2019
//
// Modifications:
//
// ****************************************************************************

void
avtPseudocolorMapper::SetGlyphType(GlyphType type)
{
    if (glyphType != type)
    {
        glyphType = type;
        CustomizeMappersInternal();
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetPointSize
//
//  Purpose:
//     Sets the point size.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 10, 2019
//
// ****************************************************************************

void
avtPseudocolorMapper::SetPointSize(double ps)
{
    if (pointSize != ps)
    {
        pointSize = ps;
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL)
                actors[i]->GetProperty()->SetPointSize(pointSize);
        }
    }
}


// ****************************************************************************
// Method: avtPseudocolorMapper::SetFullFrameScaling
//
// Purpose:
//   Sets a fullframe scale factor that can be used by the mapper to compensate
//   for the stretching that fullframe mode performs on geometry.
//
// Arguments:
//   useScale : True if the scale is used.
//   s        : The fullframe scale vector.
//
// Returns:    True if any vtk mappers use the scale.
//
// Note:  Taken from avtPointMapper.
//
// Programmer: Kathleen Biagas
// Creation:   November 5, 2019
//
// Modifications:
//
// ****************************************************************************

bool
avtPseudocolorMapper::SetFullFrameScaling(bool useScale, const double *s)
{
    bool retval = false;
    for (int i = 0; i < nMappers && !retval; ++i)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
           retval = ((vtkPointGlyphMapper*)mappers[i])->
                SetFullFrameScaling(useScale, s);
    }
    return retval;
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetLabels
//
//  Purpose:
//     Saves the labels coming from the input data tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   November 5, 2019
//
// ****************************************************************************

void
avtPseudocolorMapper::SetLabels(vector<string> &l, bool fromTree)
{
    if (!fromTree)
        return;

    labels = l;
}

