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
    
/* EXAMPLE OF OPTIONS

    rv->SetBool("Big Endian", false);
    rv->SetEnum("Dimension", 1);
    vector<string> dims;
    dims.push_back("0D");
    dims.push_back("1D");
    dims.push_back("2D");
    dims.push_back("3D");
    rv->SetEnumStrings("Dimension", dims);
    rv->SetInt("Number of variables", 5);
    rv->SetString("Name of auxiliary file", );
    rv->SetDouble("Displacement factor", 1.0);

    // When reading or writing the file, you can get the options out of this object like:
    rv->GetDouble("Displacement factor");
*/
}


// ****************************************************************************
//  Function: GetWavefrontOBJWriteOptions
//
//  Purpose:
//      Creates the options for WavefrontOBJ writers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require write options, no modifications are 
//      necessary.
//
//  Programmer: Justin Privitera
//  Creation:   11/03/23
//
//  Modifications:
//
// ****************************************************************************

DBOptionsAttributes *
GetWavefrontOBJWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    rv->SetBool("Output colors", false);
    rv->SetString("Color table", "hot");
    return rv;
}
