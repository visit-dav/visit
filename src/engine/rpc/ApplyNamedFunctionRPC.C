#include "ApplyNamedFunctionRPC.h"

// ****************************************************************************
//  Method: ApplyNamedFunctionRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 18 17:29:36 PDT 2002
//
//  Modifications:
// ****************************************************************************
ApplyNamedFunctionRPC::ApplyNamedFunctionRPC() : BlockingRPC("si")
{
    // Nothing
}

// ****************************************************************************
//  Method: ApplyNamedFunctionRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 18 17:29:36 PDT 2002
//
//  Modifications:
// ****************************************************************************
void
ApplyNamedFunctionRPC::operator()(const std::string &inname,
                                  const int innargs)
{
    SetName(inname);
    SetNArgs(innargs);

    Execute();
}

// ****************************************************************************
//  Method: ApplyNamedFunctionRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 18 17:29:38 PDT 2002
//
// ****************************************************************************
void
ApplyNamedFunctionRPC::SelectAll()
{
    Select(0, (void*)&name);
    Select(1, (void*)&nargs);
}
