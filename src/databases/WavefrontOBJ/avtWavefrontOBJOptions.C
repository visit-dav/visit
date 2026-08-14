// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtWavefrontOBJOptions.C                              //
// ************************************************************************* //

#include <avtWavefrontOBJOptions.h>

#include <DBOptionsAttributes.h>

#include <string>


// ****************************************************************************
//  Function: GetWavefrontOBJReadOptions
//
//  Purpose:
//      Creates the options for WavefrontOBJ readers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require read options, no modifications are 
//      necessary.
//
//  Programmer: Justin Privitera
//  Creation:   11/03/23
//
// ****************************************************************************

DBOptionsAttributes *
GetWavefrontOBJReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}


// ****************************************************************************
//  Function: GetWavefrontOBJWriteOptions
//
//  Purpose:
//      Creates the options for WavefrontOBJ writers.
//
//  Programmer: Justin Privitera
//  Creation:   11/03/23
//
//  Modifications:
//    Justin Privitera, Tue Nov 28 17:31:40 PST 2023
//    Added "Invert color table" option.
//
//    Justin Privitera, Mon Jul 20 16:40:22 PDT 2026
//    Added new options for controlling limits and turned on Output colors by
//    default.
//
// ****************************************************************************

DBOptionsAttributes *
GetWavefrontOBJWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    rv->SetBool("Output colors", true);
    rv->SetString("Color table", "hot");
    rv->SetInt("Number of colors", 256);
    rv->SetBool("Invert color table", false);
    rv->SetBool("Use minimum", false);
    rv->SetDouble("Minimum", 0.);
    rv->SetBool("Use color for values < min", false);
    rv->SetColor("Color for values < min", 0, 0, 0, 255);
    rv->SetBool("Use maximum", false);
    rv->SetDouble("Maximum", 1.);
    rv->SetBool("Use color for values > max", false);
    rv->SetColor("Color for values > max", 0, 0, 0, 255);
    return rv;
}
