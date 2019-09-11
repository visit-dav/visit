// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtUintahOptions.C                             //
// ************************************************************************* //

#include <avtUintahOptions.h>

#include <DBOptionsAttributes.h>

#include <string>
#include <vector>

using namespace UintahDBOptions;

DBOptionsAttributes *
GetUintahReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    std::vector<std::string> extraOpts;
    extraOpts.push_back(UINTAH_LOAD_EXTRA_NONE);    // 0
    extraOpts.push_back(UINTAH_LOAD_EXTRA_CELLS);   // 1
    extraOpts.push_back(UINTAH_LOAD_EXTRA_PATCHES); // 2

    rv->SetEnum(       UINTAH_LOAD_EXTRA, 0);
    rv->SetEnumStrings(UINTAH_LOAD_EXTRA, extraOpts);

    rv->SetBool(UINTAH_DATA_VARIES_OVER_TIME, true);
    return rv;
}

DBOptionsAttributes *
GetUintahWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}
