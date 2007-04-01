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
// ****************************************************************************

RenderRPC::RenderRPC() : NonBlockingRPC("i*bb")
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
// ****************************************************************************

void
RenderRPC::operator()(const intVector& ids_, bool sendZBuffer_,
    bool do3DAnnotsOnly_)
{
    SetIDs(ids_);
    SetSendZBuffer(sendZBuffer_);
    SetDo3DAnnotsOnly(do3DAnnotsOnly_);

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
// ****************************************************************************

void
RenderRPC::SelectAll()
{
    Select(0, (void*)&ids);
    Select(1, (void*)&sendZBuffer);
    Select(2, (void*)&do3DAnnotsOnly);
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
RenderRPC::SetDo3DAnnotsOnly(bool do3DAnnotsOnly_)
{
    do3DAnnotsOnly = do3DAnnotsOnly_;
    Select(2, (void*)&do3DAnnotsOnly);
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

bool
RenderRPC::GetDo3DAnnotsOnly() const
{
    return do3DAnnotsOnly;
}

