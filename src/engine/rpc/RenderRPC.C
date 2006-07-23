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

#include "RenderRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: RenderRPC::RenderRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Mon Mar 29 14:11:42 PST 2004
//    Added bool for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Brad Whitlock, Thu Jan 13 13:44:47 PST 2005
//    Initialized member values.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

RenderRPC::RenderRPC() : NonBlockingRPC("i*biib"), ids()
{
    sendZBuffer = false;
    annotMode = 0;
    windowID = 0;
    leftEye = true;
}

// ****************************************************************************
//  Destructor: RenderRPC::~RenderRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
// ****************************************************************************

RenderRPC::~RenderRPC()
{
}

// ****************************************************************************
//  Method: RenderRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Mon Mar 29 14:11:42 PST 2004
//    Added bool for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

void
RenderRPC::operator()(const intVector& ids_, bool sendZBuffer_,
    int annotMode_, int windowID_, bool leftEye_)
{
    SetIDs(ids_);
    SetSendZBuffer(sendZBuffer_);
    SetAnnotMode(annotMode_);
    SetWindowID(windowID_);
    SetLeftEye(leftEye_);

    Execute();
}

// ****************************************************************************
//  Method: RenderRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

void
RenderRPC::SelectAll()
{
    Select(0, (void*)&ids);
    Select(1, (void*)&sendZBuffer);
    Select(2, (void*)&annotMode);
    Select(3, (void*)&windowID);
    Select(4, (void*)&leftEye);
}


// ****************************************************************************
//  Method: RenderRPC::SetXXX methods
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    ids_         the ids
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//    Mark C. Miller, added method for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Brad Whitlock, Thu Jan 13 13:45:17 PST 2005
//    Fixed critical bug in SetWindowID where the address of the argument was
//    being taken for the Select call.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

void
RenderRPC::SetIDs(const intVector& ids_)
{
    ids = ids_;
    Select(0, (void*)&ids);
}

void
RenderRPC::SetSendZBuffer(bool sendZBuffer_)
{
    sendZBuffer = sendZBuffer_;
    Select(1, (void*)&sendZBuffer);
}

void
RenderRPC::SetAnnotMode(int annotMode_)
{
    annotMode = annotMode_;
    Select(2, (void*)&annotMode);
}

void
RenderRPC::SetWindowID(int windowID_)
{
    windowID = windowID_;
    Select(3, (void*)&windowID);
}

void
RenderRPC::SetLeftEye(bool leftEye_)
{
    leftEye = leftEye_;
    Select(4, (void*)&leftEye);
}



// ****************************************************************************
//  Method: RenderRPC::GetXXX methods
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
//
//  Modifications:
//    Mark C. Miller, added method for 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
// ****************************************************************************

const intVector&
RenderRPC::GetIDs() const
{
    return ids;
}

bool
RenderRPC::GetSendZBuffer() const
{
    return sendZBuffer;
}

int
RenderRPC::GetAnnotMode() const
{
    return annotMode;
}

int
RenderRPC::GetWindowID() const
{
    return windowID;
}

bool
RenderRPC::GetLeftEye() const
{
    return leftEye;
}
