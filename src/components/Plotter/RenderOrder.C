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

#include <RenderOrder.h>

static const char *RenderOrderNameLookup[] = {
    "MUST_GO_FIRST",
    "DOES_NOT_MATTER",
    "MUST_GO_LAST",
    "ABSOLUTELY_LAST",
    "MAX_ORDER"
};

// *******************************************************************
// Function: RenderOrder2Int
//
// Purpose:
//   Converts a RenderOrder variable to an integer value.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//   Added ABSOLUTELY_LAST.
//
// *******************************************************************

int
RenderOrder2Int(RenderOrder order)
{
    if(order == MUST_GO_FIRST)
        return 0;
    else if(order == DOES_NOT_MATTER)
        return 1;
    else if(order == MUST_GO_LAST)
        return 2;        
    else if(order == ABSOLUTELY_LAST)
        return 3;        
    else
        return 4;
}

// *******************************************************************
// Function: Int2RenderOrder
//
// Purpose:
//   Converts an integer variable to a RenderOrder value.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//   Added ABSOLUTELY_LAST.
//
// *******************************************************************

RenderOrder
Int2RenderOrder(int order)
{
    if(order == 0)
        return MUST_GO_FIRST;        
    else if(order == 1)
        return DOES_NOT_MATTER;
    else if(order == 2)
        return MUST_GO_LAST;
    else if(order == 3)
        return ABSOLUTELY_LAST;
    else
        return MAX_ORDER;
}


// *******************************************************************
// Function: RenderOrderName
//
// Purpose:
//   Returns the name of the plot order.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// *******************************************************************

const char *
RenderOrderName(RenderOrder order)
{
    return RenderOrderNameLookup[RenderOrder2Int(order)];
}


// *******************************************************************
// Function: RenderOrderName
//
// Purpose:
//   Returns the name of the plot order.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// *******************************************************************

const char *
RenderOrderName(int order)
{
    return RenderOrderNameLookup[order];
}
