// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtPseudocolorMapper.C                              //
// ************************************************************************* //

#include <avtPseudocolorMapper.h>

#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkMultiRepMapper.h>
#include <vtkProperty.h>


// ****************************************************************************
//  Method: avtPseudocolorMapper constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   Autust 24, 2016
//
//  Modifications:
//
// ****************************************************************************

avtPseudocolorMapper::avtPseudocolorMapper() : avtVariableMapper()
{
    drawSurface   = true;
    drawWireframe = false;
    drawPoints    = false;
    pointSize = 2;
    wireframeColor[0] = wireframeColor[1] = wireframeColor[2] = 0.;
    pointsColor[0] = pointsColor[1] = pointsColor[2] = 0.;
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


// ****************************************************************************
//  Method: avtPseudocolorMapper::CreateMapper
//
//  Purpose:
//    Creates a vtkMultiRepMapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
// ****************************************************************************

vtkDataSetMapper *
avtPseudocolorMapper::CreateMapper()
{
    return vtkMultiRepMapper::New();
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
// ****************************************************************************

void
avtPseudocolorMapper::CustomizeMappers()
{
    avtVariableMapper::CustomizeMappers();

    for (int i = 0; i < nMappers; ++i)
    {
        if (mappers[i] == NULL)
            continue;

        vtkMultiRepMapper *mrm = (vtkMultiRepMapper*)mappers[i];
        mrm->SetDrawSurface(drawSurface);
        mrm->SetDrawWireframe(drawWireframe);
        mrm->SetDrawPoints(drawPoints);
        mrm->SetWireframeColor(wireframeColor);
        mrm->SetPointsColor(pointsColor);
        actors[i]->GetProperty()->SetPointSize(pointSize);
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
// ****************************************************************************

void
avtPseudocolorMapper::SetDrawSurface(bool val)
{
    if (drawSurface != val)
    {
        drawSurface = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
                ((vtkMultiRepMapper *)mappers[i])->SetDrawSurface(drawSurface);
        }
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
// ****************************************************************************

void
avtPseudocolorMapper::SetDrawWireframe(bool val)
{
    if (drawWireframe != val)
    {
        drawWireframe = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
                ((vtkMultiRepMapper *)mappers[i])->SetDrawWireframe(drawWireframe);
        }
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
// ****************************************************************************

void
avtPseudocolorMapper::SetDrawPoints(bool val)
{
    if (drawPoints != val)
    {
        drawPoints = val;
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
                ((vtkMultiRepMapper *)mappers[i])->SetDrawPoints(drawPoints);
        }
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
avtPseudocolorMapper::SetPointSize(int ps)
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
// ****************************************************************************

void
avtPseudocolorMapper::SetWireframeColor(double rgb[3])
{
    if (ColorsAreDifferent(wireframeColor, rgb))
    {
        wireframeColor[0] = rgb[0];
        wireframeColor[1] = rgb[1];
        wireframeColor[2] = rgb[2];
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
                ((vtkMultiRepMapper *)mappers[i])->SetWireframeColor(wireframeColor);
        }
    }
}


// ****************************************************************************
//  Method: avtPseudocolorMapper::SetPointsColor
//
//  Purpose:
//     Sets color to be used for the wirefame mode
//
// 
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016 
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
        for (int i = 0; i < nMappers; ++i)
        {
            if (mappers[i] != NULL)
                ((vtkMultiRepMapper *)mappers[i])->SetPointsColor(pointsColor);
        }
    }
}


