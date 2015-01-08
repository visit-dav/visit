/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <SetBackendTypeRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetBackendTypeRPC::SetBackendTypeRPC
//
// Purpose:
//   This is the constructor.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

SetBackendTypeRPC::SetBackendTypeRPC()
    : BlockingRPC("i",NULL)
{
}


// *******************************************************************
// Denstructor: SetBackendTypeRPC::~SetBackendTypeRPC
//
// Purpose:
//   This is the denstructor.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

SetBackendTypeRPC::~SetBackendTypeRPC()
{
}


// *******************************************************************
// Method: SetBackendTypeRPC::operator()
//
// Purpose:
//   This is the RPC's invocation method.
//
// Arguments:
//   bType : This is the new backend type.
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

void
SetBackendTypeRPC::operator()(const int bType)
{
    SetBackendType(bType);
    Execute();
}


// *******************************************************************
// Method: SetBackendTypeRPC::SelectAll
//
// Purpose:
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

void
SetBackendTypeRPC::SelectAll()
{
    Select(0, (void*)&backendType);
}


// *******************************************************************
// Method: SetBackendTypeRPC::SetBackendType
//
// Purpose:
//   Set the backend type argument of the RPC.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

void
SetBackendTypeRPC::SetBackendType(const int bType)
{
    backendType = bType;
}

// *******************************************************************
// Method: SetBackendTypeRPC::GetBackendType
//
// Purpose:
//   Get the backend type for the RPC.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

int
SetBackendTypeRPC::GetBackendType() const
{
    return backendType;
}

// ****************************************************************************
// Method: SetBackendTypeRPC::TypeName
//
// Purpose:
//   Returns the name of the RPC.
//
// Arguments:
//
// Returns:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// ****************************************************************************

const std::string
SetBackendTypeRPC::TypeName() const
{
    return "SetBackendTypeRPC";
}
