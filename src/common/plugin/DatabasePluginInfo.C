// ************************************************************************* //
//                           DatabasePluginInfo.C                            //
// ************************************************************************* //

#include <DatabasePluginInfo.h>

#include <DBOptionsAttributes.h>


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


