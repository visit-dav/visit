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

#include "ApplyOperatorRPC.h"

#include <Xfer.h>

#include <string>
using std::string;

// ----------------------------------------------------------------------------
//                           class PrepareOperatorRPC
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Method:  PrepareOperatorRPC::PrepareOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PrepareOperatorRPC::PrepareOperatorRPC() 
    : BlockingRPC("s")
{
    id = "unknown";
    applyOperatorRPC = NULL;
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::~PrepareOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PrepareOperatorRPC::~PrepareOperatorRPC() 
{
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::SetApplyOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PrepareOperatorRPC::SetApplyOperatorRPC(ApplyOperatorRPC *rpc)
{
    applyOperatorRPC = rpc;
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::operator
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PrepareOperatorRPC::operator()(const std::string &n)
{
    id = n;
    Select(0, (void*)&id);
    Execute();
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PrepareOperatorRPC::SelectAll()
{
    Select(0, (void*)&id);
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
string
PrepareOperatorRPC::GetID()
{
    return id;
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::GetApplyOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
ApplyOperatorRPC*
PrepareOperatorRPC::GetApplyOperatorRPC()
{
    return applyOperatorRPC;
}


// ----------------------------------------------------------------------------
//                           class ApplyOperatorRPC
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:      : BlockingRPC("a
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
ApplyOperatorRPC::ApplyOperatorRPC() 
    : BlockingRPC("a")
{
    atts = NULL;
    prepareOperatorRPC.SetApplyOperatorRPC(this);
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::~ApplyOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
ApplyOperatorRPC::~ApplyOperatorRPC()
{
    if (atts)
        delete atts;
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::SetAtts
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
ApplyOperatorRPC::SetAtts(AttributeSubject *a)
{
    if (atts)
        delete atts;
    atts = a;
    if (atts)
        Select(0, (void*)atts);
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::SetXfer
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
ApplyOperatorRPC::SetXfer(Xfer *x)
{
    VisItRPC::SetXfer(x);
    xfer->Add(&prepareOperatorRPC);
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
ApplyOperatorRPC::SelectAll()
{
    // nothing
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
std::string
ApplyOperatorRPC::GetID()
{
    return prepareOperatorRPC.GetID();
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::GetAtts
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
AttributeSubject*
ApplyOperatorRPC::GetAtts()
{
    return atts;
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::GetPrepareOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PrepareOperatorRPC&
ApplyOperatorRPC::GetPrepareOperatorRPC()
{
    return prepareOperatorRPC;
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::operator
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 21 11:09:54 PST 2002
//    Added propogation of error from preparatory RPC.
//
// ****************************************************************************
void
ApplyOperatorRPC::operator()(const string &n, const AttributeSubject *a)
{
    prepareOperatorRPC(n);
    if (prepareOperatorRPC.GetStatus() == VisItRPC::error)
    {
        reply.SetStatus(VisItRPC::error);
        reply.SetMessage(prepareOperatorRPC.Message());
        reply.SetExceptionType(prepareOperatorRPC.GetExceptionType());
        return;
    }

    if (a)
        Select(0, (void*)a);

    Execute();
}
