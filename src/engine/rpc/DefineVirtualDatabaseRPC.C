#include <DefineVirtualDatabaseRPC.h>

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::DefineVirtualDatabaseRPC
//
// Purpose: 
//   Constructor for the DefineVirtualDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:48:46 PST 2003
//
// Modifications:
//
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

DefineVirtualDatabaseRPC::DefineVirtualDatabaseRPC() : NonBlockingRPC("ssss*i"),
    fileFormat(), databaseName(), databasePath(), databaseFiles()
{
    time = 0;
}

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::~DefineVirtualDatabaseRPC
//
// Purpose: 
//   Destructor for the DefineVirtualDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:48:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DefineVirtualDatabaseRPC::~DefineVirtualDatabaseRPC()
{
}

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::operator()
//
// Purpose: 
//   Executes the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:47:52 PST 2003
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

void
DefineVirtualDatabaseRPC::operator()(const std::string &fileFormatType,
    const std::string &wholeDBName, const std::string &pathToTimesteps, 
    const stringVector &dbFiles, int timestep)
{
    fileFormat = fileFormatType;
    databaseName = wholeDBName;
    databasePath = pathToTimesteps;
    databaseFiles = dbFiles;
    time = timestep;
    SelectAll();
    Execute();
}

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all of the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:47:42 PST 2003
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

void
DefineVirtualDatabaseRPC::SelectAll()
{
    Select(0, (void*)&fileFormat);
    Select(1, (void*)&databaseName);
    Select(2, (void*)&databasePath);
    Select(3, (void*)&databaseFiles);
    Select(4, (void*)&time);
}

