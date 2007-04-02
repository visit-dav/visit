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
//                         avtLevelsPointGlyphMapper.C                       //
// ************************************************************************* //

#include <avtLevelsPointGlyphMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>

#include <vtkVisItDataSetMapper.h>

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
//    Brad Whitlock, Thu Aug 25 15:21:23 PST 2005
//    Added support for point texturing.
// 
// ****************************************************************************

void
avtLevelsPointGlyphMapper::CustomizeMappers(void)
{
    avtLevelsMapper::CustomizeMappers();
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
//   Brad Whitlock, Thu Aug 25 10:26:57 PDT 2005
//   Added support for sphere glyphs.
//
// ****************************************************************************

void
avtLevelsPointGlyphMapper::SetGlyphType(const int type)
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
