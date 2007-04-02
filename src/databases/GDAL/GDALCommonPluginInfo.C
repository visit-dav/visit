#include <GDALPluginInfo.h>
#include <avtGDALFileFormat.h>
#include <avtSTMDFileFormatInterface.h>
#include <avtGenericDatabase.h>
#include <stdlib.h>

// ****************************************************************************
//  Method:  GDALCommonPluginInfo::GetDatabaseType
//
//  Purpose:
//    Returns the type of a GDAL database.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Sep 6 11:53:39 PDT 2005
//
// ****************************************************************************
DatabaseType
GDALCommonPluginInfo::GetDatabaseType()
{
    return DB_TYPE_STMD;
}

// ****************************************************************************
//  Method:  GDALCommonPluginInfo::GetDefaultExtensions
//
//  Purpose:
//    Returns the default extensions for a GDAL database.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Sep 6 11:53:39 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Thu Sep 8 16:07:10 PST 2005
//    Add image extensions if the VISIT_READ_IMAGES_WITH_GDAL var is set.
//
// ****************************************************************************

std::vector<std::string>
GDALCommonPluginInfo::GetDefaultExtensions()
{
    std::vector<std::string> defaultExtensions;

    // If we want to read images using GDAL instead of the Images plugin,
    // add the extensions here.
    if(getenv("VISIT_READ_IMAGES_WITH_GDAL") != 0)
    {
        defaultExtensions.push_back("pnm");
        defaultExtensions.push_back("ppm");
        defaultExtensions.push_back("png");
        defaultExtensions.push_back("tiff");
        defaultExtensions.push_back("jpeg");
        defaultExtensions.push_back("jpg");
        defaultExtensions.push_back("tif");
        defaultExtensions.push_back("PNM");
        defaultExtensions.push_back("PPM");
        defaultExtensions.push_back("PNG");
        defaultExtensions.push_back("TIFF");
        defaultExtensions.push_back("JPEG");
        defaultExtensions.push_back("JPG");
        defaultExtensions.push_back("TIF");
        defaultExtensions.push_back("bmp");
        defaultExtensions.push_back("BMP");
    }

    // Add the other GDAL extensions.
    defaultExtensions.push_back("adf");
    defaultExtensions.push_back("asc");
    defaultExtensions.push_back("bt");
    defaultExtensions.push_back("ddf");
    defaultExtensions.push_back("dem");
    defaultExtensions.push_back("ecw");
    defaultExtensions.push_back("gxf");
    defaultExtensions.push_back("jp2");
    defaultExtensions.push_back("map");
    defaultExtensions.push_back("mem");
    defaultExtensions.push_back("mpl");
    defaultExtensions.push_back("mpr");
    defaultExtensions.push_back("n1");
    defaultExtensions.push_back("nat");
    defaultExtensions.push_back("ntf");
    defaultExtensions.push_back("pix");
    defaultExtensions.push_back("rsw");
    defaultExtensions.push_back("sid");
    defaultExtensions.push_back("vrt");
    defaultExtensions.push_back("xpm");

    return defaultExtensions;
}

// ****************************************************************************
//  Method: GDALCommonPluginInfo::SetupGDALDatabase
//
//  Purpose:
//      Sets up a GDAL database.
//
//  Arguments:
//      list    A list of file names.
//      nList   The number of timesteps in list.
//      nBlocks The number of blocks in the list.
//
//  Returns:    A GDAL database from list.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Sep 6 11:53:39 PDT 2005
//
// ****************************************************************************
avtDatabase *
GDALCommonPluginInfo::SetupDatabase(const char *const *list,
                                   int nList, int nBlock)
{
    avtSTMDFileFormat **ffl = new avtSTMDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        ffl[i] = new avtGDALFileFormat(list[i]);
    }
    avtSTMDFileFormatInterface *inter 
           = new avtSTMDFileFormatInterface(ffl, nList);
    return new avtGenericDatabase(inter);
}
