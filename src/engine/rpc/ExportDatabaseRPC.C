// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ExportDatabaseRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: ExportDatabaseRPC::ExportDatabaseRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
//  Modifications:
//    Kathleen Biagas, Fri Apr 23 2021
//    Added return Atts.
//
// ****************************************************************************

ExportDatabaseRPC::ExportDatabaseRPC() : BlockingRPC("i*as", &returnAtts),
    ids(), exportDBAtts(), timeSuffix()
{
}

// ****************************************************************************
//  Destructor: ExportDatabaseRPC::~ExportDatabaseRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

ExportDatabaseRPC::~ExportDatabaseRPC()
{
}

// ****************************************************************************
//  Method: ExportDatabaseRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//    origData  the type of input 
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Jan 24 16:40:22 PST 2014
//    Allow more than one network.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Thu Jul 24 22:16:34 EDT 2014
//    Pass time suffix.
//
// ****************************************************************************

void
ExportDatabaseRPC::operator()(const intVector &ids_, 
                              const ExportDBAttributes *atts, 
                              const std::string &s)
{
    SetIDs(ids_);
    SetExportDBAtts(atts);
    SetTimeSuffix(s);

    Execute();
}

// ****************************************************************************
//  Method: ExportDatabaseRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

void
ExportDatabaseRPC::SelectAll()
{
    Select(0, (void*)&ids);
    Select(1, (void*)&exportDBAtts);
    Select(2, (void*)&timeSuffix);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::SetIDs
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

void
ExportDatabaseRPC::SetIDs(const intVector &ids_)
{
    ids = ids_;
    Select(0, (void*)&ids);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::GetID
//
//  Purpose: 
//    This returns network ids.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

const intVector &
ExportDatabaseRPC::GetIDs() const
{
    return ids;
}

// ****************************************************************************
//  Method: ExportDatabaseRPC::SetExportDBAtts 
//
//  Purpose:
//    This sets the exportDB atts.
//
//  Arguments:
//    atts      the exportDB atts
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005 
//
// ****************************************************************************
 
void
ExportDatabaseRPC::SetExportDBAtts(const ExportDBAttributes *atts)
{
    exportDBAtts = *atts;
    Select(1, (void*)&exportDBAtts);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::GetExportDBAtts 
//
//  Purpose:
//    This returns the exportDB atts.
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005 
//
// ****************************************************************************
 
ExportDBAttributes *
ExportDatabaseRPC::GetExportDBAtts() 
{
    return &exportDBAtts;
}

void
ExportDatabaseRPC::SetTimeSuffix(const std::string &s)
{
    timeSuffix = s;
    Select(2, (void*)&timeSuffix);
}

const std::string &
ExportDatabaseRPC::GetTimeSuffix() const
{
    return timeSuffix;
}
