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
//                              avtSamplePoints.C                            //
// ************************************************************************* //

#include <avtSamplePoints.h>

#include <avtCellList.h>
#include <avtSamplePointsSource.h>
#include <avtVolume.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtSamplePoints constructor
//
//  Arguments:
//      up      The upstream source for this data object.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Sun Jan  8 12:52:20 PST 2006
//    Initialize useWeightingScheme.
//
// ****************************************************************************

avtSamplePoints::avtSamplePoints(avtDataObjectSource *up)
    : avtDataObject(up)
{
    volume   = NULL;
    celllist = NULL;
    numVars  = -1;
    useWeightingScheme = false;
}


// ****************************************************************************
//  Method: avtSamplePoints destructor
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

avtSamplePoints::~avtSamplePoints()
{
    if (celllist != NULL)
    {
        delete celllist;
    }
    if (volume != NULL)
    {
        delete volume;
    }
}


// ****************************************************************************
//  Method: avtSamplePoints::SetNumberOfVariables
//
//  Purpose:
//      Sets the number of variables that we will sample over.
//
//  Arguments:
//      nv      The number of variables to sample over.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2001
//
// ****************************************************************************

void
avtSamplePoints::SetNumberOfVariables(int nv)
{
    numVars = nv;
}


// ****************************************************************************
//  Method: avtSamplePoints::GetCellList
//
//  Purpose:
//      Gets the cell list.
//
//  Returns:    The cell list.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2001
//
// ****************************************************************************

avtCellList *
avtSamplePoints::GetCellList(void)
{
    if (numVars < 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    if (celllist == NULL)
    {
        celllist = new avtCellList(numVars);
    }

    return celllist;
}


// ****************************************************************************
//  Method: avtSamplePoints::ResetCellList
//
//  Purpose:
//      Resets the cell list.
//
//  Programmer: Hank Childs
//  Creation:   December 12, 2004
//
// ****************************************************************************

void
avtSamplePoints::ResetCellList(void)
{
    if (celllist != NULL)
        delete celllist;
    celllist = NULL;
}


// ****************************************************************************
//  Method: avtSamplePoints::SetVolume
//
//  Purpose:
//      Sets the size of the output volume.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 14:51:34 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Sun Dec  4 19:18:37 PST 2005
//    Add support for weighting schemes.
//
// ****************************************************************************

void
avtSamplePoints::SetVolume(int width, int height, int depth)
{
    if (volume != NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    int nv = numVars;
    if (useWeightingScheme)
        nv++;
    volume = new avtVolume(width, height, depth, nv);
}


