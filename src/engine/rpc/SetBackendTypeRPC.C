// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetBackendTypeRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetBackendTypeRPC::SetBackendTypeRPC
//
// Purpose:
//   This is the constructor.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

SetBackendTypeRPC::SetBackendTypeRPC()
    : BlockingRPC("i",NULL)
{
}


// *******************************************************************
// Denstructor: SetBackendTypeRPC::~SetBackendTypeRPC
//
// Purpose:
//   This is the denstructor.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

SetBackendTypeRPC::~SetBackendTypeRPC()
{
}


// *******************************************************************
// Method: SetBackendTypeRPC::operator()
//
// Purpose:
//   This is the RPC's invocation method.
//
// Arguments:
//   bType : This is the new backend type.
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

void
SetBackendTypeRPC::operator()(const int bType)
{
    SetBackendType(bType);
    Execute();
}


// *******************************************************************
// Method: SetBackendTypeRPC::SelectAll
//
// Purpose:
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

void
SetBackendTypeRPC::SelectAll()
{
    Select(0, (void*)&backendType);
}


// *******************************************************************
// Method: SetBackendTypeRPC::SetBackendType
//
// Purpose:
//   Set the backend type argument of the RPC.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

void
SetBackendTypeRPC::SetBackendType(const int bType)
{
    backendType = bType;
}

// *******************************************************************
// Method: SetBackendTypeRPC::GetBackendType
//
// Purpose:
//   Get the backend type for the RPC.
//
// Arguments:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// *******************************************************************

int
SetBackendTypeRPC::GetBackendType() const
{
    return backendType;
}

// ****************************************************************************
// Method: SetBackendTypeRPC::TypeName
//
// Purpose:
//   Returns the name of the RPC.
//
// Arguments:
//
// Returns:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// ****************************************************************************

const std::string
SetBackendTypeRPC::TypeName() const
{
    return "SetBackendTypeRPC";
}
