// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtLabelMapper.h>

#include <vtkLabelMapper.h>


// ****************************************************************************
//  Method: avtLabelMapper::avtLabelMapper
//
//  Purpose:
//    Constructor for the avtLabelMapper class.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

avtLabelMapper::avtLabelMapper() : atts(), varname(), globalLabel()
{
    fgColor[0] = 0.;
    fgColor[1] = 0.;
    fgColor[2] = 0.;
    fgColor[3] = 1.;

    treatAsASCII = false;
    renderLabels3D = false;

    spatialExtents[0] = 0.;
    spatialExtents[1] = 0.;
    spatialExtents[2] = 0.;
    spatialExtents[3] = 0.;
    spatialExtents[4] = 0.;
    spatialExtents[5] = 0.;

    useGlobalLabel = false;
    cellOrigin = 0;
    nodeOrigin = 0;
}


// ****************************************************************************
//  Method: avtLabelMapper::~avtLabelMapper
//
//  Purpose: 
//    Destructor for the avtLabelMapper class.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

avtLabelMapper::~avtLabelMapper()
{
}


// ****************************************************************************
//  Method: avtLabelMapper::CreateMapper
//
//  Purpose:
//    Creates a vtkLabelMapper.
//
//  Returns:    A pointer to the new mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

vtkMapper2D *
avtLabelMapper::CreateMapper2D(void)
{
    return vtkLabelMapper::New();
}


