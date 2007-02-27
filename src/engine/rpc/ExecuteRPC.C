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

#include <ExecuteRPC.h>
#include <string.h>
#include <DebugStream.h>

// ****************************************************************************
//  Constructor: ExecuteRPC::ExecuteRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//     Mark C. Miller, 08Apr03, added respond with null data boolean
// ****************************************************************************

ExecuteRPC::ExecuteRPC() : NonBlockingRPC("b")
{
}

// ****************************************************************************
//  Destructor: ExecuteRPC::~ExecuteRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

ExecuteRPC::~ExecuteRPC()
{
}

// ****************************************************************************
//  Method: ExecuteRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:41:39 PST 2000
//    Made output go to log file instead of cout or cerr.
//
//    Mark C. Miller, 08Apr03, added respond with null data object boolean
//
// ****************************************************************************

void
ExecuteRPC::operator()(bool respond)
{
    debug3 << "Executing ExecuteRPC" << endl;

    SetRespondWithNull(respond);

    Execute();
}


// ****************************************************************************
//  Method: ExecuteRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//     Mark C. Miller, 08Apr03, Added respondWithNullDataObject boolean
//
// ****************************************************************************

void
ExecuteRPC::SelectAll()
{
    Select(0, (void*)&respondWithNullDataObject);
}

// ****************************************************************************
//  Method: ExecuteRPC::SetRespondWithNull
//
//  Purpose: 
//    Sets flag to indicate if execute should respond with null data or not 
//
//  Programmer: Mark C. Miller 
//  Creation:   08Apr03 
// ****************************************************************************

void
ExecuteRPC::SetRespondWithNull(bool respond)
{
   respondWithNullDataObject = respond;
   Select(0, (void*)&respondWithNullDataObject);
}

// ****************************************************************************
//  Method: ExecuteRPC::GetRespondWithNull
//
//  Purpose: 
//    Gets flag indicating if execute should respond with null data or not 
//
//  Programmer: Mark C. Miller 
//  Creation:   08Apr03 
// ****************************************************************************
bool
ExecuteRPC::GetRespondWithNull() const
{
   return respondWithNullDataObject;
}
