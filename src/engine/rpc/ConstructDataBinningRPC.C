// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ConstructDataBinningRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: ConstructDataBinningRPC::ConstructDataBinningRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

ConstructDataBinningRPC::ConstructDataBinningRPC() : BlockingRPC("ia")
{
}

// ****************************************************************************
//  Destructor: ConstructDataBinningRPC::~ConstructDataBinningRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

ConstructDataBinningRPC::~ConstructDataBinningRPC()
{
}

// ****************************************************************************
//  Method: ConstructDataBinningRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//    origData  the type of input 
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
//  Modifications:
// ****************************************************************************

void
ConstructDataBinningRPC::operator()(const int id_, const ConstructDataBinningAttributes *atts)
{
    SetID(id_);
    SetConstructDataBinningAtts(atts);

    Execute();
}

// ****************************************************************************
//  Method: ConstructDataBinningRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

void
ConstructDataBinningRPC::SelectAll()
{
    Select(0, (void*)&id);
    Select(1, (void*)&constructDataBinningAtts);
}


// ****************************************************************************
//  Method: ConstructDataBinningRPC::SetID
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

void
ConstructDataBinningRPC::SetID(const int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ConstructDataBinningRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

int
ConstructDataBinningRPC::GetID() const
{
    return id;
}

// ****************************************************************************
//  Method: ConstructDataBinningRPC::SetConstructDataBinningAtts 
//
//  Purpose:
//    This sets the constructDataBinning atts.
//
//  Arguments:
//    atts      the constructDataBinning atts
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006 
//
// ****************************************************************************
 
void
ConstructDataBinningRPC::SetConstructDataBinningAtts(const ConstructDataBinningAttributes *atts)
{
    constructDataBinningAtts = *atts;
    Select(1, (void*)&constructDataBinningAtts);
}


// ****************************************************************************
//  Method: ConstructDataBinningRPC::GetConstructDataBinningAtts 
//
//  Purpose:
//    This returns the constructDataBinning atts.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006 
//
// ****************************************************************************
 
ConstructDataBinningAttributes *
ConstructDataBinningRPC::GetConstructDataBinningAtts()
{
    return &constructDataBinningAtts;
}
