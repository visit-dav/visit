// ************************************************************************* //
//                            avtLogicalSelection.C                          //
// ************************************************************************* //

#include <limits.h>

#include <avtLogicalSelection.h>

// ****************************************************************************
//  Method: avtLogicalSelection constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************

avtLogicalSelection::avtLogicalSelection()
{
    ndims = 3;
    starts[0] = 0;
    starts[1] = 0;
    starts[2] = 0;
    stops[0] = -1; // '-1' ==> max
    stops[1] = -1; // '-1' ==> max
    stops[2] = -1; // '-1' ==> max
    strides[0] = 1;
    strides[1] = 1;
    strides[2] = 1;
}

// ****************************************************************************
//  Method: SetStarts 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::SetStarts(const int *_starts)
{
    for (int i = 0; i < ndims; i++)
        starts[i] = _starts[i];
    for (int i = ndims; i < 3; i++)
        starts[i] = 0;
}

// ****************************************************************************
//  Method: SetStops
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::SetStops(const int *_stops)
{
    for (int i = 0; i < ndims; i++)
        stops[i] = _stops[i];
    for (int i = ndims; i < 3; i++)
        stops[i] = 0;
}

// ****************************************************************************
//  Method: SetStrides
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::SetStrides(const int *_strides)
{
    for (int i = 0; i < ndims; i++)
        strides[i] = _strides[i];
    for (int i = ndims; i < 3; i++)
        strides[i] = 0;
}

// ****************************************************************************
//  Method: GetStarts
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::GetStarts(int *_starts) const
{
    for (int i = 0; i < ndims; i++)
        _starts[i] = starts[i];
    for (int i = ndims; i < 3; i++)
        _starts[i] = 0;
}

// ****************************************************************************
//  Method: GetStops
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::GetStops(int *_stops) const
{
    for (int i = 0; i < ndims; i++)
        _stops[i] = stops[i];
    for (int i = ndims; i < 3; i++)
        _stops[i] = 0;
}

// ****************************************************************************
//  Method: GetStrides
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::GetStrides(int *_strides) const
{
    for (int i = 0; i < ndims; i++)
        _strides[i] = strides[i];
    for (int i = ndims; i < 3; i++)
        _strides[i] = 0;
}

// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
bool
avtLogicalSelection::operator==(const avtLogicalSelection &sel) const
{
    if (ndims != sel.ndims)
        return false;

    for (int i = 0; i < ndims; i++)
    {
        if (starts[i] != sel.starts[i])
            return false;
        if (stops[i] != sel.stops[i])
            return false;
        if (strides[i] != sel.strides[i])
            return false;
    }

    return true;
}

// ****************************************************************************
//  Method: Compose 
//
//  Purpose: Composes the given selection with 'this' selection
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtLogicalSelection::Compose(const avtLogicalSelection &sel)
{

    for (int i = 0; i < 3; i++)
    {
        if (sel.starts[i] > starts[i])
            starts[i] = sel.starts[i];

        if (sel.stops[i] != -1)
        {
            if (stops[i] == -1)
                stops[i] = sel.stops[i];
            else
            {
                if (sel.stops[i] < stops[i])
                    stops[i] = sel.stops[i];
            }
        }

        strides[i] = strides[i] * sel.strides[i];
    }

}

// ****************************************************************************
//  Method: FactorBestPowerOf2 
//
//  Purpose: Factors 'this' selection into two pieces the first of which is the
//           maximum power of 2 along each axis. This is useful for something
//           like the ViSUS format that can handle only power-of-2 strides.
//           If a factor is found, the 2 pieces are returned in the arguments
//           and 'true' is returned as the value of the call. Otherwise, the
//           call returns false and the contents of the arguments are
//           unchanged.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************
bool
avtLogicalSelection::FactorBestPowerOf2(avtLogicalSelection &pow2Sel,
                                        avtLogicalSelection &otherSel) const
{

    int pow2Strides[31];
    for (int i = 0; i < 31; i++)
        pow2Strides[i] = 1<<i;

    int pow2SelStrides[3] = {1, 1, 1};

    bool foundFactor = false;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 30; j > 0; j--)
        {
            if (strides[i] % pow2Strides[j] == 0)
            {
                pow2SelStrides[i] = pow2Strides[j];
                foundFactor = true;
                break;
            }
        }
    }

    if (foundFactor == false)
        return false;

    for (int i = 0; i < 3; i++)
    {
        pow2Sel.starts[i] = starts[i];
        pow2Sel.stops[i] = stops[i];
        otherSel.starts[i] = starts[i];
        otherSel.stops[i] = stops[i];
        pow2Sel.strides[i] = pow2SelStrides[i];
        otherSel.strides[i] = strides[i] / pow2SelStrides[i];
    }

    return true;

}
