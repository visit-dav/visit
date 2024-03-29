// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <WavefrontOBJPluginInfo.h>
#include <avtWavefrontOBJFileFormat.h>
#include <avtSTSDFileFormatInterface.h>
#include <avtGenericDatabase.h>
#include <avtWavefrontOBJOptions.h>

// ****************************************************************************
//  Method:  WavefrontOBJCommonPluginInfo::GetDatabaseType
//
//  Purpose:
//    Returns the type of a WavefrontOBJ database.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
DatabaseType
WavefrontOBJCommonPluginInfo::GetDatabaseType()
{
    return DB_TYPE_STSD;
}

// ****************************************************************************
//  Method: WavefrontOBJCommonPluginInfo::SetupDatabase
//
//  Purpose:
//      Sets up a WavefrontOBJ database.
//
//  Arguments:
//      list    A list of file names.
//      nList   The number of timesteps in list.
//      nBlocks The number of blocks in the list.
//
//  Returns:    A WavefrontOBJ database from list.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************
avtDatabase *
WavefrontOBJCommonPluginInfo::SetupDatabase(const char *const *list,
                                   int nList, int nBlock)
{
    int nTimestep = nList / nBlock;
    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nTimestep];
    for (int i = 0; i < nTimestep; i++)
    {
        ffl[i] = new avtSTSDFileFormat*[nBlock];
        for (int j = 0; j < nBlock; j++)
        {
            ffl[i][j] = new avtWavefrontOBJFileFormat(list[i*nBlock + j]);
        }
    }
    avtSTSDFileFormatInterface *inter
           = new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
    return new avtGenericDatabase(inter);
}

// ****************************************************************************
//  Method: WavefrontOBJCommonPluginInfo::GetWriteOptions
//
//  Purpose:
//      Gets the write options.
//
//  Programmer: Justin Privitera
//  Creation:   11/03/23
//
// ****************************************************************************

DBOptionsAttributes *
WavefrontOBJCommonPluginInfo::GetWriteOptions() const
{
    return GetWavefrontOBJWriteOptions();
}
