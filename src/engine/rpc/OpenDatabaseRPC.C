// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Hank Childs, Fri Mar  5 11:13:32 PST 2004
//   Added string for 'format'
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added createMeshQualityExpressions and 
//   createTimeDerivativeExpresisons. 
//
// ****************************************************************************

OpenDatabaseRPC::OpenDatabaseRPC() : NonBlockingRPC("ssibbb"), databaseName("")
{
    time = 0;
    createMeshQualityExpressions = true;
    createTimeDerivativeExpressions = true;
    ignoreExtents = false;
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
//   Hank Childs, Fri Mar  5 11:13:32 PST 2004
//   Added 'format'.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added createMeshQualityExpressions and 
//   createTimeDerivativeExpresisons. 
//
// ****************************************************************************

void
OpenDatabaseRPC::operator()(const std::string &f,
                            const std::string &dbName, int timestep,
                            bool cmqe, bool ctde, bool ie)
{
    fileFormat = f;
    databaseName = dbName;
    time = timestep;
    createMeshQualityExpressions = cmqe;
    createTimeDerivativeExpressions = ctde;
    ignoreExtents = ie;
    
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
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added createMeshQualityExpressions and 
//   createTimeDerivativeExpresisons. 
//   
// ****************************************************************************

void
OpenDatabaseRPC::SelectAll()
{
    Select(0, (void*)&fileFormat);
    Select(1, (void*)&databaseName);
    Select(2, (void*)&time);
    Select(3, (void*)&createMeshQualityExpressions);
    Select(4, (void*)&createTimeDerivativeExpressions);
    Select(5, (void*)&ignoreExtents);
}

