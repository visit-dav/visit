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
// ****************************************************************************

RenderRPC::RenderRPC() : NonBlockingRPC("i*b")
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
// ****************************************************************************

void
RenderRPC::operator()(const intVector& ids_, bool sendZBuffer_)
{
    SetIDs(ids_);
    SetSendZBuffer(sendZBuffer_);

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
}


// ****************************************************************************
//  Method: RenderRPC::SetIDs
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



// ****************************************************************************
//  Method: RenderRPC::GetIDs
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03
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

