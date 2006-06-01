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

#include "StartPickRPC.h"


// ****************************************************************************
//  Constructor: StartPickRPC::StartPickRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added another bool arg.
//
// ****************************************************************************

StartPickRPC::StartPickRPC() : BlockingRPC("bbi")
{
}

// ****************************************************************************
//  Destructor: StartPickRPC::~StartPickRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

StartPickRPC::~StartPickRPC()
{
}

// ****************************************************************************
//  Method: StartPickRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    atts      the pickAttsdow atts
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added bool forZones arg.
//
// ****************************************************************************

void
StartPickRPC::operator()(const bool forZones, const bool flag, const int nid)
{
    SetForZones(forZones);
    SetStartFlag(flag);
    SetNetId(nid);

    Execute();
}

// ****************************************************************************
//  Method: StartPickRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added forZones.
//
// ****************************************************************************

void
StartPickRPC::SelectAll()
{
    Select(0, (void*)&forZones);
    Select(1, (void*)&startFlag);
    Select(2, (void*)&netId);
}


// ****************************************************************************
//  Method: StartPickRPC::SetForZones
//
//  Purpose:  Set the value of forZones.
//
//  Arguments:
//    forZones  If true, pick mode is for zone picking, otherwise pick mode
//              is for node picking.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

void
StartPickRPC::SetForZones(const bool forZonesFlag)
{
    forZones = forZonesFlag;
    Select(0, (void*)&forZones);
}


// ****************************************************************************
//  Method: StartPickRPC::GetForZones
//
//  Purpose: 
//    This returns the flag specifying picking is for zones or nodes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

bool
StartPickRPC::GetForZones() 
{
    return forZones;
}


// ****************************************************************************
//  Method: StartPickRPC::SetStartFlag
//
//  Purpose:  Set the value of startFlag.
//
//  Arguments:
//    flag      If true, start pick mode.  Stop otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

void
StartPickRPC::SetStartFlag(const bool flag)
{
    startFlag = flag;
    Select(1, (void*)&startFlag);
}


// ****************************************************************************
//  Method: StartPickRPC::GetStartFlag
//
//  Purpose: 
//    This returns the flag specifying whether to start or stop pick mode.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

bool
StartPickRPC::GetStartFlag() 
{
    return startFlag;
}


// ****************************************************************************
//  Method: StartPickRPC::SetNetId
//
//  Purpose: 
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

void
StartPickRPC::SetNetId(const int nid)
{
    netId = nid;
    Select(2, (void*)&netId);
}


// ****************************************************************************
//  Method: StartPickRPC::GetNetId
//
//  Purpose: 
//    This returns the net id.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2001
//
// ****************************************************************************

int
StartPickRPC::GetNetId() 
{
    return netId;
}

