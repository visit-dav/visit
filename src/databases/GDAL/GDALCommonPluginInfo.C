/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
