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
//                               avtOpacityMap.C                             //
// ************************************************************************* //

#include <avtOpacityMap.h>

// For NULL
#include <stdlib.h>
#include <string.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtOpacityMap constructor
//
//  Arguments:
//      te      The desired number of table entries.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 12:00:51 PST 2001
//    Modified to make map table-based.
//
//    Hank Childs, Wed Jan  4 11:26:15 PST 2006
//    Call memset to make sure that all entries of RGBA struct are initialized.
//    This cannot be done without a memset because of padding issues.  We
//    want to make sure the inaccessible byte is initialized to avoid purify
//    warnings.
//
// ****************************************************************************

avtOpacityMap::avtOpacityMap(int te)
{
    tableEntries = te;
    table = new RGBA[tableEntries];
  
    // RGBA contains a padded byte after the B and before the A.  Use a memset
    // to make sure this inaccessible byte is initialized.  This will allow
    // us to avoid purify issues.
    memset(table, 0, sizeof(RGBA)*tableEntries);

    for (int i = 0 ; i < tableEntries ; i++)
    {
        table[i].R = 0;
        table[i].G = 0;
        table[i].B = 0;
        table[i].A = 0.;
    }

    min = 0.;
    max = 1.;
    SetIntermediateVars();
}


// ****************************************************************************
//  Method: avtOpacityMap destructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

avtOpacityMap::~avtOpacityMap()
{
    if (table != NULL)
    {
        delete [] table;
    }
}


// ****************************************************************************
//  Method: avtOpacityMap::SetMin
//
//  Purpose:
//      Sets the minimum for the opacity map.  Used for quantizing.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

void
avtOpacityMap::SetMin(double m)
{
    min = m;
    SetIntermediateVars();
}


// ****************************************************************************
//  Method: avtOpacityMap::SetMax
//
//  Purpose:
//      Sets the maximum for the opacity map.  Used for quantizing.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

void
avtOpacityMap::SetMax(double m)
{
    max = m;
    SetIntermediateVars();
}


// ****************************************************************************
//  Method: avtOpacityMap::SetIntermediateVars
//
//  Purpose:
//      Sets up some variables used when quantizing a value.  This is strictly
//      to prevent unnecessary calculation.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 12 17:25:14 PST 2001
//    Multiplier was being set incorrectly.
//
// ****************************************************************************

void
avtOpacityMap::SetIntermediateVars(void)
{
    if (min != max)
    {
        range        = max - min;
    }
    else
    {
        range        = 1.;
    }
    inverseRange = 1./range;
    multiplier   = inverseRange*(tableEntries-1);
}


// ****************************************************************************
//  Method: avtOpacityMap::SetTable
//
//  Purpose:
//      Allows the table to be set from some outside array.
//
//  Arguments:
//      arr     The new table.
//      te      The number of entries in arr.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 19 09:01:31 PST 2001
//    Made opacity a float instead of an unsigned char.
//
//    Hank Childs, Tue Dec 21 16:39:22 PST 2004
//    Add support for attenuation.
//
// ****************************************************************************

void
avtOpacityMap::SetTable(unsigned char *arr, int te, double attenuation)
{
    if (attenuation < 0. || attenuation > 1.)
    {
        debug1 << "Bad attenuation value " << attenuation << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (table != NULL)
    {
        delete [] table;
    }

    tableEntries = te;
    table = new RGBA[tableEntries];
    for (int i = 0 ; i < tableEntries ; i++)
    {
        table[i].R = arr[i*4];
        table[i].G = arr[i*4+1];
        table[i].B = arr[i*4+2];
        table[i].A = ((float) arr[i*4+3] / 255.) * attenuation;
    }

    //
    // We need to set the intermediate vars again since the table size has
    // potentially changed.
    //
    SetIntermediateVars();
}


// ****************************************************************************
//  Method: avtOpacityMap::SetTable
//
//  Purpose:
//      Allows the table to be set from some outside array in the predefined
//      RGBA format.
//
//  Arguments:
//      arr     The new table in RGBA format.
//      te      The number of entries in arr.
//
//  Programmer: Hank Childs
//  Creation:   February 19, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 21 16:39:22 PST 2004
//    Add support for attenuation.
//
// ****************************************************************************

void
avtOpacityMap::SetTable(RGBA *arr, int te, double attenuation)
{
    if (attenuation < 0. || attenuation > 1.)
    {
        debug1 << "Bad attenuation value " << attenuation << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (table != NULL)
    {
        delete [] table;
    }

    tableEntries = te;
    table = new RGBA[tableEntries];
    for (int i = 0 ; i < tableEntries ; i++)
    {
        table[i].R = arr[i].R;
        table[i].G = arr[i].G;
        table[i].B = arr[i].B;
        table[i].A = arr[i].A * attenuation;
        if (table[i].A < 0. || table[i].A > 1.)
        {
            debug1 << "Bad value " << table[i].A << endl;
            EXCEPTION0(ImproperUseException);
        }
    }

    //
    // We need to set the intermediate vars again since the table size has
    // potentially changed.
    //
    SetIntermediateVars();
}


// ****************************************************************************
//  Method: avtOpacityMap::GetOpacity
//
//  Purpose:
//      Gets the opacity for a specific value.  This method should not be used
//      when performing a lot of mappings and is only provided for
//      completeness.  When lots of mappings are done, the GetTable method
//      should be used in conjunction with Quantize.
//
//  Arguments:
//      v       The value.
//
//  Returns:    The <R,G,B,A> corresponding to v.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

const RGBA &
avtOpacityMap::GetOpacity(double v)
{
    return table[Quantize(v)];
}


// ****************************************************************************
//  Method: avtOpacityMap::AddRange
//
//  Purpose:
//      Adds a range to the table.
//
//  Arguments:
//      lo     The low part of the range.
//      hi     The high part of the range.
//      rgba   The opacity and color the range should have.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

void
avtOpacityMap::AddRange(double lo, double hi, RGBA &rgba)
{
    int low  = Quantize(lo);
    int high = Quantize(hi);
    for (int i = low ; i <= high ; i++)
    {
        table[i].R = rgba.R;
        table[i].G = rgba.G;
        table[i].B = rgba.G;
        table[i].A = rgba.A;
    }
}


