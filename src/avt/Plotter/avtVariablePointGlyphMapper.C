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
//                         avtVariablePointGlyphMapper.C                     //
// ************************************************************************* //

#include <avtVariablePointGlyphMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>


// ****************************************************************************
//  Method: avtVariablePointGlyphMapper constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 12, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:37:30 EDT 2008
//    Made constructor initializers match true initializer order.
//
//    Kathleen Biagas, Tue Aug 23 11:38:11 PDT 2016
//    Changed inheritance from avtPointGlypher to avtPointMapper.
//
// ****************************************************************************

avtVariablePointGlyphMapper::avtVariablePointGlyphMapper():
    avtVariableMapper(), avtPointMapper()
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
//    Dave Pugmire, Mon Jul 12 15:34:29 EDT 2010
//    Glyph if topological dimension is <= 1.
//
//    Brad Whitlock, Tue Jan 24 17:04:26 PST 2012
//    Work around mapper/painter issue when setting point type.
//
//    Kathleen Biagas, Tue Aug 23 11:41:14 PDT 2016
//    Glyph-related customization now handled by avtPointMapper.
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

        ColorBySingleColor(singleColor);
    }
    avtPointMapper::CustomizeMappers();
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

