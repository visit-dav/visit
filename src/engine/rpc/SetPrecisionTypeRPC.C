// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetPrecisionTypeRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetPrecisionTypeRPC::SetPrecisionTypeRPC
//
// Purpose:
//   This is the constructor.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

SetPrecisionTypeRPC::SetPrecisionTypeRPC()
    : BlockingRPC("i",NULL)
{
}


// *******************************************************************
// Denstructor: SetPrecisionTypeRPC::~SetPrecisionTypeRPC
//
// Purpose:
//   This is the denstructor.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

SetPrecisionTypeRPC::~SetPrecisionTypeRPC()
{
}


// *******************************************************************
// Method: SetPrecisionTypeRPC::operator()
//
// Purpose:
//   This is the RPC's invocation method.
//
// Arguments:
//   pType : This is the new precision type.
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

void
SetPrecisionTypeRPC::operator()(const int pType)
{
    debug3 << "Executing SetPrecisionTypeRPC\n";

    SetPrecisionType(pType);
    Execute();
}


// *******************************************************************
// Method: SetPrecisionTypeRPC::SelectAll
//
// Purpose:
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

void
SetPrecisionTypeRPC::SelectAll()
{
    Select(0, (void*)&precisionType);
}


// *******************************************************************
// Method: SetPrecisionTypeRPC::SetPrecisionType
//
// Purpose:
//   Set the precision type argument of the RPC.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

void
SetPrecisionTypeRPC::SetPrecisionType(const int pType)
{
    precisionType = pType;
}

// *******************************************************************
// Method: SetPrecisionTypeRPC::GetPrecisionType
//
// Purpose:
//   Get the precision type for the RPC.
//
// Arguments:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// *******************************************************************

int
SetPrecisionTypeRPC::GetPrecisionType() const
{
    return precisionType;
}

// ****************************************************************************
// Method: SetPrecisionTypeRPC::TypeName
//
// Purpose:
//   Returns the name of the RPC.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// ****************************************************************************

const std::string
SetPrecisionTypeRPC::TypeName() const
{
    return "SetPrecisionTypeRPC";
}
