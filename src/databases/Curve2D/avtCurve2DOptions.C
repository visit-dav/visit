// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurve2DOptions.C                              //
// ************************************************************************* //

#include <avtCurve2DOptions.h>

#include <DBOptionsAttributes.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

// ****************************************************************************
//  Function: GetCurve2DReadOptions
//
//  Purpose:
//      Creates the options for Curve2D readers. (currently none)
//
//  Programmer: Kathleen Biagas
//  Creation:   August 31, 2018
//
// ****************************************************************************

DBOptionsAttributes *
GetCurve2DReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}


// ****************************************************************************
//  Function: GetCurve2DWriteOptions
//
//  Purpose:
//      Creates the options for Curve2D writers.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 31, 2018
//
//  Modifications:
//
// ****************************************************************************

DBOptionsAttributes *
GetCurve2DWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    vector<string> commentStyle;
    commentStyle.push_back("# (Ultra)");
    commentStyle.push_back("% (MatLab)");
    rv->SetEnum("CommentStyle", 0);
    rv->SetEnumStrings("CommentStyle", commentStyle);

    return rv;
}
