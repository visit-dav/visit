// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Kathleen Bonnell, Wed Oct 10 08:18:49 PDT 2007 
//   Added createMeshQualityExpressions and createTimeDerivativeExpressions.
//
// ****************************************************************************

DefineVirtualDatabaseRPC::DefineVirtualDatabaseRPC() : NonBlockingRPC("ssss*ibb"),
    fileFormat(), databaseName(), databasePath(), databaseFiles()
{
    time = 0;
    createMeshQualityExpressions = true;
    createTimeDerivativeExpressions = true;
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
//   Kathleen Bonnell, Wed Oct 10 08:18:49 PDT 2007 
//   Added createMeshQualityExpressions and createTimeDerivativeExpressions.
//
// ****************************************************************************

void
DefineVirtualDatabaseRPC::operator()(const std::string &fileFormatType,
    const std::string &wholeDBName, const std::string &pathToTimesteps, 
    const stringVector &dbFiles, int timestep, bool cmqe, bool ctde)
{
    fileFormat = fileFormatType;
    databaseName = wholeDBName;
    databasePath = pathToTimesteps;
    databaseFiles = dbFiles;
    time = timestep;
    createMeshQualityExpressions = cmqe;
    createTimeDerivativeExpressions = ctde;
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
//   Kathleen Bonnell, Wed Oct 10 08:18:49 PDT 2007 
//   Added createMeshQualityExpressions and createTimeDerivativeExpressions.
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
    Select(5, (void*)&createMeshQualityExpressions);
    Select(6, (void*)&createTimeDerivativeExpressions);
}

