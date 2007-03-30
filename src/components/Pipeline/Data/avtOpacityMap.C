// ************************************************************************* //
//                               avtOpacityMap.C                             //
// ************************************************************************* //

#include <avtOpacityMap.h>

// For NULL
#include <stdlib.h>

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
// ****************************************************************************

avtOpacityMap::avtOpacityMap(int te)
{
    tableEntries = te;
    table = new RGBA[tableEntries];
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
// ****************************************************************************

void
avtOpacityMap::SetTable(unsigned char *arr, int te)
{
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
        table[i].A = (float) arr[i*4+3] / 255.;
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
// ****************************************************************************

void
avtOpacityMap::SetTable(RGBA *arr, int te)
{
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
        table[i].A = arr[i].A;
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


