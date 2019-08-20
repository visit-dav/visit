// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtDataRangeSelection.C                          //
// ************************************************************************* //

#include <limits.h>

#include <avtDataRangeSelection.h>

// ****************************************************************************
//  Method: avtDataRangeSelection constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 11, 2007 
//
// ****************************************************************************

avtDataRangeSelection::avtDataRangeSelection()
{
    var = "default";
    min = -FLT_MAX;
    max = +FLT_MAX; 
}


// ****************************************************************************
//  Method: avtDataRangeSelection constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 11, 2007 
//
// ****************************************************************************

avtDataRangeSelection::avtDataRangeSelection(const std::string _var, 
    const double _min, const double _max)
{ 
    var = _var; 
    min = _min; 
    max = _max; 
}


// ****************************************************************************
//  Method: avtDataRangeSelection destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 11, 2007 
//
// ****************************************************************************

avtDataRangeSelection::~avtDataRangeSelection()
{ 
}

// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 11, 2007 
//
// ****************************************************************************
bool
avtDataRangeSelection::operator==(const avtDataRangeSelection &s) const
{
    return ( (min == s.min) && (max == s.max) && (var == s.var) ); 
}


// ****************************************************************************
//  Method: avtDataRangeSelection::DescriptionString
//
//  Purpose:
//      Creates a string (used as a key for caching) that describes this
//      selection.
//
//  Programmmer: Hank Childs
//  Creation:    December 20, 2011
//
// ****************************************************************************

std::string
avtDataRangeSelection::DescriptionString(void)
{
    char str[1024];
    snprintf(str, sizeof(str), "avtDataRangeSelection:%s_%f_%f",
                                var.c_str(), min, max);
    std::string s2 = str;
    return s2;
}


