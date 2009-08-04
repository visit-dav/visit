/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
