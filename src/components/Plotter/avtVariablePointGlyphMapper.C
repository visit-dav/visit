// ************************************************************************* //
//                         avtVariablePointGlyphMapper.C                     //
// ************************************************************************* //

#include <avtVariablePointGlyphMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 12, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtVariablePointGlyphMapper::avtVariablePointGlyphMapper():
    avtPointGlypher(), avtVariableMapper()
{
    singleColor[0] = 0.;
    singleColor[1] = 0.;
    singleColor[2] = 0.;
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004
//
//  Modifications:
//
// ****************************************************************************

avtVariablePointGlyphMapper::~avtVariablePointGlyphMapper()
{
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::CustomizeMappers
//
//  Purpose:
//    A hook from the base class that allows the variable mapper to force
//    the vtk mappers to be the same as its state.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::CustomizeMappers(void)
{
    if (colorByScalar)
    {
        avtVariableMapper::CustomizeMappers();
    }
    else     
    {
        if (lighting)
        {
            TurnLightingOn();
        }
        else
        {
            TurnLightingOff();
        }

        SetOpacity(opacity);

        for (int i = 0; i < nMappers; i++)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->SetLookupTable(lut);
            }
            if (actors[i] != NULL)
            {
                vtkProperty *prop = actors[i]->GetProperty();
                prop->SetLineStipplePattern(LineStyle2StipplePattern(lineStyle));
                prop->SetLineWidth(LineWidth2Int(lineWidth));
            }
        }
    }
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        CustomizeGlyphs(GetInput()->GetInfo().GetAttributes().GetSpatialDimension());

        if (!colorByScalar)
            ColorBySingleColor(singleColor);
        if (dataScaling)
            ScaleByVar(scalingVarName);
        else 
            DataScalingOff();
    }
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::SetUpFilters
//
//  Purpose:
//    The glyph mapper inserts filters into the VTK pipeline, but can
//    only do so inside another routines (avtMapper::SetUpMappers) loop.
//    This is called before InsertFilters to allow for initialization work.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::SetUpFilters(int nDoms)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        SetUpGlyphs(nDoms);
    }
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::InsertFilters
//
//  Purpose:
//    Inserts a glyph filter into the vtk Pipeline.
//
//  Arguments:
//    ds        The upstream dataset.
//    dom       The domain number.
//
//  Returns:      The dataset to be sent downstream.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004 
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtVariablePointGlyphMapper::InsertFilters(vtkDataSet *ds, int dom)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
        return ds;

    return InsertGlyphs(ds, dom, 
                 GetInput()->GetInfo().GetAttributes().GetSpatialDimension());

}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::ColorBySingleColor
//
//  Purpose:
//    Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     November 12, 2004 
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::ColorBySingleColor(const float col[3])
{
    ColorByScalarOff();

    singleColor[0] = col[0];
    singleColor[1] = col[1];
    singleColor[2] = col[2];
  
    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                vtkProperty *prop = actors[i]->GetProperty();
                prop->SetColor(singleColor);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::ColorBySingleColor
//
//  Purpose:
//    Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     November 12, 2004 
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::ColorBySingleColor(const unsigned char col[3])
{
    float fc[3];
    fc[0] = (float)col[0] / 255.;
    fc[1] = (float)col[1] / 255.;
    fc[2] = (float)col[2] / 255.;
    ColorBySingleColor(fc);
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::ColorBySingleColor
//
//  Purpose:
//    Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     November 12, 2004 
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::ColorBySingleColor(const double col[3])
{
    float fc[3];
    fc[0] = (float)col[0];
    fc[1] = (float)col[1];
    fc[2] = (float)col[2];
    ColorBySingleColor(fc);
}


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper::ScaleByVar
//
//  Purpose:
//    Turns on data scaling for the glyph portion of this mapper. 
//
//  Arguments:
//    sname     The name of the variable to be used for scaling.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 11, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::ScaleByVar(const string &sname)
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
 
