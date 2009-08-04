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
//                             BadCellException.C                            //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <BadCellException.h>

using std::vector;
using std::string;

// ****************************************************************************
//  Method: BadCellException constructor
//
//  Arguments:
//      index        The cell number that was out of range.
//      numCells     The total number of cells.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2002
//
// ****************************************************************************

BadCellException::BadCellException(int index, int numCells)
{
    char str[1024];
    sprintf(str, "Cell %d is invalid (%d maximum).", index, numCells-1);
    msg = str;
}


// ****************************************************************************
//  Method: BadCellException constructor
//
//  Arguments:
//    index     The cell number that was invalid. 
//    reason    The reason the cell number is invalid. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 18, 2005 
//
// ****************************************************************************

BadCellException::BadCellException(int index, string &reason)
{
    char str[1024];
    sprintf(str, "Cell %d is invalid.  %s", index, reason.c_str());
    msg = str;
}


// ****************************************************************************
//  Method: BadCellException constructor
//
//  Arguments:
//      index        The cell index that was out of range. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2002 
//
// ****************************************************************************

BadCellException::BadCellException(vector<int> &index)
{
    char str[1024];
    if (index.size() == 3)
    {
        sprintf(str, "Cell index (%d %d %d) is invalid.",  
                index[0], index[1], index[2]);
  
    }
    else 
    {
        sprintf(str, "Cell index (%d %d) is invalid.", index[0], index[1]);
  
    }
    msg = str;
}


