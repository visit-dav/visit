#include <ClearCacheRPC.h>

// ****************************************************************************
// Method: ClearCacheRPC::ClearCacheRPC
//
// Purpose: 
//   Constructor for the ClearCacheRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:58:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

ClearCacheRPC::ClearCacheRPC() : BlockingRPC("sb"), dbName("")
{
    clearAll = true;
}

// ****************************************************************************
// Method: ClearCacheRPC::~ClearCacheRPC
//
// Purpose: 
//   Destructor for the ClearCacheRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:58:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

ClearCacheRPC::~ClearCacheRPC()
{
}

// ****************************************************************************
// Method: ClearCacheRPC::operator()
//
// Purpose: 
//   Invokation method for the ClearCacheRPC class. Clears the caches that 
//   relate to the specified database.
//
// Arguments:
//   filename : The name of the database for which we're clearing information.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:58:56 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ClearCacheRPC::operator()(const std::string &filename, bool clearAllCaches)
{
    dbName = filename;
    clearAll = clearAllCaches;
    Execute();
}

// ****************************************************************************
// Method: ClearCacheRPC::SelectAll
//
// Purpose: 
//   Selects all components in the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:59:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ClearCacheRPC::SelectAll()
{
    Select(0, (void*)&dbName);
    Select(1, (void*)&clearAll);
}
