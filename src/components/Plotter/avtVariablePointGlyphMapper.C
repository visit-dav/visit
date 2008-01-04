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
//                         avtVariablePointGlyphMapper.C                     //
// ************************************************************************* //

#include <avtVariablePointGlyphMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>

#include <vtkVisItDataSetMapper.h>

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
//    Brad Whitlock, Thu Aug 25 15:18:17 PST 2005
//    Added support for point texturing.
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

        //
        // Set the appropriate point texturing mode based on the glyph type.
        // This might not be the best VTK way to do it because we have to have
        // a little knowledge about which type of mapper was created but
        // it seems a lot more efficient to enhance the mapper than to create
        // some special textured glyph or have an actor for each point. Since
        // I'm changing the mapper and there's no base class support for
        // anything like this, I have to cast to the concrete types that
        // I care about.
        //
        for (int i = 0; i < nMappers; i++)
        {
            if (mappers[i] != NULL)
            {
                if(strcmp(mappers[i]->GetClassName(), 
                          "vtkVisItDataSetMapper") == 0)
                {
                    vtkVisItDataSetMapper *dsm = (vtkVisItDataSetMapper *)mappers[i];
                    dsm->SetPointTextureMethod(glyphType == 4 ?
                         vtkVisItDataSetMapper::TEXTURE_USING_POINTSPRITES :
                         vtkVisItDataSetMapper::TEXTURE_NO_POINTS);
                }
            }
        }

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
avtVariablePointGlyphMapper::ColorBySingleColor(const double col[3])
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
    double fc[3];
    fc[0] = (double)col[0] / 255.;
    fc[1] = (double)col[1] / 255.;
    fc[2] = (double)col[2] / 255.;
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

// ****************************************************************************
// Method: avtVariablePointGlyphMapper::SetGlyphType
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
//   Brad Whitlock, Thu Aug 25 10:26:57 PDT 2005
//   Added support for sphere glyphs.
//
// ****************************************************************************

void
avtVariablePointGlyphMapper::SetGlyphType(const int type)
{
    if (type < 0 || type > 4) 
        return; 

    if (glyphType != type)
    {
        // If we're going into point glyphing mode or out of point
        // glyphing mode then change the mapper's input accordingly.
        // We do this switch so we don't have to glyph points but we
        // can still switch from points to glyphs and vice versa.
        if(nMappers > 0 && 
          (glyphType == 3 || type == 3 || glyphType == 4 || type == 4))
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

// ****************************************************************************
// Method: avtVariablePointGlyphMapper::SetFullFrameScaling
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
avtVariablePointGlyphMapper::SetFullFrameScaling(bool useScale, const double *s)
{
    return avtPointGlypher::ProtectedSetFullFrameScaling(useScale, s);
}
