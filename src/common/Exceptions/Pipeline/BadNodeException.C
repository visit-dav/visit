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
//                             BadNodeException.C                            //
// ************************************************************************* //

#include <BadNodeException.h>
#include <snprintf.h>

using std::vector;
using std::string;

// ****************************************************************************
//  Method: BadNodeException constructor
//
//  Arguments:
//      index        The cell number that was out of range.
//      numCells     The total number of cells.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 13, 2004 
//
// ****************************************************************************

BadNodeException::BadNodeException(int index, int numNodes)
{
    char str[1024];
    SNPRINTF(str, 1024, "Node %d is invalid (%d maximum).", index, numNodes-1);
    msg = str;
}


// ****************************************************************************
//  Method: BadNodeException constructor
//
//  Arguments:
//      index        The node index that was out of range. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 13, 2004 
//
// ****************************************************************************

BadNodeException::BadNodeException(vector<int> &index)
{
    char str[1024];
    if (index.size() == 3)
    {
        SNPRINTF(str, 1024, "Node index (%d %d %d) is invalid.",  
                 index[0], index[1], index[2]);
    }
    else 
    {
        SNPRINTF(str, 1024, "Node index (%d %d) is invalid.", 
                 index[0], index[1]);
    }
    msg = str;
}


// ****************************************************************************
//  Method: BadNodeException constructor
//
//  Arguments:
//    index     The node number that was invalid.
//    numCells  The reason the node was invalid. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 19, 2005 
//
// ****************************************************************************

BadNodeException::BadNodeException(int index, string &reason)
{
    char str[1024];
    SNPRINTF(str, 1024, "Node %d is invalid. %s", index, reason.c_str());
    msg = str;
}

