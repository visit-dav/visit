#include <OpenDatabaseRPC.h>

// ****************************************************************************
// Method: OpenDatabaseRPC::OpenDatabaseRPC
//
// Purpose: 
//   Constructor for the OpenDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:05:36 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 24 11:14:57 PDT 2003
//   I made it inherit from NonBlockingRPC.
//
// ****************************************************************************

OpenDatabaseRPC::OpenDatabaseRPC() : NonBlockingRPC("si"), databaseName("")
{
    time = 0;
}

// ****************************************************************************
// Method: OpenDatabaseRPC::~OpenDatabaseRPC
//
// Purpose: 
//   Destructor for the OpenDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:05:54 PST 2002
//
// Modifications:
//   
// ****************************************************************************

OpenDatabaseRPC::~OpenDatabaseRPC()
{
}

// ****************************************************************************
// Method: OpenDatabaseRPC::operator()
//
// Purpose: 
//   Executes the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:05:54 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
OpenDatabaseRPC::operator()(const std::string &dbName, int timestep)
{
    databaseName = dbName;
    time = timestep;
    SelectAll();
    Execute();
}

// ****************************************************************************
// Method: OpenDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all of the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:06:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
OpenDatabaseRPC::SelectAll()
{
    Select(0, (void*)&databaseName);
    Select(1, (void*)&time);
}

