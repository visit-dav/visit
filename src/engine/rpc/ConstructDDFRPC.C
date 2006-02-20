#include "ConstructDDFRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: ConstructDDFRPC::ConstructDDFRPC
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

ConstructDDFRPC::ConstructDDFRPC() : BlockingRPC("ia")
{
}

// ****************************************************************************
//  Destructor: ConstructDDFRPC::~ConstructDDFRPC
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

ConstructDDFRPC::~ConstructDDFRPC()
{
}

// ****************************************************************************
//  Method: ConstructDDFRPC::operator()
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
ConstructDDFRPC::operator()(const int id_, const ConstructDDFAttributes *atts)
{
    SetID(id_);
    SetConstructDDFAtts(atts);

    Execute();
}

// ****************************************************************************
//  Method: ConstructDDFRPC::SelectAll
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
ConstructDDFRPC::SelectAll()
{
    Select(0, (void*)&id);
    Select(1, (void*)&constructDDFAtts);
}


// ****************************************************************************
//  Method: ConstructDDFRPC::SetID
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
ConstructDDFRPC::SetID(const int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ConstructDDFRPC::GetID
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
ConstructDDFRPC::GetID() const
{
    return id;
}

// ****************************************************************************
//  Method: ConstructDDFRPC::SetConstructDDFAtts 
//
//  Purpose:
//    This sets the constructDDF atts.
//
//  Arguments:
//    atts      the constructDDF atts
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006 
//
// ****************************************************************************
 
void
ConstructDDFRPC::SetConstructDDFAtts(const ConstructDDFAttributes *atts)
{
    constructDDFAtts = *atts;
    Select(1, (void*)&constructDDFAtts);
}


// ****************************************************************************
//  Method: ConstructDDFRPC::GetConstructDDFAtts 
//
//  Purpose:
//    This returns the constructDDF atts.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006 
//
// ****************************************************************************
 
ConstructDDFAttributes *
ConstructDDFRPC::GetConstructDDFAtts()
{
    return &constructDDFAtts;
}
