// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPointMapper.C                              //
// ************************************************************************* //

#include <avtPointMapper.h>

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkPointGlyphMapper.h>
#include <vtkProperty.h>


using std::string;

// ****************************************************************************
//  Method: avtPointMapper constructor
//
//  Arguments:
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 17, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtPointMapper::avtPointMapper()
{
    glyphType = Point;
    colorByScalar = true;
    scale = 0.2;
    scalingVarName = "";
    scalingVarDim = 1;
    coloringVarName = "";
    spatialDim = 3;
    pointSize = 0;

    lut = NULL;
}


// ****************************************************************************
//  Method: avtPointMapper destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016
//
//  Modifications:
//
// ****************************************************************************

avtPointMapper::~avtPointMapper()
{
}

// ****************************************************************************
//  Method: avtPointMapper::CreateMapper
//
//  Purpose:
//    Creates a vtkPointGlyphMapper
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016
//
//  Modifications:
//
// ****************************************************************************

vtkDataSetMapper *
avtPointMapper::CreateMapper()
{
    return (vtkDataSetMapper*)(vtkPointGlyphMapper::New());
}


// ****************************************************************************
//  Method: avtPointMapper::CustomizeMappers
//
//  Purpose:
//    A hook from the base class that allows this mapper to force
//    the vtk mappers to be the same as its state.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtPointMapper::CustomizeMappers(void)
{
    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] == NULL)
            continue;

        actors[i]->GetProperty()->SetPointSize(pointSize);

        vtkPointGlyphMapper *pm = (vtkPointGlyphMapper*)mappers[i];
        pm->SetSpatialDimension(
            GetInput()->GetInfo().GetAttributes().GetSpatialDimension());
        pm->SetGlyphType(glyphType);
        pm->SetLookupTable(lut);
        if (colorByScalar)
            pm->ColorByScalarOn(coloringVarName);
        else
            pm->ColorByScalarOff();
    }

    if (dataScaling)
    {
        // need to call ScaleByVar rather than DataScalingOn, because
        // scalingVarDim may not yet have been set correctly.
        ScaleByVar(scalingVarName);
    }
    else
        DataScalingOff();

    SetScale(scale);
}


// ****************************************************************************
//  Method: avtPointMapper::ColorByScalarOn
//
//  Purpose:
//    Tells the mapper to color by the given scalar.
//
//  Arguments:
//    sn          The name of the scalar var.
//
//  Programmer:   Kathleen Biagas
//  Creation:     August 17, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointMapper::ColorByScalarOn(const string &sn)
{
    colorByScalar = true;
    coloringVarName = sn;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL) 
            ((vtkPointGlyphMapper*)mappers[i])->ColorByScalarOn(coloringVarName); 
    }
}

// ****************************************************************************
//  Method: avtPointMapper::ColorByScalarOff
//
//  Purpose:
//    Tells the mapper to color all of the points the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Biagas
//  Creation:     August 17, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointMapper::ColorByScalarOff()
{
    colorByScalar = false;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL) 
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
//  Creation:   August 17, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointMapper::ScaleByVar(const std::string &sname)
{
    if (sname == "" || sname == "\0")
    {
        DataScalingOff();
        return; 
    }
    scalingVarName = sname;       
    scalingVarDim = 1;
    if (*(GetInput()) != NULL && 
        GetInput()->GetInfo().GetAttributes().ValidVariable(sname.c_str()))
    {
        scalingVarDim = GetInput()->GetInfo().GetAttributes().
                        GetVariableDimension(sname.c_str()); 
    }
    DataScalingOn(scalingVarName, scalingVarDim);
}

// ****************************************************************************
//  Method: avtPointMapper::DataScalingOff 
//
//  Purpose:
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointMapper::DataScalingOff(void)
{
    dataScaling = false;
    scalingVarName = "";

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
            ((vtkPointGlyphMapper*)mappers[i])->DataScalingOff(); 
    }
}

// ****************************************************************************
//  Method: avtPointMapper::DataScalingOn 
//
//  Purpose:
//    Turns on the appropriate type of data scaling based on the dimension
//    of the variable to be used in scaling. 
//
//  Arguments:
//    sname     The name of the scalars to be used for scaling the glyphs.
//    varDim    The dimension of the var to be used for scaling.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 09:24:15 PST 2004
//    Added varDim argument so that data scaling can be done by other
//    than just scalar vars.  
//
//    John Schmidt, Thu Nov 15 13:08:21 MST 2012
//    Added capability to scale by a 3x3 tensor.  
//
// ****************************************************************************


void
avtPointMapper::DataScalingOn(const string &sname, int varDim)
{
    dataScaling = true;
    scalingVarName = sname;
    scalingVarDim = varDim;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
            ((vtkPointGlyphMapper*)mappers[i])->DataScalingOn(sname, varDim); 
    }
}



// ****************************************************************************
//  Method: avtPointMapper::SetScale
//
//  Purpose:
//    Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Kathleen Biagas 
//  Creation:    August 17, 2016 
//
// ****************************************************************************

void
avtPointMapper::SetScale(double s)
{
    scale = s;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
            ((vtkPointGlyphMapper*)mappers[i])->SetScale(scale);
    }
}


// ****************************************************************************
// Method: avtPointMapper::SetGlyphType
//
// Purpose: 
//   This method sets the point glyph type.
//
// Programmer: Kathleen Biagas 
// Creation:   August 17, 2016
//
// Modifications:
//
// ****************************************************************************

void
avtPointMapper::SetGlyphType(GlyphType type)
{
    glyphType = type;
    for (int i = 0; i < nMappers; i++)
    {
        if (mappers[i] != NULL)
            ((vtkPointGlyphMapper*)mappers[i])->SetGlyphType(type);
    }
}

// ****************************************************************************
//  Method: avtPointMapper::SetPointSize
//
//  Purpose:
//      Sets the point size for all the actors of plot.
//
//  Arguments:
//      s        The new point size
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 22, 2001
//
// ****************************************************************************

void
avtPointMapper::SetPointSize(double s)
{
    pointSize = s;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetPointSize(s);
        }
    }
}


// ****************************************************************************
// Method: avtPointMapper::SetFullFrameScaling
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
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 24 13:55:54 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtPointMapper::SetFullFrameScaling(bool useScale, const double *s)
{
    bool retval = false;
    for (int i = 0; i < nMappers && !retval; ++i)
    {
        if (mappers[i] != NULL)
           retval = ((vtkPointGlyphMapper*)mappers[i])->
                SetFullFrameScaling(useScale, s);
    }
    return retval;
}



// ****************************************************************************
//  Method: avtPointMapper::SetLUT
//
//  Purpose:
//    Causes the mapper to use the specified lookup table. 
//
//  Arguments:
//    lut    The new value for the lookup table. 
//
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 08, 2001 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointMapper::SetLUT(vtkLookupTable *LUT)
{
    if (lut == LUT)
    {
        // no need to change anything
        return;
    }

    lut = LUT;

    for (int i = 0; i < nMappers; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkPointGlyphMapper*)mappers[i])->SetLookupTable(lut);
        }
    }
}


