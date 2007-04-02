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
// ****************************************************************************

ExportDatabaseRPC::ExportDatabaseRPC() : BlockingRPC("ia")
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
// ****************************************************************************

void
ExportDatabaseRPC::operator()(const int id_, const ExportDBAttributes *atts)
{
    SetID(id_);
    SetExportDBAtts(atts);

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
    Select(0, (void*)&id);
    Select(1, (void*)&exportDBAtts);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::SetID
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
ExportDatabaseRPC::SetID(const int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

int
ExportDatabaseRPC::GetID() const
{
    return id;
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