// ****************************************************************************
//  Method: avtLabelMapper::CustomizeMappers
//
//  Purpose:
//    Sets the label to be used for labelling a particular dataset in the
//    mapper, which will then set the label into the renderer.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::CustomizeMappers(void)
{
    vtkLabelMapper *firstMapper = NULL;
    vtkLabelMapper *lastMapper = NULL;

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            if(firstMapper == NULL)
                firstMapper = (vtkLabelMapper*)mappers[i];
            lastMapper = (vtkLabelMapper*)mappers[i];

            vtkLabelMapper *M = (vtkLabelMapper *)mappers[i];
            M->SetAtts(&atts);
            if((size_t)i < labelNames.size())
                M->SetGlobalLabel(labelNames[i]);
            M->SetUseGlobalLabel(useGlobalLabel);
            M->SetRendererAction(vtkLabelMapper::RENDERER_ACTION_NOTHING);

            M->SetVariable(varname);
            M->SetTreatAsASCII(treatAsASCII);
            M->SetRenderLabels3D(renderLabels3D);
            M->SetSpatialExtents(spatialExtents);
            M->SetForegroundColor(fgColor);
            M->SetUseGlobalLabel(useGlobalLabel);
            M->SetCellOrigin(cellOrigin);
            M->SetNodeOrigin(nodeOrigin);
        }
    }

    // Set some "renderer action" flags
    if(firstMapper != NULL)
    {
        if(firstMapper == lastMapper)
        {
            firstMapper->SetRendererAction(
                vtkLabelMapper::RENDERER_ACTION_INIT_ZBUFFER |
                vtkLabelMapper::RENDERER_ACTION_FREE_ZBUFFER);
        }
        else
        {
            firstMapper->SetRendererAction(
                vtkLabelMapper::RENDERER_ACTION_INIT_ZBUFFER);
            lastMapper->SetRendererAction(
                vtkLabelMapper::RENDERER_ACTION_FREE_ZBUFFER);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetLabels
//
//  Purpose:
//    Sets the label names to use when drawing labels.
//
//  Arguments:
//    L :        The labels to use.
//    fromTree:  Wether or not the labels came from the actual data tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetLabels(std::vector<std::string> &L, bool fromTree)
{
    if(fromTree)
    {
        // These are the actual labels present in the plot.
        labelNames = L;
    }
    // else all of the possible levels.
}


// ****************************************************************************
//  Method:  avtLabelMapper::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetAtts(const LabelAttributes *newAtts)
{
    atts = *newAtts;

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetAtts(&atts);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetForegroundColor
//
//  Purpose:
//    Sets the foreground color to be used by the labels if they are being
//    drawn in the foreground color.
//
//  Arguments:
//    fg : The foreground color.
//
//  Returns:    True if the color was set; False otherwise.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

bool
avtLabelMapper::SetForegroundColor(const double *fg)
{
    bool retVal = false;

    if (!atts.GetTextFont1().GetUseForegroundColor() ||
        (atts.GetVarType() == LabelAttributes::LABEL_VT_MESH &&
         !atts.GetTextFont2().GetUseForegroundColor()))
    {
       if (fgColor[0] != fg[0] || fgColor[1] != fg[1] || fgColor[2] != fg[2])
       {
           retVal = true;
       }
    }

    fgColor[0] = fg[0];
    fgColor[1] = fg[1];
    fgColor[2] = fg[2];

    return retVal;
}


// ****************************************************************************
//  Method: avtLabelMapper::SetVariable
//
//  Purpose:
//    Sets the name of the variable that is being rendered.
//
//  Arguments:
//    name : The name of the plotted variable.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetVariable(const std::string &name)
{
    varname = name;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetVariable(varname);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetTreatAsASCII
//
//  Purpose:
//    Causes the renderer to draw character data as characters.
//
//  Arguments:
//    val : Whether to draw char data as characters.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetTreatAsASCII(bool val)
{
    treatAsASCII = val;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetTreatAsASCII(treatAsASCII);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::Set3D
//
//  Purpose:
//    Tells the renderer whether to draw the labels as 3D.
//
//  Arguments:
//    val : Whether to draw in 3D.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::Set3D(bool val)
{
    renderLabels3D = val;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetRenderLabels3D(renderLabels3D);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetExtents
//
//  Purpose:
//    Sets the spatial extents into the renderer.
//
//  Arguments:
//    ext : The spatial extents.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetExtents(const double *ext)
{
    spatialExtents[0] = ext[0];
    spatialExtents[1] = ext[1];
    spatialExtents[2] = ext[2];
    spatialExtents[3] = ext[3];
    spatialExtents[4] = ext[4];
    spatialExtents[5] = ext[5];
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetSpatialExtents(spatialExtents);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetUseGlobalLabel
//
//  Purpose:
//    Tells the renderer that it should use a global label instead of creating
//    labels for each cell, node.
//
//  Arguments:
//    val : Whether or not to use a global label.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetUseGlobalLabel(bool val)
{
    useGlobalLabel = val;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetUseGlobalLabel(useGlobalLabel);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetCellOrigin
//
//  Purpose:
//    Sets the cell origin.
//
//  Arguments:
//    origin : The new cell origin.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//   
// ****************************************************************************

void
avtLabelMapper::SetCellOrigin(vtkIdType origin)
{
    cellOrigin = origin;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetCellOrigin(cellOrigin);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetNodeOrigin
//
//  Purpose: 
//    Sets the node origin.
//
//  Arguments:
//   origin : The new node origin.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtLabelMapper::SetNodeOrigin(vtkIdType origin)
{
    nodeOrigin = origin;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            ((vtkLabelMapper*)mappers[i])->SetNodeOrigin(nodeOrigin);
        }
    }
}


// ****************************************************************************
//  Method: avtLabelMapper::SetFullFrameScaling
//
//  Purpose:
//    Sets a full frame scale factor that can be used by the mapper to
//    compensate for the stretching that full frame mode performs on geometry.
//
//  Arguments:
//    useScale : True if the scale is used.
//    s        : The fullframe scale vector.
//
//  Returns:    True if any vtk mappers use the scale.
//
//  Note:  Taken from avtPseudocolorMapper.
//
//  Programmer: Alister Maguire
//  Creation:   May 21, 2021
//
//  Modifications:
//
// ****************************************************************************

bool
avtLabelMapper::SetFullFrameScaling(bool useScale, const double *s)
{
    bool retval = false;
    for (int i = 0; i < nMappers && !retval; ++i)
    {
        if (mappers[i] != NULL && mappers[i]->IsA("vtkLabelMapper"))
           retval = ((vtkLabelMapper*)mappers[i])->
                SetFullFrameScaling(useScale, s);
    }
    return retval;
}
