#include "ReleaseDataRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: ReleaseDataRPC::ReleaseDataRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2002
//
// ****************************************************************************

ReleaseDataRPC::ReleaseDataRPC() : BlockingRPC("i")
{
}

// ****************************************************************************
//  Destructor: ReleaseDataRPC::~ReleaseDataRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2002
//
// ****************************************************************************

ReleaseDataRPC::~ReleaseDataRPC()
{
}

// ****************************************************************************
//  Method: ReleaseDataRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2002
//
//  Modifications:
// ****************************************************************************

void
ReleaseDataRPC::operator()(int id_)
{
    SetID(id_);

    Execute();
}

// ****************************************************************************
//  Method: ReleaseDataRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2002
//
// ****************************************************************************

void
ReleaseDataRPC::SelectAll()
{
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ReleaseDataRPC::SetID
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2002
//
// ****************************************************************************

void
ReleaseDataRPC::SetID(int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ReleaseDataRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2002
//
// ****************************************************************************

int
ReleaseDataRPC::GetID() const
{
    return id;
}

