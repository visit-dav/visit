/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            avtVariablePixelizer.C                         //
// ************************************************************************* //

#include <avtVariablePixelizer.h>

#include <vtkLookupTable.h>


// ****************************************************************************
//  Method: avtVariablePixelizer constructor
//
//  Arguments:
//      min      The minimum possible value.
//      max      The maximum possible value.
//      
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
// ****************************************************************************

avtVariablePixelizer::avtVariablePixelizer(double min, double max)
{
    minVal = min;
    maxVal = max;
    lut = vtkLookupTable::New();
    lut->SetTableRange(minVal, maxVal);
    lut->SetHueRange(0.6667, 0.);
    lut->Build();
}


// ****************************************************************************
//  Method: avtVariable destructor
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
// ****************************************************************************

avtVariablePixelizer::~avtVariablePixelizer()
{
    if (lut != NULL)
    {
        lut->Delete();
        lut = NULL;
    }
}


// ****************************************************************************
//  Method: avtVariablePixelizer::SetLookupTable
//
//  Purpose:
//      Overrides the default lookup table.  This routine does not need to be
//      called for the pixelizer to work properly.
//
//  Arguments:
//      newlut   The new lookup table.
//
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
// ****************************************************************************

void
avtVariablePixelizer::SetLookupTable(vtkLookupTable *newlut)
{
    if (newlut != NULL)
    {
        if (lut != NULL)
        {
            lut->Delete();
            lut = NULL;
        }

        lut = newlut;
        lut->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtVariablePixelizer::GetColor
//
//  Purpose:
//      Gets the color appropriate for a value.  Also performs shading based
//      on the intensity.
//
//  Arguments:
//      val         The value along the ray (could be maximum or average, but
//                  it is not important for coloring).
//      intensity   The shading at that point.
//      rgb         Where to put the resulting rgb values.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Cast to get rid of compiler warning.
//
// ****************************************************************************

void
avtVariablePixelizer::GetColor(double val, double intensity,
                               unsigned char rgb[3])
{
    unsigned char *lutsrgb = lut->MapValue(val);
    rgb[0] = (unsigned char)(lutsrgb[0] * intensity);
    rgb[1] = (unsigned char)(lutsrgb[1] * intensity);
    rgb[2] = (unsigned char)(lutsrgb[2] * intensity);
}


