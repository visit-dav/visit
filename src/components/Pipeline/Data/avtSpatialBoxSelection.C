// ************************************************************************* //
//                            avtSpatialBoxSelection.C                       //
// ************************************************************************* //

#include <float.h>

#include <avtSpatialBoxSelection.h>

// ****************************************************************************
//  Method: avtSpatialBoxSelection constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************

avtSpatialBoxSelection::avtSpatialBoxSelection()
{
    for (int i = 0; i < 3; i++)
    {
        mins[i] = -FLT_MAX;
        maxs[i] = +FLT_MAX;
    }
}

// ****************************************************************************
//  Method: SetMins
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************
void
avtSpatialBoxSelection::SetMins(const double *_mins)
{
    for (int i = 0; i < 3; i++)
        mins[i] = _mins[i];
}

// ****************************************************************************
//  Method: SetMaxs
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************
void
avtSpatialBoxSelection::SetMaxs(const double *_maxs)
{
    for (int i = 0; i < 3; i++)
        maxs[i] = _maxs[i];
}

// ****************************************************************************
//  Method: GetMins
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************
void
avtSpatialBoxSelection::GetMins(double *_mins) const
{
    for (int i = 0; i < 3; i++)
        _mins[i] = mins[i];
}

// ****************************************************************************
//  Method: GetMaxs
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************
void
avtSpatialBoxSelection::GetMaxs(double *_maxs) const
{
    for (int i = 0; i < 3; i++)
        _maxs[i] = maxs[i];
}

// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 26, 2004 
//
// ****************************************************************************
bool
avtSpatialBoxSelection::operator==(const avtSpatialBoxSelection &sel) const
{
    if (inclusionMode != sel.inclusionMode)
        return false;

    for (int i = 0; i < 3; i++)
    {
        if (mins[i] != sel.mins[i])
            return false;
        if (maxs[i] != sel.maxs[i])
            return false;
    }

    return true;
}

// ****************************************************************************
//  Method: GetLogicalBounds 
//
//  Purpose: Given rectilinear coordinate arrays, this routine determines the
//           logical, inclusive bounds of the selection.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtSpatialBoxSelection::GetLogicalBounds(const int nodeCounts[3],
    double *nodalCoords[3], int *firstZone, int *lastZone) const
{
    for (int i = 0; i < 3; i++)
    {
        // march forward from 0 to find min crossing
        int j = 0;
        while ((j < nodeCounts[i]) && (nodalCoords[i][j] < mins[i]))
            j++;
        if (j == nodeCounts[i])
            firstZone[i] = -1;
        else
        {
            if ((inclusionMode != Whole) && (j > 0) &&
                (nodalCoords[i][j] != mins[i]))
                j--;
            firstZone[i] = j;
        }

        // march backwards from end to find max crossing
        // we don't go all the way to zeroth node here because were
        // returning zone-numbers which, for the upper bound, are 1 less than
        // node numbers
        j = nodeCounts[i] - 1;
        while ((j > 0) && (nodalCoords[i][j] > maxs[i]))
            j--;
        if (j == 0)
            lastZone[i] = -1;
        else
        {
            if ((inclusionMode != Whole) && (j < nodeCounts[i]-1) &&
                (nodalCoords[i][j] != maxs[i]))
                j++;
            lastZone[i] = j - 1; // inclusive, max zone # is 1 less than node #
        }
    }
}

// ****************************************************************************
//  Method: Compose 
//
//  Purpose: Composes the given selection with 'this' selection producing a
//           new selection
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************
void
avtSpatialBoxSelection::Compose(const avtSpatialBoxSelection &sel,
                                      avtSpatialBoxSelection &composeSel)
{

    for (int i = 0; i < 3; i++)
    {
        if (mins[i] > sel.mins[i])
            composeSel.mins[i] = mins[i];
        else
            composeSel.mins[i] = sel.mins[i];

        if (maxs[i] < sel.maxs[i])
            composeSel.maxs[i] = maxs[i];
        else
            composeSel.maxs[i] = sel.maxs[i];
    }
}
