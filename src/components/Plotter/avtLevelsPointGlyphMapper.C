// ************************************************************************* //
//                         avtLevelsPointGlyphMapper.C                       //
// ************************************************************************* //

#include <avtLevelsPointGlyphMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 12, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtLevelsPointGlyphMapper::avtLevelsPointGlyphMapper():
    avtPointGlypher(), avtLevelsMapper()
{
    ColorByScalarOff();
}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004
//
//  Modifications:
//
// ****************************************************************************

avtLevelsPointGlyphMapper::~avtLevelsPointGlyphMapper()
{
}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper::CustomizeMappers
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
avtLevelsPointGlyphMapper::CustomizeMappers(void)
{
    avtLevelsMapper::CustomizeMappers();
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        CustomizeGlyphs(GetInput()->GetInfo().GetAttributes().GetSpatialDimension());
        if (dataScaling)
            ScaleByVar(scalingVarName);
        else 
            DataScalingOff();
    }
}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper::SetUpFilters
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
avtLevelsPointGlyphMapper::SetUpFilters(int nDoms)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        SetUpGlyphs(nDoms);
    }
}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper::InsertFilters
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
avtLevelsPointGlyphMapper::InsertFilters(vtkDataSet *ds, int dom)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
        return ds;

    return InsertGlyphs(ds, dom, 
                 GetInput()->GetInfo().GetAttributes().GetSpatialDimension());

}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper::ScaleByVar
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
avtLevelsPointGlyphMapper::ScaleByVar(const string &sname)
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
// Method: avtLevelsPointGlyphMapper::SetGlyphType
//
// Purpose: 
//   This method sets the point glyph type.
//
// Arguments:
//
// Returns:    
//
// Note:       This method overrides avtPointGlypher::SetGlyphType and allows
//             us to change the mapper's input if we're switching to and fro
//             between point glyphing mode. We do the switch because we don't
//             want to expend any effort in a glyph filter or a normals filter
//             if we're just drawing points so we set the mapper's input to
//             the point dataset directly. If we're switching out of points
//             mode then we have to add the glyph and normals filters to make
//             sure that the points get glyphed appropriately.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 22 11:17:08 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLevelsPointGlyphMapper::SetGlyphType(const int type)
{
    if (type < 0 || type > 3) 
        return; 

    if (glyphType != type)
    {
        // If we're going into point glyphing mode or out of point
        // glyphing mode then change the mapper's input accordingly.
        // We do this switch so we don't have to glyph points but we
        // can still switch from points to glyphs and vice versa.
        if(nMappers > 0 && (glyphType == 3 || type == 3))
        {
            avtDataObject_p input = GetInput();
            if (*input != NULL)
            {
                int tmp = glyphType;
                glyphType = type;
                avtDataTree_p tree = GetInputDataTree();
                vtkDataSet **children = tree->GetAllLeaves(nMappers);
                for (int i = 0; i < nMappers; i++)
                {
                    if (mappers[i] != NULL)
                    {
                        mappers[i]->SetInput(InsertFilters(children[i], i));
                    }
                }
                // this was allocated in GetAllLeaves, need to free it now
                delete [] children;

                PrepareExtents();
                CustomizeMappers();

                glyphType = tmp;
            }
        }

        avtPointGlypher::SetGlyphType(type);
    }
}
