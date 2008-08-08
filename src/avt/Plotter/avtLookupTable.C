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
//                            avtLookupTable.C                               //
// ************************************************************************* //

#include <avtLookupTable.h>

#include <avtColorTables.h>
#include <vtkLookupTable.h>
#include <vtkLogLookupTable.h>
#include <vtkSkewLookupTable.h>

#include <InvalidColortableException.h>

#define INV_255 0.0039215686274509803377

// ****************************************************************************
//  Method: avtLookupTable constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2001 
//
// ****************************************************************************

avtLookupTable::avtLookupTable()
{
    //
    //  Build a basic 'hot' lut, in case user does not want to set
    //  colors separately.  VTK's default size is 256 colors.
    //  The default range for scalars mapped through the table is (0, 1).
    // 
    stdLUT  = vtkLookupTable::New();    
    stdLUT->SetHueRange(0.6667, 0);
    stdLUT->Build();

    logLUT  = vtkLogLookupTable::New();    
    logLUT->SetHueRange(0.6667, 0);
    logLUT->Build();

    skewLUT = vtkSkewLookupTable::New();    
    skewLUT->SetHueRange(0.6667, 0);
    skewLUT->Build();
}


// ****************************************************************************
//  Method: avtLookupTable destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2001 
//
// ****************************************************************************

avtLookupTable::~avtLookupTable()
{
    stdLUT->Delete();
    stdLUT = NULL;

    logLUT->Delete();
    logLUT = NULL;

    skewLUT->Delete();
    skewLUT = NULL;
}


// ****************************************************************************
//  Method: avtLookupTable::SetSkewFactor
//
//  Purpose:
//      Sets the skew factor for the skew LUT. 
//
//  Arguments:
//      s      The skew factor. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 27, 2001 
//
// ****************************************************************************

void
avtLookupTable::SetSkewFactor(const double s)
{
    skewLUT->SetSkewFactor(s);
}


// ****************************************************************************
// Method: avtVariableMapper::SetLUTColors
//
// Purpose: 
//   Sets the specified colors into each type of lookup table. 
//
// Arguments:
//   colors  : An array of rgb triples stored as unsigned chars.
//   nColors : The number of colors in the colors array.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 11:35:13 PDT 2001
//
// Modifications:
//
//   Kathleen Bonnell, Mon Aug 27 12:42:00 PDT 2001
//   Moved from all avtXXXMapper classes.  Removed call to lut->SetHueRange 
//   and lut->SetRange.  Use instead lut->SetNumberOfTableValues.
//   
// ****************************************************************************

void
avtLookupTable::SetLUTColors(const unsigned char *colors, int nColors)
{
    // Rebuild the lut with the new color table.
    stdLUT->SetNumberOfTableValues(nColors);
    logLUT->SetNumberOfTableValues(nColors);
    skewLUT->SetNumberOfTableValues(nColors);


    const unsigned char *cptr = colors;
    for(int i = 0; i < nColors; ++i)
    {
        double r = double(cptr[0]) * INV_255;
        double g = double(cptr[1]) * INV_255;
        double b = double(cptr[2]) * INV_255;
        stdLUT->SetTableValue(i, r, g, b, 1.);
        logLUT->SetTableValue(i, r, g, b, 1.);
        skewLUT->SetTableValue(i, r, g, b, 1.);
        cptr += 3;
    }
}


// ****************************************************************************
// Method: avtVariableMapper::SetLUTColorsWithOpacity
//
// Purpose: 
//   Sets the specified colors into each type of lookup table. 
//
// Arguments:
//   colors  : An array of rgb triples stored as unsigned chars.
//   nColors : The number of colors in the colors array.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 11:35:13 PDT 2001
//
// Modifications:
//
//   Kathleen Bonnell, Mon Aug 27 12:42:00 PDT 2001
//   Modified from Brad's original method to include opacity.  
//   
// ****************************************************************************

void
avtLookupTable::SetLUTColorsWithOpacity(const unsigned char *colors, 
                                        int nColors)
{
    // Rebuild the lut with the new color table.
    stdLUT->SetNumberOfTableValues(nColors);
    logLUT->SetNumberOfTableValues(nColors);
    skewLUT->SetNumberOfTableValues(nColors);

    const unsigned char *cptr = colors;
    for(int i = 0; i < nColors; ++i)
    {
        double r = (double) cptr[0] * INV_255 ;
        double g = (double) cptr[1] * INV_255 ;
        double b = (double) cptr[2] * INV_255 ;
        double a = (double) cptr[3] * INV_255 ;
 
        stdLUT->SetTableValue(i, r, g, b, a);
        logLUT->SetTableValue(i, r, g, b, a);
        skewLUT->SetTableValue(i, r, g, b, a);
        cptr += 4;
    }
}


// ****************************************************************************
// Method: avtVariableMapper::GetNumberOfColors
//
// Purpose: 
//   Retrieves the number of colors currently in the luts. 
//
// Returns: 
//   The number of colors. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 28, 2001 
//
// ****************************************************************************

int
avtLookupTable::GetNumberOfColors()
{
    return stdLUT->GetNumberOfColors();
}


// ****************************************************************************
// Method: avtLookupTable::SetColorTable
//
// Purpose: 
//   Sets the color table for the lookup table.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    Returns true if the color table is updated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:52:49 PST 2001
//
// Modifications:
//    Jeremy Meredith, Thu Aug 23 14:11:40 PDT 2001
//    Made it use the color table name "Default" instead of the boolean flag.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    Moved from avtXXXPlot.C.  Added 'validName' for plots to
//    send along.
//
//    Brad Whitlock, Wed Nov 20 14:23:47 PST 2002
//    I changed it so it conforms to the new interface for avtColortTables.
//    It gets the default continuous colortable if it is supposed to use
//    the default. If it cannot get the default continuous colortable, it
//    tries to get the default discrete colortable.
//
//    Brad Whitlock, Fri Apr 25 12:26:40 PDT 2003
//    I made it throw InvalidColortableException.
//
//    Jeremy Meredith, Thu Aug  7 14:38:16 EDT 2008
//    Use true string comparison (instead of undefined pointer comparison).
//
//    Jeremy Meredith, Fri Aug  8 10:26:38 EDT 2008
//    Check for NULL ctName before doing a comparison, not afterwards....
//
// ****************************************************************************

bool
avtLookupTable::SetColorTable(const char *ctName, bool validName)
{
    bool retval = false;
    bool useDefault = false;
    avtColorTables *ct = avtColorTables::Instance();

    // Figure out the circumstances in which we should use the default
    // color table.
    if(ctName == NULL)
        useDefault = true;
    else if(std::string(ctName) == "Default")
        useDefault = true;
    else if(!ct->ColorTableExists(ctName))
    {
        EXCEPTION1(InvalidColortableException, ctName);
    }

    if(useDefault)
    {
        // Use the default color table.
        const char *dct = ct->GetDefaultContinuousColorTable().c_str();
        if(dct == 0)
            dct = ct->GetDefaultDiscreteColorTable().c_str();
        const unsigned char *c = ct->GetColors(dct);
        if(c != NULL)
        {
            // Set the colors into the lookup table.
            retval = true;
            SetLUTColors(c, ct->GetNumColors());
        }
    }
    else if (validName) 
    {
        // Use the specified color table. It was a valid color table.
        const unsigned char *c = ct->GetColors(ctName);
        if(c != NULL)
        {
            // Set the colors into the lookup table.
            retval = true;
            SetLUTColors(c, ct->GetNumColors());
        }
    }

    return retval;
}
