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

#include <SetPrecisionTypeRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetPrecisionTypeRPC::SetPrecisionTypeRPC
//
// Purpose:
//   This is the constructor.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

SetPrecisionTypeRPC::SetPrecisionTypeRPC()
    : BlockingRPC("i",NULL)
{
}


// *******************************************************************
// Denstructor: SetPrecisionTypeRPC::~SetPrecisionTypeRPC
//
// Purpose:
//   This is the denstructor.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

SetPrecisionTypeRPC::~SetPrecisionTypeRPC()
{
}


// *******************************************************************
// Method: SetPrecisionTypeRPC::operator()
//
// Purpose:
//   This is the RPC's invocation method.
//
// Arguments:
//   pType : This is the new precision type.
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

void
SetPrecisionTypeRPC::operator()(const int pType)
{
    debug3 << "Executing SetPrecisionTypeRPC\n";

    SetPrecisionType(pType);
    Execute();
}


// *******************************************************************
// Method: SetPrecisionTypeRPC::SelectAll
//
// Purpose:
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

void
SetPrecisionTypeRPC::SelectAll()
{
    Select(0, (void*)&precisionType);
}


// *******************************************************************
// Method: SetPrecisionTypeRPC::SetPrecisionType
//
// Purpose:
//   Set the precision type argument of the RPC.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

void
SetPrecisionTypeRPC::SetPrecisionType(const int pType)
{
    precisionType = pType;
}

// *******************************************************************
// Method: SetPrecisionTypeRPC::GetPrecisionType
//
// Purpose:
//   Get the precision type for the RPC.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

int
SetPrecisionTypeRPC::GetPrecisionType() const
{
    return precisionType;
}

// ****************************************************************************
// Method: SetPrecisionTypeRPC::TypeName
//
// Purpose:
//   Returns the name of the RPC.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// ****************************************************************************

const std::string
SetPrecisionTypeRPC::TypeName() const
{
    return "SetPrecisionTypeRPC";
}
