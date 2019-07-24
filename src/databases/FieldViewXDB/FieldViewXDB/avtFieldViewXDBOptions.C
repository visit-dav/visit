// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtFieldViewXDBOptions.C
// ************************************************************************* //

#include <avtFieldViewXDBOptions.h>
#include <DBOptionsAttributes.h>

#include <string>

// ****************************************************************************
//  Function: GetFieldViewXDBReadOptions
//
//  Purpose:
//      Creates the options for VTK readers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require read options, no modifications are 
//      necessary.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 28 16:08:26 PDT 2015
//
// ****************************************************************************

DBOptionsAttributes *
GetFieldViewXDBReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}


// ****************************************************************************
//  Function: GetFieldViewXDBWriteOptions
//
//  Purpose:
//      Creates the options for VTK writers.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 28 16:08:26 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

DBOptionsAttributes *
GetFieldViewXDBWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    rv->SetBool("Strip mesh name prefix", false);
    return rv;
}
