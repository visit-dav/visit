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
//  Method: GeneralDatabasePluginInfo::GetDefaultScalarComponentREs
//
//  Purpose:
//      Return default REs for identifying scalar components of aggregates.
//      Capture group 1 is the aggregate base name; group 2 is component id.
//      Will recognize candidate scalar component name strings with 2 or more
//      characters in the basename and of the forms...
//
//         foo1 foo2 foo3
//         foo_1 foo_2 foo_3
//         foo.1 foo.2 foo.3
//
//         foox fooy fooz          fooX fooY fooZ
//         foo_x foo_y foo_z       foo_X foo_Y foo_Z
//         foo.x foo.y foo.z       foo.X foo.Y foo.Z
//
//         foou foov foow          fooU fooV fooW
//         foo_u foo_v foo_w       foo_U foo_V foo_W
//         foo.u foo.v foo.w       foo.U foo.V foo.W
//
//  ChatGPT via Mark C. Miller, Fri Aug 14 17:50:57 PDT 2026
// ****************************************************************************
std::vector<std::string>
GeneralDatabasePluginInfo::GetDefaultScalarComponentREs() const
{
    std::vector<std::string> result;

    // Vector candidates
    result.push_back("^(.{2,})_([123xyzXYZ])$");
    result.push_back("^(.{2,})\\.([123xyzXYZ])$");
    result.push_back("^(.{1,}[^_.])([123xyzXYZ])$");

    // Tensor candidates
    result.push_back("^(.{2,})_([123xyzXYZuvwUVW]{2})$");
    result.push_back("^(.{2,})\\.([123xyzXYZuvwUVW]{2})$");
    result.push_back("^(.{1,}[^_.])([123xyzXYZuvwUVW]{2})$");

    return result;
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
