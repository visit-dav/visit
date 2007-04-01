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
// ****************************************************************************

RenderRPC::RenderRPC() : NonBlockingRPC("i*bii")
{
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
// ****************************************************************************

void
RenderRPC::operator()(const intVector& ids_, bool sendZBuffer_,
    int annotMode_, int windowID_)
{
    SetIDs(ids_);
    SetSendZBuffer(sendZBuffer_);
    SetAnnotMode(annotMode_);
    SetWindowID(windowID_);

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
// ****************************************************************************

void
RenderRPC::SelectAll()
{
    Select(0, (void*)&ids);
    Select(1, (void*)&sendZBuffer);
    Select(2, (void*)&annotMode);
    Select(3, (void*)&windowID);
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
    Select(3, (void*)&windowID_);
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
