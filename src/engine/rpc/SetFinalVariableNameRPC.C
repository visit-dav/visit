#include "SetFinalVariableNameRPC.h"

// ****************************************************************************
//  Method: SetFinalVariableNameRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 13 15:03:33 PDT 2002
//
//  Modifications:
// ****************************************************************************
SetFinalVariableNameRPC::SetFinalVariableNameRPC() : BlockingRPC("s")
{
    // Nothing
}

// ****************************************************************************
//  Method: SetFinalVariableNameRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    name        the final variable name
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 13 15:03:37 PDT 2002
//
//  Modifications:
// ****************************************************************************
void
SetFinalVariableNameRPC::operator()(const std::string &inname)
{
    SetName(inname);

    Execute();
}

// ****************************************************************************
//  Method: SetFinalVariableNameRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 13 15:03:38 PDT 2002
//
// ****************************************************************************
void
SetFinalVariableNameRPC::SelectAll()
{
    Select(0, (void*)&name);
}
