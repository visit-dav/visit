#include "UseNetworkRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: UseNetworkRPC::UseNetworkRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

UseNetworkRPC::UseNetworkRPC() : BlockingRPC("i")
{
}

// ****************************************************************************
//  Destructor: UseNetworkRPC::~UseNetworkRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

UseNetworkRPC::~UseNetworkRPC()
{
}

// ****************************************************************************
//  Method: UseNetworkRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
//  Modifications:
// ****************************************************************************

void
UseNetworkRPC::operator()(int id_)
{
    SetID(id_);

    Execute();
}

// ****************************************************************************
//  Method: UseNetworkRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

void
UseNetworkRPC::SelectAll()
{
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: UseNetworkRPC::SetID
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

void
UseNetworkRPC::SetID(int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: UseNetworkRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
// ****************************************************************************

int
UseNetworkRPC::GetID() const
{
    return id;
}

