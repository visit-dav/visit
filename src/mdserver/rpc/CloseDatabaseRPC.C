#include <CloseDatabaseRPC.h>

// ****************************************************************************
// Method: CloseDatabaseRPC::CloseDatabaseRPC
//
// Purpose: 
//   This is the constructor for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:25:02 PDT 2005
//   Added database member.
//
// ****************************************************************************

CloseDatabaseRPC::CloseDatabaseRPC() : BlockingRPC(""), database()
{
}

// ****************************************************************************
// Method: CloseDatabaseRPC::~CloseDatabaseRPC
//
// Purpose: 
//   This is the destructor for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

CloseDatabaseRPC::~CloseDatabaseRPC()
{
}

// ****************************************************************************
// Method: CloseDatabaseRPC::operator()
//
// Purpose: 
//   This is the invokation method for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:27:36 PDT 2005
//   Added database argument,
//
// ****************************************************************************

void
CloseDatabaseRPC::operator()(const std::string &db)
{
    SetDatabase(db);
    Execute();
}

// ****************************************************************************
// Method: CloseDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all components of the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:58:42 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:28:17 PDT 2005
//   Added database.
//
// ****************************************************************************

void
CloseDatabaseRPC::SelectAll()
{
    Select(0, (void *)&database);
}

// ****************************************************************************
// Method: CloseDatabase::SetDatabase
//
// Purpose: 
//   Sets the value to use for database.
//
// Arguments:
//   db : The database to close.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 4 08:29:13 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
CloseDatabaseRPC::SetDatabase(const std::string &db)
{
    database = db;
    SelectAll();
}
