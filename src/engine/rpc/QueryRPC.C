/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include "QueryRPC.h" 


// ****************************************************************************
//  Method: QueryRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 6, 2002 
//
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:25:55 PDT 2003
//    Added '*' to 'i*' since we now have a vector of ints.
//
// ****************************************************************************

QueryRPC::QueryRPC() : NonBlockingRPC("i*a", &returnAtts)
{
  // nothing here;
}


// ****************************************************************************
//  Method: QueryRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 6, 2002 
//
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:25:55 PDT 2003
//    Renamed networkId to networkIds.
//
// ****************************************************************************

void
QueryRPC::operator()(const std::vector<int> &netIds,
                     const QueryAttributes *atts)
{
    SetNetworkIds(netIds);
    SetQueryAtts(atts);
    Execute();
}


// ****************************************************************************
//  Method: QueryRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 6, 2002 
//
// ****************************************************************************

void
QueryRPC::SelectAll()
{
    Select(0, (void*)&networkIds);
    Select(1, (void*)&queryAtts);
}

 
// ****************************************************************************
//  Method: QueryRPC::SetQueryAtts
//
//  Purpose:
//    This sets the query atts.
//
//  Arguments:
//    atts      the query atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 20, 2002 
//
// ****************************************************************************
 
void
QueryRPC::SetQueryAtts(const QueryAttributes *atts)
{
    queryAtts = *atts;
    Select(1, (void*)&queryAtts);
}


// ****************************************************************************
//  Method: QueryRPC::GetQueryAtts
//
//  Purpose:
//    This returns the query atts.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 20, 2002
//
// ****************************************************************************
 
QueryAttributes*
QueryRPC::GetQueryAtts()
{
    return &queryAtts;
}
