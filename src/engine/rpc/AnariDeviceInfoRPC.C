// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "AnariDeviceInfoRPC.h"
#include <string>

// ****************************************************************************
//  Method: AnariDeviceInfoRPC::AnariDeviceInfoRPC
//
//  Purpose:
//    This is the RPC's constructor
//
//  Programmer: Kevin Griffin
//  Creation:   Thu 27 Aug 2026
//
// ****************************************************************************

AnariDeviceInfoRPC::AnariDeviceInfoRPC() : BlockingRPC("sssa", &mapNodeString),
    libraryName(""), librarySubtype(""), rendererSubtype("")
{
    // nothing here;
}

// ****************************************************************************
//  Method: AnariDeviceInfoRPC::~AnariDeviceInfoRPC
//
//  Purpose:
//    This is the RPC's destructor
//
//  Programmer: Kevin Griffin
//  Creation:   Thu 27 Aug 2026
//
// ****************************************************************************

AnariDeviceInfoRPC::~AnariDeviceInfoRPC()
{
    // nothing here;
}

// ****************************************************************************
// Method: AnariDeviceInfoRPC::operator()
//
// Purpose:
//   Invocation method for the AnariDeviceInfoRPC class. Asks the engine to
//   report the ANARI libraries/subtypes/parameters it has available.
//
// Arguments:
//   libName     : ANARI library name (may be empty).
//   libSubtype  : ANARI device subtype for libName (may be empty).
//   rendSubtype : ANARI renderer subtype for libName/libSubtype (may be empty).
//
// Programmer: Kevin Griffin
// Creation:   Thu 27 Aug 2026
//
// ****************************************************************************

std::string
AnariDeviceInfoRPC::operator()(const std::string &libName,
                               const std::string &libSubtype,
                               const std::string &rendSubtype)
{
    libraryName = libName;
    librarySubtype = libSubtype;
    rendererSubtype = rendSubtype;
    Execute();
    return mapNodeString.s;
}

// ****************************************************************************
// Method: AnariDeviceInfoRPC::SelectAll
//
// Purpose:
//   Selects all components in the RPC.
//
// Programmer: Kevin Griffin
// Creation:   Thu 27 Aug 2026
//
// ****************************************************************************

void
AnariDeviceInfoRPC::SelectAll()
{
    Select(0, (void*)&libraryName);
    Select(1, (void*)&librarySubtype);
    Select(2, (void*)&rendererSubtype);
}

// ****************************************************************************
//  Constructor:  AnariDeviceInfoRPC::MapNodeString::MapNodeString
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************
AnariDeviceInfoRPC::MapNodeString::MapNodeString() : AttributeSubject("s")
{
    s = "";
}

// ****************************************************************************
//  Constructor:  AnariDeviceInfoRPC::MapNodeString::MapNodeString
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************
AnariDeviceInfoRPC::MapNodeString::MapNodeString(std::string s_) : AttributeSubject("s")
{
    s = s_;
}

// ****************************************************************************
//  Destructor:  AnariDeviceInfoRPC::MapNodeString::~MapNodeString
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************
AnariDeviceInfoRPC::MapNodeString::~MapNodeString()
{
}

// ****************************************************************************
//  Method:  AnariDeviceInfoRPC::MapNodeString::SelectAll
//
//  Purpose:
//    Select all attributes.
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************
void
AnariDeviceInfoRPC::MapNodeString::SelectAll()
{
    Select(0, (void*)&s);
}
