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

#include "PickRPC.h"


// ****************************************************************************
//  Constructor: PickRPC::PickRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

PickRPC::PickRPC() : BlockingRPC("iai", &returnAtts)
{
}

// ****************************************************************************
//  Destructor: PickRPC::~PickRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

PickRPC::~PickRPC()
{
}

// ****************************************************************************
//  Method: PickRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
//  Modifications:
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
// ****************************************************************************

PickAttributes
PickRPC::operator()(const int nid, const PickAttributes *atts, int wid)
{
    SetNetId(nid);
    SetPickAtts(atts);
    SetWinId(wid);
    Execute();

    return returnAtts;
}

// ****************************************************************************
//  Method: PickRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
// ****************************************************************************

void
PickRPC::SelectAll()
{
    Select(0, (void*)&netId);
    Select(1, (void*)&pickAtts);
    Select(2, (void*)&winId);
}


// ****************************************************************************
//  Method: PickRPC::SetPickAtts
//
//  Purpose: 
//    This sets the pickAttsdow atts.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
PickRPC::SetPickAtts(const PickAttributes *atts)
{
    pickAtts = *atts;
    Select(1, (void*)&pickAtts);
}


// ****************************************************************************
//  Method: PickRPC::GetPickAtts
//
//  Purpose: 
//    This returns the pickAttsdow atts.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

PickAttributes*
PickRPC::GetPickAtts() 
{
    return &pickAtts;
}


// ****************************************************************************
//  Method: PickRPC::SetNetId
//
//  Purpose: 
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
PickRPC::SetNetId(const int nid)
{
    netId = nid;
    Select(0, (void*)&netId);
}

// ****************************************************************************
//  Method: PickRPC::SetWinId
//
//  Programmer: Mark C. Miller 
//  Creation:   December 15, 2004 
//
// ****************************************************************************
void
PickRPC::SetWinId(const int wid)
{
    winId = wid;
    Select(2, (void*)&winId);
}


// ****************************************************************************
//  Method: PickRPC::GetNetId
//
//  Purpose: 
//    This returns the net id.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 20, 2001
//
// ****************************************************************************

int
PickRPC::GetNetId() 
{
    return netId;
}

// ****************************************************************************
//  Method: PickRPC::SetWinId
//
//  Programmer: Mark C. Miller 
//  Creation:   December 15, 2004 
//
// ****************************************************************************
int
PickRPC::GetWinId() const
{
    return winId;
}
