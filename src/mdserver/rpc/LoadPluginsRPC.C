#include <LoadPluginsRPC.h>

// ****************************************************************************
// Method: LoadPluginsRPC::LoadPluginsRPC
//
// Purpose: 
//   Constructor for the LoadPluginsRPC class.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// ****************************************************************************

LoadPluginsRPC::LoadPluginsRPC() : NonBlockingRPC("")
{
}

// ****************************************************************************
// Method: LoadPluginsRPC::~LoadPluginsRPC
//
// Purpose: 
//   Destructor for the LoadPluginsRPC class.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// Modifications:
//   
// ****************************************************************************

LoadPluginsRPC::~LoadPluginsRPC()
{
}

// ****************************************************************************
// Method: LoadPluginsRPC::operator()
//
// Purpose: 
//   This is the invokation method for this function object. It copies the
//   arguments into local attributes and calls RPC's Execute method.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// ****************************************************************************

void
LoadPluginsRPC::operator()(void)
{
    Execute();
}

// ****************************************************************************
// Method: LoadPluginsRPC::SelectAll
//
// Purpose: 
//   Gets the addresses of the local attributes so the attributes can be
//   transmitted over the network as AttributeGroups.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// ****************************************************************************

void
LoadPluginsRPC::SelectAll()
{
}

