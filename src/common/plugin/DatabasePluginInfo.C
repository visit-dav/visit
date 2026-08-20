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
//  Return default REs for identifying scalar components of aggregate types.
//  Capture group 1 is the aggregate base name; group 2 is component id. Will
//  recognize candidate scalar component name strings with 2 or more characters
//  in the basename and of the forms...
//
//         v1 v2 v3                foo1 foo2 foo3
//         v_1 v_2 v_3             foo_1 foo_2 foo_3
//         v.1 v.2 v.3             foo.1 foo.2 foo.3
//
//         foox fooy fooz          fooX fooY fooZ
//         foo_x foo_y foo_z       foo_X foo_Y foo_Z
//         foo.x foo.y foo.z       foo.X foo.Y foo.Z
//
//         foou foov foow          fooU fooV fooW
//         foo_u foo_v foo_w       foo_U foo_V foo_W
//         foo.u foo.v foo.w       foo.U foo.V foo.W
//
//         e000, e001, e002 (e00 vector)
//
//  We could make the REs much simpler here and just have more of them for each
//  of the specific cases. But, there is a problem with that. Each RE is passed
//  over all the scalar variables defined in a database. And, this work happens
//  early on as part of a database **OPEN** operation. So, the number of REs
//  here multiplies times the number of scalar variables in a database to size
//  the total string matching work. For databases with a few hundered scalar
//  variables, that is no problem. But, equation of state databases can have
//  tens of thousands of scalar variables.
//
//  These are the **DEFAULT** REs over all database plugins in VisIt. Each
//  database can override these with the <ScalarComponentREs> tag in the .xml
//  file. We already do this with Pixie. We could also make the REs here part
//  of a database's preferences and user-settable in the GUI/CLI as opposed
//  to set at compile time (or xml2info time).
//  
//  ChatGPT+Mark C. Miller, Fri Aug 14 17:50:57 PDT 2026
// ****************************************************************************

std::vector<std::string>
GeneralDatabasePluginInfo::GetDefaultScalarComponentREs() const
{
    std::vector<std::string> result;

    //
    // With an explicit separator the component identifier may consist of
    // arbitrary characters. GroupAndOrderStringsByRE requires all identifiers
    // within a group to have the same length and orders them lexicographically.
    //    ^ : means anchor at beginning of string
    //    (.+) : means any sequence of 1 or more chars
    //        . : means any char
    //        + : means one or more of preceding
    //        (...) : means capture whatever matches as subexpr (capture group)
    //    [._] : means either . or _ (inside [] . means actual .)
    //    ([^._]+) : means any sequence of one or more chars not . or _
    //        [^._] : means any char not a . or _
    //        + : means one or more of preceding
    //        (...) : means capture whatever matches as subexpr (capture group)
    //    $ : means anchor at end of string
    //
    result.push_back("^(.+)[._]([^._]+)$");

    //
    // Without a separator, determining where the basename ends and the
    // component identifier begins is inherently ambiguous. Restrict these
    // cases to the conventional component-designator characters most commonly
    // used by data producers.
    //
    result.push_back("^(.*[^_.])([0123abcABCijkIJKuvwUVWxyzXYZ])$"); // vectors
    result.push_back("^(.*[^_.])([0123abcABCijkIJKuvwUVWxyzXYZ]{2})$"); // tensors

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
