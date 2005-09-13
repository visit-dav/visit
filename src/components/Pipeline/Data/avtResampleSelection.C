// ************************************************************************* //
//                            avtResampleSelection.C                         //
// ************************************************************************* //

#include <limits.h>

#include <avtResampleSelection.h>

// ****************************************************************************
//  Method: avtResampleSelection constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************

avtResampleSelection::avtResampleSelection()
{
    ndims = 3;
    starts[0] = -10.0;
    starts[1] = -10.0;
    starts[2] = -10.0;
    stops[0]  =  10.0;
    stops[1]  =  10.0;
    stops[2]  =  10.0;
    counts[0] = 100;
    counts[1] = 100;
    counts[2] = 100;
}

// ****************************************************************************
//  Method: SetStarts 
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
void
avtResampleSelection::SetStarts(const double *_starts)
{
    int i;
    for (i = 0; i < ndims; i++)
        starts[i] = _starts[i];
    for (i = ndims; i < 3; i++)
        starts[i] = 0.0;
}

// ****************************************************************************
//  Method: SetStops
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
void
avtResampleSelection::SetStops(const double *_stops)
{
    int i;
    for (i = 0; i < ndims; i++)
        stops[i] = _stops[i];
    for (i = ndims; i < 3; i++)
        stops[i] = 0.0;
}

// ****************************************************************************
//  Method: SetCounts
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
void
avtResampleSelection::SetCounts(const int *_counts)
{
    int i;
    for (i = 0; i < ndims; i++)
        counts[i] = _counts[i];
    for (i = ndims; i < 3; i++)
        counts[i] = 0;
}

// ****************************************************************************
//  Method: GetStarts
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
void
avtResampleSelection::GetStarts(double *_starts) const
{
    int i;
    for (i = 0; i < ndims; i++)
        _starts[i] = starts[i];
    for (i = ndims; i < 3; i++)
        _starts[i] = 0.0;
}

// ****************************************************************************
//  Method: GetStops
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
void
avtResampleSelection::GetStops(double *_stops) const
{
    int i;
    for (i = 0; i < ndims; i++)
        _stops[i] = stops[i];
    for (i = ndims; i < 3; i++)
        _stops[i] = 0.0;
}

// ****************************************************************************
//  Method: GetCounts
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
void
avtResampleSelection::GetCounts(int *_counts) const
{
    int i;
    for (i = 0; i < ndims; i++)
        _counts[i] = counts[i];
    for (i = ndims; i < 3; i++)
        _counts[i] = 0;
}

// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005    
//
// ****************************************************************************
bool
avtResampleSelection::operator==(const avtResampleSelection &sel) const
{
    if (ndims != sel.ndims)
        return false;

    for (int i = 0; i < ndims; i++)
    {
        if (starts[i] != sel.starts[i])
            return false;
        if (stops[i] != sel.stops[i])
            return false;
        if (counts[i] != sel.counts[i])
            return false;
    }

    return true;
}
