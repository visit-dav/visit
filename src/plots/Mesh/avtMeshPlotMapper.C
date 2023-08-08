// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtMeshPlotMapper.C
// ****************************************************************************

#include <avtMeshPlotMapper.h>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPointGlyphMapper.h>
#include <vtkProperty.h>

#include <avtTransparencyActor.h>

#include <DebugStream.h>
#include <NoInputException.h>


#include <vector>
#include <string>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtMeshPlotMapper constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
//  Modifications:
//    Kathleen Biagas, Wed Apr  3 16:11:45 PDT 2019
//    Added pointSize.
//
// ****************************************************************************

avtMeshPlotMapper::avtMeshPlotMapper()
{
    lineWidth = 1;
    linesColor[0] = linesColor[1] = linesColor[2] = 0.;
    polysColor[0] = polysColor[1] = polysColor[2] = 1.;
    opacity = 1.;
    surfaceVis = true;
    glyphType = Point;
    // set by PointSize match its default
    scale = 0.05;
    scalingVarName = "";
    // set by PointSizePixels
    pointSize = 2;
    autoSize = true;
}


// ****************************************************************************
//  Method: avtMeshPlotMapper destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
// ****************************************************************************

avtMeshPlotMapper::~avtMeshPlotMapper()
{
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::CreateActorMapperPairs
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
//  Creation:   June 10, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::CreateActorMapperPairs(vtkDataSet **children)
{
    int topoDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();

    mappers  = new vtkDataSetMapper*[nMappers];
    actors   = new vtkActor*[nMappers];

    for (int i = 0; i < nMappers; ++i)
    {
        if (children[i] == NULL || children[i]->GetNumberOfCells() <= 0)
        {
            mappers[i] = NULL;
            actors[i]  = NULL;
            continue;
        }
        if (topoDim != 0 && (labels.empty() ||
            (labels[i].compare(0, 12, string("mesh_points_")) != 0)))
        {
            mappers[i] = vtkDataSetMapper::New();
            mappers[i]->SetInputData(children[i]);
            actors[i]  = vtkActor::New();
            actors[i]->SetMapper(mappers[i]);
        }
        else
        {
            mappers[i] = (vtkDataSetMapper*)vtkPointGlyphMapper::New();
            mappers[i]->SetInputData(children[i]);
            actors[i]  = vtkActor::New();
            actors[i]->SetMapper(mappers[i]);
        }
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::CustomizeMappers
//
//  Purpose:
//     Sets up vtkMappers/vtkActors differently depending upon whether
//     they are the polys (opaque surface) or the lines (the mesh).
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
//  Modifications:
//    Kathleen Biagas, Wed Apr  3 16:11:45 PDT 2019
//    Added pointSize.
//
//    Kathleen Biagas, Thu Dec 15, 2023
//    Ensure opacity is set for all actors. 
//
// ****************************************************************************

void
avtMeshPlotMapper::CustomizeMappers()
{
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] == NULL)
            continue;

        mappers[i]->SetScalarVisibility(false);
        vtkProperty *prop = actors[i]->GetProperty();
        prop->SetOpacity(opacity);

        if (mappers[i]->IsA("vtkPointGlyphMapper"))
        {
            vtkPointGlyphMapper *pgm = (vtkPointGlyphMapper*)mappers[i];
            pgm->SetSpatialDimension(
                GetInput()->GetInfo().GetAttributes().GetSpatialDimension());
            pgm->SetGlyphType(glyphType);
            pgm->ColorByScalarOff();
            prop->SetPointSize(pointSize);
            prop->SetColor(linesColor);
            prop->SetAmbient(0.);
            prop->SetDiffuse(1.);
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
        else if (labels[i].compare(0, 11, string("mesh_lines_")) == 0)
        {
            prop->SetAmbient(1.);
            prop->SetDiffuse(0.);
            prop->SetColor(linesColor);
            prop->SetLineWidth(lineWidth);
            prop->SetPointSize(pointSize);
        }
        else if (labels[i].compare(0, 11, string("mesh_polys_")) == 0)
        {
            actors[i]->SetVisibility(surfaceVis);
            prop->SetAmbient(1.);
            prop->SetDiffuse(0.);
            prop->SetColor(polysColor);
            // this is to satisfy avtTransparencyActor which does
            // not differentiate properties from different actors
            prop->SetLineWidth(lineWidth);
        }
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetSurfaceVisibility
//
//  Purpose:
//      Toggles surface visibility.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 16, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::SetSurfaceVisibility(bool val)
{
    if (surfaceVis != val)
    {
        surfaceVis = val;
        vector<int> v(nMappers);
        for (int i = 0; i < nMappers; ++i)
        {
            v[i] = 1;
            if (actors[i] != NULL)
            {
                if (labels[i].compare(0, 11, string("mesh_polys_")) == 0)
                {
                    actors[i]->SetVisibility(surfaceVis);
                    v[i] = surfaceVis ? 1 : 0;
                }
            }
        }
        // need to tell the transparencyActor that visibility changed
        if (transparencyActor != NULL)
        {
            transparencyActor->ReplaceActorVisibility(transparencyIndex, v);
            NotifyTransparencyActor();
        }

        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->Modified();
                actors[i]->Modified();
            }
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
//  Creation:   June 30, 2016
//
// ****************************************************************************

static bool
ColorsAreDifferent(double a[3], double b[3])
{
   return ((a[0] != b[0]) ||
           (a[1] != b[1]) ||
           (a[2] != b[2]));
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetMeshColor
//
//  Purpose:
//     Sets color for mesh lines.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
//  Modifications:
//    Kathleen Biagas, Thu Jun 18 09:12:22 PDT 2020
//    Fix ambiguous if statement.
//
// ****************************************************************************

void
avtMeshPlotMapper::SetMeshColor(double rgb[3])
{
    if (ColorsAreDifferent(linesColor, rgb))
    {
        linesColor[0] = rgb[0];
        linesColor[1] = rgb[1];
        linesColor[2] = rgb[2];
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL && (!labels.empty() &&
                (labels[i].compare(0, 11, string("mesh_lines_")) == 0) ||
                (labels[i].compare(0, 12, string("mesh_points_")) == 0)))
            {
                actors[i]->GetProperty()->SetColor(rgb);
            }
        }
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetSurfaceColor
//
//  Purpose:
//     Sets color to be used for the opaque surface.
//
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
// ****************************************************************************

void
avtMeshPlotMapper::SetSurfaceColor(double rgb[3])
{
    if (ColorsAreDifferent(polysColor, rgb))
    {
        polysColor[0] = rgb[0];
        polysColor[1] = rgb[1];
        polysColor[2] = rgb[2];
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL && !labels.empty() &&
                labels[i].compare(0, 11, string("mesh_polys_")) == 0)
            {
                actors[i]->GetProperty()->SetColor(rgb);
            }
        }
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetOpacity
//
//  Purpose:
//     Sets the opacity.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
// ****************************************************************************

void
avtMeshPlotMapper::SetOpacity(double val)
{
    if (opacity != val)
    {
        opacity = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (actors[i] != NULL)
            {
                actors[i]->GetProperty()->SetOpacity(opacity);
            }
        }
        NotifyTransparencyActor();
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetLineWidth
//
//  Purpose:
//     Sets the line width.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
// ****************************************************************************

void
avtMeshPlotMapper::SetLineWidth(int lw)
{
    if (lineWidth != lw)
    {
        lineWidth = lw;
        for (int i = 0; i < nMappers; ++i)
        {
            // should not need to set this for polys, but avtTransparencyActor
            // doesn't handle properties correctly in this instance, so set
            // line width for all actors
            //if (actors[i] != NULL && !labels.empty() && labels[i] == "lines_")

            if (actors[i] != NULL)
            {
                actors[i]->GetProperty()->SetLineWidth(lw);
            }
        }
        NotifyTransparencyActor();
    }
}

// ****************************************************************************
//  Method: avtMeshPlotMapper::SetAutoSize
//
//  Purpose:
//     Sets the flag indicating point size should be automagically computed.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 7, 2023
//
// ****************************************************************************

void
avtMeshPlotMapper::SetAutoSize(bool val)
{
    if(autoSize != val)
    {
        autoSize = val;
        SetPointSize(pointSize);
        SetScale(scale);
    }
}

// ****************************************************************************
//  Method: avtMeshPlotMapper::SetPointSize
//
//  Purpose:
//     Sets the point size.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 3, 2019
//
// ****************************************************************************

void
avtMeshPlotMapper::SetPointSize(double ps)
{
    pointSize = ps;
    double tmp = pointSize;
    bool psChanged=false;
    if(autoSize)
    {
        tmp = ComputePointSize(false);
    }
    for (int i = 0; i < nMappers; ++i)
    {
        if (actors[i] != NULL)
        {
            psChanged = (actors[i]->GetProperty()->GetPointSize() != tmp);
            actors[i]->GetProperty()->SetPointSize(tmp);
        }
    }
    if(psChanged)
        NotifyTransparencyActor();
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::NotifyTransparencyActor
//
//  Purpose:
//    Informs the transparency actor that its input has changed.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::NotifyTransparencyActor()
{
    if (transparencyActor != NULL)
    {
        transparencyActor->InputWasModified(transparencyIndex);
        transparencyActor->InvalidateTransparencyCache();
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::SetLabels
//
//  Purpose:
//     Saves the labels coming from the input data tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 30, 2016
//
// ****************************************************************************

void
avtMeshPlotMapper::SetLabels(vector<string> &l, bool fromTree)
{
    if (!fromTree)
        return;

    if (l.empty())
    {
        debug1 << "avtMeshPlotMapper expects labels in the input tree." << endl;
        EXCEPTION0(NoInputException);
    }
    else
    {
       labels = l;
    }
}


// ****************************************************************************
//  Method: avtMeshPlotMapper::ActorIsShiftable
//
//  Purpose:
//     Only the lines portion of the Mesh plot should be shifted.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 26, 2018
//
// ****************************************************************************

bool
avtMeshPlotMapper::ActorIsShiftable(int i)
{
    bool shiftable = true;
    if (!labels.empty() && i >= 0 && i < labels.size())
        shiftable &= (labels[i].compare(0, 11, string("mesh_lines_")) == 0);
    return shiftable;
}


// ****************************************************************************
// Method: avtMeshPlotMapper::SetFullFrameScaling
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
// Creation:   June 10, 2020
//
// Modifications:
//
// ****************************************************************************

bool
avtMeshPlotMapper::SetFullFrameScaling(bool useScale, const double *s)
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
//  Method: avtMeshPlotMapper::ScaleByVar
//
//  Purpose:
//    Turns on data scaling for the glyph portion of this mapper.
//
//  Arguments:
//    sname     The name of the variable to be used for scaling.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 10, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::ScaleByVar(const std::string &sname)
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
//  Method: avtMeshPlotMapper::DataScalingOff
//
//  Purpose:
//
//  Programmer: Kathleen Biagas
//  Creation:   June 10, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::DataScalingOff(void)
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
//  Method: avtMeshPlotMapper::DataScalingOn
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
//  Creation:   June 10, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::DataScalingOn(const string &sname, int varDim)
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
//  Method: avtMeshPlotMapper::SetScale
//
//  Purpose:
//    Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Kathleen Biagas
//  Creation:    June 10, 2020
//
// ****************************************************************************

void
avtMeshPlotMapper::SetScale(double s)
{
    scale = s;
    double tmp = scale;
    if (autoSize)
    {
        tmp = ComputePointSize(true);
    }

    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
            ((vtkPointGlyphMapper*)mappers[i])->SetScale(tmp);
    }
}


// ****************************************************************************
// Method: avtMeshPlotMapper::SetGlyphType
//
// Purpose:
//   This method sets the point glyph type.
//
// Programmer: Kathleen Biagas
// Creation:   June 10, 2020
//
// Modifications:
//
// ****************************************************************************

void
avtMeshPlotMapper::SetGlyphType(GlyphType type)
{
    if (glyphType != type)
    {
        glyphType = type;
        CustomizeMappers();
    }
}

// ****************************************************************************
// Method: avtMeshPlotMapper::ComputePointSize
//
// Purpose:
//   Computes a point size based on spatial extents.
//   Use for autoSize.
//
// Programmer: Kathleen Biagas
// Creation:   August 8, 2023 
//
// Modifications:
//
// ****************************************************************************

double
avtMeshPlotMapper::ComputePointSize(bool forGlyphs)
{
    double ps = 0.02;
    avtDataset_p input = GetTypedInput();
    if (*input != 0)
    {
        avtDataAttributes &atts=input->GetInfo().GetAttributes();
// should this be actual extents instead?  doesn't work with actual!
        avtExtents *extents = atts.GetOriginalSpatialExtents();
        int nDims = extents->GetDimension();
        double exts[6];
        extents->CopyTo(exts);
        double dist = 0.;
        for (int i = 0; i < nDims; ++i)
        {
            dist += (exts[2*i+1] - exts[2*i]) * (exts[2*i+1] - exts[2*i]);
        }
        dist = sqrt(dist);

        // Should we be autocomputing for pixels?
        double mult = forGlyphs ? 0.0002 : 0.05;
        ps = dist * mult; 
        cerr << " ComputePointSize dist: " << dist << endl;
        if (forGlyphs)
            cerr << " ComputePointSize glyph size: " << ps << endl;
        else 
            cerr << " ComputePointSize pixel size: " << ps << endl;
    }
    return ps;
}


