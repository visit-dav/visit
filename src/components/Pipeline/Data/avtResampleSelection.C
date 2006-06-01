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
