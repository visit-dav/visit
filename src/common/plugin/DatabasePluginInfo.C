// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           DatabasePluginInfo.C                            //
// ************************************************************************* //

#include <DatabasePluginInfo.h>

#include <DBOptionsAttributes.h>
#include <DatabasePluginManager.h>

// ****************************************************************************
//  Method: CommonDatabasePluginInfo constructor
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

CommonDatabasePluginInfo::CommonDatabasePluginInfo()
{
    readOptions = NULL;
    writeOptions = NULL;
    pluginManager = 0;
}


// ****************************************************************************
//  Method: CommonDatabasePluginInfo destructor
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

CommonDatabasePluginInfo::~CommonDatabasePluginInfo()
{
    if (readOptions != NULL)
        delete readOptions;
    if (writeOptions != NULL)
        delete writeOptions;
}


// ****************************************************************************
//  Method: CommonDatabasePluginInfo::GetReadOptions
//
//  Purpose:
//      Gets the read options.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

DBOptionsAttributes *
CommonDatabasePluginInfo::GetReadOptions(void) const
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}

// ****************************************************************************
//  Method: CommonDatabasePluginInfo::GetWriteOptions
//
//  Purpose:
//      Gets the write options.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

DBOptionsAttributes *
CommonDatabasePluginInfo::GetWriteOptions(void) const
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}


// ****************************************************************************
//  Method: CommonDatabasePluginInfo::SetReadOptions
//
//  Purpose:
//      Sets the read options.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

void
CommonDatabasePluginInfo::SetReadOptions(DBOptionsAttributes *ro)
{
    readOptions = ro;
}


// ****************************************************************************
//  Method: CommonDatabasePluginInfo::SetWriteOptions
//
//  Purpose:
//      Sets the write options.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

void
CommonDatabasePluginInfo::SetWriteOptions(DBOptionsAttributes *wo)
{
    writeOptions = wo;
}

// ****************************************************************************
// Method: CommonDatabasePluginInfo::SetPluginManager
//
// Purpose: 
//   Set the database plugin manager that is associated with this info.
//
// Arguments:
//   ptr : The database plugin manager that read the info.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 16:29:03 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
CommonDatabasePluginInfo::SetPluginManager(DatabasePluginManager *ptr)
{
    pluginManager = ptr;
}

// ****************************************************************************
// Method: CommonDatabasePluginInfo::GetPluginManager
//
// Purpose: 
//   Get the plugin manager that read this info.
//
// Arguments:
//
// Returns:    The plugin manager that read this info.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 16:29:38 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

DatabasePluginManager *
CommonDatabasePluginInfo::GetPluginManager() const
{
    return pluginManager;
}
