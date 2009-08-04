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
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
void
avtLogicalSelection::SetStarts(const int *_starts)
{
    int i;
    for (i = 0; i < ndims; i++)
        starts[i] = _starts[i];
    for (i = ndims; i < 3; i++)
        starts[i] = 0;
}

// ****************************************************************************
//  Method: SetStops
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
void
avtLogicalSelection::SetStops(const int *_stops)
{
    int i;
    for (i = 0; i < ndims; i++)
        stops[i] = _stops[i];
    for (i = ndims; i < 3; i++)
        stops[i] = 0;
}

// ****************************************************************************
//  Method: SetStrides
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
void
avtLogicalSelection::SetStrides(const int *_strides)
{
    int i;
    for (i = 0; i < ndims; i++)
        strides[i] = _strides[i];
    for (i = ndims; i < 3; i++)
        strides[i] = 0;
}

// ****************************************************************************
//  Method: GetStarts
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
void
avtLogicalSelection::GetStarts(int *_starts) const
{
    int i;
    for (i = 0; i < ndims; i++)
        _starts[i] = starts[i];
    for (i = ndims; i < 3; i++)
        _starts[i] = 0;
}

// ****************************************************************************
//  Method: GetStops
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
void
avtLogicalSelection::GetStops(int *_stops) const
{
    int i;
    for (i = 0; i < ndims; i++)
        _stops[i] = stops[i];
    for (i = ndims; i < 3; i++)
        _stops[i] = 0;
}

// ****************************************************************************
//  Method: GetStrides
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
void
avtLogicalSelection::GetStrides(int *_strides) const
{
    int i;
    for (i = 0; i < ndims; i++)
        _strides[i] = strides[i];
    for (i = ndims; i < 3; i++)
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
//  Modifications:
//    Brad Whitlock, Wed Nov 3 09:40:46 PDT 2004
//    Fixed for win32.
//
// ****************************************************************************
bool
avtLogicalSelection::FactorBestPowerOf2(avtLogicalSelection &pow2Sel,
                                        avtLogicalSelection &otherSel) const
{

    int i, pow2Strides[31];
    for (i = 0; i < 31; i++)
        pow2Strides[i] = 1<<i;

    int pow2SelStrides[3] = {1, 1, 1};

    bool foundFactor = false;
    for (i = 0; i < 3; i++)
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

    for (i = 0; i < 3; i++)
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
