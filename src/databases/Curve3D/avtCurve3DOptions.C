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
//      Creates the options for Curve3D readers. (currently none)
//
//  Programmer: Kathleen Biagas
//  Creation:   August 31, 2018
//
// ****************************************************************************

DBOptionsAttributes *
GetCurve3DReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}


// ****************************************************************************
//  Function: GetCurve3DWriteOptions
//
//  Purpose:
//      Creates the options for Curve3D writers.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 31, 2018
//
//  Modifications:
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
