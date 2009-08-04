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

#include "NamedSelectionRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: NamedSelectionRPC::NamedSelectionRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

NamedSelectionRPC::NamedSelectionRPC() : BlockingRPC("is*si")
{
}

// ****************************************************************************
//  Destructor: NamedSelectionRPC::~NamedSelectionRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

NamedSelectionRPC::~NamedSelectionRPC()
{
}

// ****************************************************************************
//  Method: NamedSelectionRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::operator()(NamedSelectionType t, 
                              const std::vector<std::string> &ids,
                              const std::string &n)
{
    SetPlotNames(ids);
    SetSelectionName(n);
    SetNamedSelectionType(t);

    Execute();
}

// ****************************************************************************
//  Method: NamedSelectionRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::operator()(NamedSelectionType t, int id,
                              const std::string &n)
{
    SetPlotID(id);
    SetSelectionName(n);
    SetNamedSelectionType(t);

    Execute();
}

// ****************************************************************************
//  Method: NamedSelectionRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::operator()(NamedSelectionType t, const std::string &n)
{
    SetSelectionName(n);
    SetNamedSelectionType(t);

    Execute();
}

// ****************************************************************************
//  Method: NamedSelectionRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SelectAll()
{
    Select(0, (void*)&plotId);
    Select(1, (void*)&plotNames);
    Select(2, (void*)&selName);
    Select(3, (void*)&selType);
}


// ****************************************************************************
//  Method: NamedSelectionRPC::SetPlotID
//
//  Purpose: 
//    This sets the plot (network) ID.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetPlotID(int id)
{
    plotId = id;
    Select(0, (void*)&plotId);
}


// ****************************************************************************
//  Method: NamedSelectionRPC::SetPlotNames
//
//  Purpose: 
//    This sets the plot names.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetPlotNames(const std::vector<std::string> &ids)
{
    plotNames = ids;
    Select(1, (void*)&plotNames);
}


// ****************************************************************************
//  Method: NamedSelectionRPC::SetSelectionName
//
//  Purpose: 
//    This sets the selection name.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetSelectionName(const std::string &n)
{
    selName = n;
    Select(2, (void*)&selName);
}


// ****************************************************************************
//  Method: NamedSelectionRPC::SetNamedSelectionType
//
//  Purpose: 
//    This sets the selection type.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetNamedSelectionType(NamedSelectionType t)
{
    selType = t;
    Select(3, (void*)&selType);
}


