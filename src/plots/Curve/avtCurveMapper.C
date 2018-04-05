/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
//                             avtCurveMapper.C                              //
// ************************************************************************* //

#include <avtCurveMapper.h>
#include <vtkCurveMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>



// ****************************************************************************
//  Method: avtCurveMapper constructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

avtCurveMapper::avtCurveMapper()
{
    curveColor[0] = curveColor[1] = curveColor[2] = 1.0;
    lineWidth = LW_0;
    drawCurve = true;
    drawPoints = false;
    staticPoints = true;
    pointSize = 5.;
    pointStride = 1;
    pointDensity = 50;
    ffScale[0] = ffScale[1] = ffScale[2] = 1.;
    symbolType = 0;
}


// ****************************************************************************
//  Method: avtCurveMapper destructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 13, 2017 
//
//  Modifications:
//
// ****************************************************************************

avtCurveMapper::~avtCurveMapper()
{
}


// ****************************************************************************
//  Method: avtCurveMapper::CreateMapper
//
//  Purpose:
//    Creates a vtkDataSetMapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
// ****************************************************************************

vtkDataSetMapper *
avtCurveMapper::CreateMapper()
{
    return (vtkDataSetMapper *)vtkCurveMapper::New();
}

// ****************************************************************************
//  Method: avtCurveMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the curve mapper to make any
//      calls that were made before the base class set up the vtk mappers
//      take effect.
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 13, 2017 
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::CustomizeMappers(void)
{
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            vtkCurveMapper *cm = (vtkCurveMapper*)mappers[i];
            cm->SetDrawCurve(drawCurve);
            cm->SetDrawPoints(drawPoints);
            cm->SetPointStride(pointStride);
            cm->SetStaticPoints(staticPoints);
            cm->SetPointDensity(pointDensity);
            cm->SetPointSize(pointSize);
            cm->SetSymbolType(symbolType);
            cm->SetFFScale(ffScale);

            vtkProperty* prop = actors[i]->GetProperty();
            prop->SetColor(curveColor);
            prop->SetLineWidth(LineWidth2Int(lineWidth));
            prop->SetPointSize(pointSize);
        }
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetColor
//
//  Purpose:
//      Sets the color for all the actors of plot.
//
//  Arguments:
//      c        The new color
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017 
//
//  Modifications:
//
//
// ****************************************************************************

void
avtCurveMapper::SetColor(double c[3])
{
    SetColor(c[0], c[1], c[2]);
}


// ****************************************************************************
//  Method: avtCurveMapper::SetColor
//
//  Purpose:
//      Sets the color for all the actors of plot.
//
//  Arguments:
//      r        The new color's red component.
//      g        The new color's green component.
//      b        The new color's blue component.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017 
//
//  Modifications:
//
//
// ****************************************************************************

void
avtCurveMapper::SetColor(double r, double g, double b)
{
    curveColor[0] = r;
    curveColor[1] = g;
    curveColor[2] = b;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetColor(r, g, b);
        }
    }
}



// ****************************************************************************
//  Method: avtCurveMapper::SetLineWidth
//
//  Purpose:
//      Sets the line width of the plot.
//
//  Arguments:
//      w        The new line width
//
//  Programmer:  Kathleen Biagas 
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetLineWidth(_LineWidth lw)
{
    lineWidth = lw; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->
                SetLineWidth(LineWidth2Int(lineWidth));
        }
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetDrawCurve
//
//  Purpose:
//      Sets the toggle for drawing the curve portion.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetDrawCurve(bool val)
{
    drawCurve = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetDrawCurve(drawCurve);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetDrawPoints
//
//  Purpose:
//      Sets the toggle for drawing the Points portion.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetDrawPoints(bool val)
{
    drawPoints = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetDrawPoints(drawPoints);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetPointSize
//
//  Purpose:
//      Sets the point size.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetPointSize(double val)
{
    pointSize = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        vtkCurveMapper *cm = (vtkCurveMapper*)mappers[i];
        cm->SetPointSize(pointSize);
        actors[i]->GetProperty()->SetPointSize(pointSize);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetPointStride
//
//  Purpose:
//      Sets the point stride.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetPointStride(int val)
{
    pointStride = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetPointStride(pointStride);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetSymbolType
//
//  Purpose:
//      Sets SymbolType.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetSymbolType(int val)
{
    symbolType = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetSymbolType(symbolType);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetPointDensity
//
//  Purpose:
//      Sets PointDensity.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetPointDensity(int val)
{
    pointDensity = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetPointDensity(val);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetStaticPoints
//
//  Purpose:
//      Sets static points.
//
//  Arguments:
//      val      The new value.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveMapper::SetStaticPoints(bool val)
{
    staticPoints = val; 

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetStaticPoints(val);
    }
}


// ****************************************************************************
//  Method: avtCurveMapper::SetFullFrameScaling
//
//  Purpose:
//      Sets full frame scaling
//
//  Arguments:
//      s        The new scaling.
//
//  Programmer:  Kathleen Biagas
//  Creation:    April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

bool
avtCurveMapper::SetFullFrameScaling(bool, const double *s)
{
    ffScale[0] = s[0];
    ffScale[1] = s[1];
    ffScale[2] = s[2];

    for (int i = 0 ; i < nMappers ; i++)
    {
        ((vtkCurveMapper*)mappers[i])->SetFFScale(ffScale);
    }
    return true;
}
