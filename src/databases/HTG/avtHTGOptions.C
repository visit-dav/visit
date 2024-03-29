// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtHTGOptions.C
// ****************************************************************************

#include <avtHTGOptions.h>

#include <DBOptionsAttributes.h>

#include <string>


// ****************************************************************************
//  Function: GetHTGReadOptions
//
//  Purpose:
//      Creates the options for HTG readers.
//
//  Important Note:
//      The code below sets up empty options.  If your format does not require
//      read options, no modifications are necessary.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Mon May 1 15:28:30 PST 2023
//
// ****************************************************************************

DBOptionsAttributes *
GetHTGReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}


// ****************************************************************************
//  Function: GetHTGWriteOptions
//
//  Purpose:
//      Creates the options for HTG writers.
//
//  Important Note:
//      The code below sets up empty options.  If your format does not require
//      write options, no modifications are necessary.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Mon May 1 15:28:30 PST 2023
//
// ****************************************************************************

DBOptionsAttributes *
GetHTGWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;

    rv->SetDouble(HTGDBOptions::HTG_BLANK_VALUE, -10000.);
    return rv;
}
