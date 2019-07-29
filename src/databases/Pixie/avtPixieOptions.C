// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPixieOptions.C                             //
// ************************************************************************* //

#include <avtPixieOptions.h>

#include <DBOptionsAttributes.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Function: GetPixieReadOptions
//
//  Purpose:
//      Creates the options for Pixie readers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require read options, no modifications are 
//      necessary.
//
//  Programmer: Jean Favre
//  Creation:   Thu Jun 21 16:20:41 PDT 2012
//
// ****************************************************************************

DBOptionsAttributes *
GetPixieReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    std::vector<std::string> partition;
    partition.push_back("X Slab");  // 0
    partition.push_back("Y Slab");  // 1
    partition.push_back("Z Slab");  // 2
    partition.push_back("KD Tree"); // 3
    rv->SetEnum(PixieDBOptions::RDOPT_PARTITIONING, PixieDBOptions::ZSLAB );
    rv->SetEnumStrings(PixieDBOptions::RDOPT_PARTITIONING, partition);
    rv->SetBool(PixieDBOptions::RDOPT_DUPLICATE, false);

    return rv;
}


// ****************************************************************************
//  Function: GetPixieWriteOptions
//
//  Purpose:
//      Creates the options for Pixie writers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require write options, no modifications are 
//      necessary.
//
//  Programmer: Jean Favre
//  Creation:   Thu Jun 21 16:20:41 PDT 2012
//
// ****************************************************************************

DBOptionsAttributes *
GetPixieWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}
