#include <NETCDFPluginInfo.h>
#include <avtNETCDFFileFormat.h>
#include <avtFileFormatInterface.h>
#include <avtGenericDatabase.h>
#include <VisItException.h>

// ****************************************************************************
//  Method:  NETCDFCommonPluginInfo::GetDatabaseType
//
//  Purpose:
//    Returns the type of a NETCDF database.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Aug 12 09:46:51 PDT 2005
//
// ****************************************************************************
DatabaseType
NETCDFCommonPluginInfo::GetDatabaseType()
{
    return DB_TYPE_MTSD;
}

// ****************************************************************************
//  Method:  NETCDFCommonPluginInfo::GetDefaultExtensions
//
//  Purpose:
//    Returns the default extensions for a NETCDF database.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Aug 12 09:46:51 PDT 2005
//
// ****************************************************************************
std::vector<std::string>
NETCDFCommonPluginInfo::GetDefaultExtensions()
{
    std::vector<std::string> defaultExtensions;
    defaultExtensions.push_back("nc");
    defaultExtensions.push_back("cdf");
    defaultExtensions.push_back("elev");
    defaultExtensions.push_back("ncd");

    return defaultExtensions;
}

// ****************************************************************************
//  Method: NETCDFCommonPluginInfo::SetupNETCDFDatabase
//
//  Purpose:
//      Sets up a NETCDF database.
//
//  Arguments:
//      list    A list of file names.
//      nList   The number of timesteps in list.
//      nBlocks The number of blocks in the list.
//
//  Returns:    A NETCDF database from list.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 12 09:46:51 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Fri Dec 9 17:39:34 PST 2005
//    I renamed a method call.
//
// ****************************************************************************
avtDatabase *
NETCDFCommonPluginInfo::SetupDatabase(const char *const *list,
                                      int nList, int nBlock)
{
    avtDatabase *db = 0;

    //
    // Create a file format interface.
    //
    avtFileFormatInterface *ffi = NETCDF_CreateFileFormatInterface(list, nList, nBlock);

    //
    // If we created a file format interface, try creating a database.
    //
    if(ffi)
    {
        // Try and create the database using the interface that was created.
        TRY
        {
            db = new avtGenericDatabase(ffi);
        }
        CATCH(VisItException)
        {
            delete ffi;
            delete db;
            RETHROW;
        }
        ENDTRY
    }

    return db;
}
