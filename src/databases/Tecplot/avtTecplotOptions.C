// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtTecplotOptions.C                              //
// ************************************************************************* //

#include <avtTecplotOptions.h>

#include <DBOptionsAttributes.h>

#include <string>


// ****************************************************************************
//  Function: GetTecplotReadOptions
//
//  Purpose:
//      Creates the options for Tecplot readers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require read options, no modifications are 
//      necessary.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 25, 2011
//
// ****************************************************************************

DBOptionsAttributes *
GetTecplotReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;

    std::vector<std::string> types;
    types.push_back("Guess from variable names");
    types.push_back("Specify explicitly (below)");
    rv->SetEnum("Method to determine coordinate axes", 0);
    rv->SetEnumStrings("Method to determine coordinate axes", types);

    rv->SetInt("X axis variable index (or -1 for none)", -1);
    rv->SetInt("Y axis variable index (or -1 for none)", -1);
    rv->SetInt("Z axis variable index (or -1 for none)", -1);

    return rv;
}


// ****************************************************************************
//  Function: GetTecplotWriteOptions
//
//  Purpose:
//      Creates the options for Tecplot writers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require write options, no modifications are 
//      necessary.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 25, 2011
//
// ****************************************************************************

DBOptionsAttributes *
GetTecplotWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    rv->SetInt("Gzip compression level [1,9] (0 for none)", 0);
    return rv;
}
