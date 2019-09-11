// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIDXOptions.C                             //
// ************************************************************************* //

#include <avtIDXOptions.h>

#include <DBOptionsAttributes.h>

#include <string>


DBOptionsAttributes *
GetIDXReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    rv->SetBool("Use extra cells", true);
    rv->SetBool("Big Endian", false);
    return rv;
}

DBOptionsAttributes *
GetIDXWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}
