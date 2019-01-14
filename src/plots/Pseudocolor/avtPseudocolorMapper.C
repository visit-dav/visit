/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                       avtPseudocolorMapper.C                              //
// ************************************************************************* //

#include <avtPseudocolorMapper.h>

#include <vtkDataSet.h>
#include <vtkMultiRepMapper.h>


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


