/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include "QueryParametersRPC.h" 
#include <string>
using std::string;

// ****************************************************************************
//  Method: QueryParametersRPC::QueryParametersRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011 
//
// ****************************************************************************

QueryParametersRPC::QueryParametersRPC() : BlockingRPC("sa", &mapNodeString), queryName("")
{
    // nothing here;
}


// ****************************************************************************
//  Method: QueryParametersRPC::~QueryParameterRPC
//
//  Purpose: 
//    This is the RPC's destructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011 
//
// ****************************************************************************

QueryParametersRPC::~QueryParametersRPC() 
{
    // nothing here;
}

// ****************************************************************************
// Method: QueryParametersRPC::operator()
//
// Purpose: 
//   Invocation method for the QueryParametersRPC class. Clears the caches that 
//   relate to the specified database.
//
// Arguments:
//   queryName : The name of the query for which we're requesting information.
//
// Programmer: Kathleen Biagas 
// Creation:   July 15, 2011
//
// Modifications:
//   
// ****************************************************************************

string
QueryParametersRPC::operator()(const std::string &queryname)
{
    queryName = queryname;
    Execute();
    return mapNodeString.s;
}

// ****************************************************************************
// Method: QueryParametersRPC::SelectAll
//
// Purpose: 
//   Selects all components in the RPC.
//
// Programmer: Kathleen Biagas 
// Creation:   July 15, 2011 
//
// Modifications:
//   
// ****************************************************************************

void
QueryParametersRPC::SelectAll()
{
    Select(0, (void*)&queryName);
}

// ****************************************************************************
//  Constructor:  QueryParametersRPC::MapNodeString::MapNodeString
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
QueryParametersRPC::MapNodeString::MapNodeString() : AttributeSubject("s")
{
    s = "";
}

// ****************************************************************************
//  Constructor:  QueryParametersRPC::MapNodeString::MapNodeString
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
QueryParametersRPC::MapNodeString::MapNodeString(string s_) : AttributeSubject("s")
{
    s = s_;
}

// ****************************************************************************
//  Destructor:  QueryParametersRPC::MapNodeString::~MapNodeString
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
QueryParametersRPC::MapNodeString::~MapNodeString()
{
}


// ****************************************************************************
//  Destructor:  QueryParametersRPC::MapNodeString::SelectAll
//
//  Purpose:
//    Select all attributes.
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
void
QueryParametersRPC::MapNodeString::SelectAll()
{
    Select(0, (void*)&s);
}


