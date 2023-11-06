// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurve3DOptions.C                              //
// ************************************************************************* //

#include <avtCurve3DOptions.h>

#include <DBOptionsAttributes.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

// ****************************************************************************
//  Function: GetCurve3DReadOptions
//
//  Purpose:
//      Creates the options for Curve3D readers.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

DBOptionsAttributes *
GetCurve3DReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    rv->SetBool("Use blank line as another curve as in gnuplot", true);
    return rv;
}


// ****************************************************************************
//  Function: GetCurve3DWriteOptions
//
//  Purpose:
//      Creates the options for Curve3D writers.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

DBOptionsAttributes *
GetCurve3DWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    vector<string> commentStyle;
    commentStyle.push_back("# (Ultra)");
    commentStyle.push_back("% (MatLab)");
    rv->SetEnum("CommentStyle", 0);
    rv->SetEnumStrings("CommentStyle", commentStyle);

    return rv;
}
