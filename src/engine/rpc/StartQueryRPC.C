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

#include "StartQueryRPC.h"


// ****************************************************************************
//  Constructor: StartQueryRPC::StartQueryRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

StartQueryRPC::StartQueryRPC() : BlockingRPC("bi")
{
}

// ****************************************************************************
//  Destructor: StartQueryRPC::~StartQueryRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

StartQueryRPC::~StartQueryRPC()
{
}

// ****************************************************************************
//  Method: StartQueryRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::operator()(const bool flag, const int nid)
{
    SetStartFlag(flag);
    SetNetId(nid);

    Execute();
}

// ****************************************************************************
//  Method: StartQueryRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::SelectAll()
{
    Select(0, (void*)&startFlag);
    Select(1, (void*)&netId);
}


// ****************************************************************************
//  Method: StartQueryRPC::SetStartFlag
//
//  Purpose:  Set the value of startFlag.
//
//  Arguments:
//    flag      If true, start pick mode.  Stop otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::SetStartFlag(const bool flag)
{
    startFlag = flag;
    Select(0, (void*)&startFlag);
}


// ****************************************************************************
//  Method: StartQueryRPC::GetStartFlag
//
//  Purpose: 
//    This returns the flag specifying whether to start or stop pick mode.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

bool
StartQueryRPC::GetStartFlag() 
{
    return startFlag;
}


// ****************************************************************************
//  Method: StartQueryRPC::SetNetId
//
//  Purpose: 
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

void
StartQueryRPC::SetNetId(const int nid)
{
    netId = nid;
    Select(1, (void*)&netId);
}


// ****************************************************************************
//  Method: StartQueryRPC::GetNetId
//
//  Purpose: 
//    This returns the net id.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************

int
StartQueryRPC::GetNetId() 
{
    return netId;
}

