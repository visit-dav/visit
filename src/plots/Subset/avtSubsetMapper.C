// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtSubsetMapper.C
// ****************************************************************************

#include <avtSubsetMapper.h>

#include <avtTransparencyActor.h>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPointGlyphMapper.h>
#include <vtkProperty.h>
#include <algorithm>

using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtSubsetMapper constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
//  Modifications:
//
// ****************************************************************************

avtSubsetMapper::avtSubsetMapper() : avtLevelsMapper()
{
    glyphType = Point;
    scale = 0.2;
    scalingVarName = "";
    pointSize = 2;
}


// ****************************************************************************
//  Method: avtSubsetMapper destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
// ****************************************************************************

avtSubsetMapper::~avtSubsetMapper()
{
}


// ****************************************************************************
//  Method: avtSubsetMapper::CreateActorMapperPairs
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
//  Creation:   June 5, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtSubsetMapper::CreateActorMapperPairs(vtkDataSet **children)
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
            (labels[i].compare(0, 14, string("subset_points_")) != 0)))
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
//  Method: avtSubsetMapper::CustomizeMappers
//
//  Purpose:
//    Adds our flags to the vtk mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
//  Modifications:
//    Kathleen Biagas, Tue Nov  5 12:16:41 PST 2019
//    Call CustomizeMappersInternal.
//
// ****************************************************************************

void
avtSubsetMapper::CustomizeMappers()
{
    avtLevelsMapper::CustomizeMappers();
    CustomizeMappersInternal();
}


// ****************************************************************************
//  Method: avtSubsetMapper::CustomizeMappersInternal
//
//  Purpose:
//    Adds our flags to the vtk mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
// ****************************************************************************

void
avtSubsetMapper::CustomizeMappersInternal(bool invalidateTransparency)
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
            pgm->SetGlyphType(glyphType);
            pgm->ColorByScalarOff();
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
    }
}


// ****************************************************************************
//  Method: avtSubsetMapper::ScaleByVar
//
//  Purpose:
//    Turns on data scaling for the glyph portion of this mapper.
//
//  Arguments:
//    sname     The name of the variable to be used for scaling.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtSubsetMapper::ScaleByVar(const std::string &sname)
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
//  Method: avtSubsetMapper::DataScalingOff
//
//  Purpose:
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtSubsetMapper::DataScalingOff(void)
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
//  Method: avtSubsetMapper::DataScalingOn
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
//  Creation:   June 5, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtSubsetMapper::DataScalingOn(const string &sname, int varDim)
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
//  Method: avtSubsetMapper::SetScale
//
//  Purpose:
//    Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Kathleen Biagas
//  Creation:    June 5, 2020
//
// ****************************************************************************

void
avtSubsetMapper::SetScale(double s)
{
    scale = s;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkPointGlyphMapper"))
            ((vtkPointGlyphMapper*)mappers[i])->SetScale(scale);
    }
}


// ****************************************************************************
// Method: avtSubsetMapper::SetGlyphType
//
// Purpose:
//   This method sets the point glyph type.
//
// Programmer: Kathleen Biagas
// Creation:   June 5, 2020
//
// Modifications:
//
// ****************************************************************************

void
avtSubsetMapper::SetGlyphType(GlyphType type)
{
    if (glyphType != type)
    {
        glyphType = type;
        CustomizeMappersInternal();
    }
}


// ****************************************************************************
//  Method: avtSubsetMapper::SetPointSize
//
//  Purpose:
//     Sets the point size.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 10, 2019
//
// ****************************************************************************

void
avtSubsetMapper::SetPointSize(double ps)
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
// Method: avtSubsetMapper::SetFullFrameScaling
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
// Creation:   June 5, 2020
//
// Modifications:
//
// ****************************************************************************

bool
avtSubsetMapper::SetFullFrameScaling(bool useScale, const double *s)
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
//  Method: avtSubsetMapper::SetLabels
//
//  Purpose:
//     Saves the labels coming from the input data tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
// ****************************************************************************

void
avtSubsetMapper::SetLabels(vector<string> &l, bool fromTree)
{
    // labels stored here are for separating out points-designated
    // portions
    if (fromTree)
        labels = l;

    // avtVertexExtractor prepends "subset_xxx" labels to facilitate
    // which vtk mapper is applied, but that prefix needs to be removed
    // when sent to parent class, otherwise level colors won't be applied
    // properly.
    vector<string> strippedLabels;
    for (size_t i = 0; i < l.size(); ++i)
    {
        if (l[i].compare(0, 12, "subset_surf_") == 0)
            strippedLabels.push_back(l[i].substr(12));
        else if (l[i].compare(0, 14, "subset_points_") == 0)
            strippedLabels.push_back(l[i].substr(14));
        else
            strippedLabels.push_back(l[i]);
    }
    // labels used here are for coloring.
    avtLevelsMapper::SetLabels(strippedLabels, fromTree);
}

