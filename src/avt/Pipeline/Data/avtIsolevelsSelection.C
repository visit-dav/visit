// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtIsolevelsSelection.C                          //
// ************************************************************************* //

#include <limits.h>
#include <string.h>

#include <avtIsolevelsSelection.h>

// ****************************************************************************
//  Method: avtIsolevelsSelection constructor
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

avtIsolevelsSelection::avtIsolevelsSelection()
{
    var = "default";
}


// ****************************************************************************
//  Method: avtIsolevelsSelection constructor
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

avtIsolevelsSelection::avtIsolevelsSelection(const std::string _var, 
                                         const std::vector<double> &_isolevels)
{ 
    var       = _var; 
    isolevels = _isolevels; 
}


// ****************************************************************************
//  Method: avtIsolevelsSelection destructor
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

avtIsolevelsSelection::~avtIsolevelsSelection()
{ 
}


// ****************************************************************************
//  Method: avtIsolevelsSelection::DescriptionString
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
avtIsolevelsSelection::DescriptionString(void)
{
    char str[1024];
    snprintf(str, 1024, "avtIsolevelsSelection:%s", var.c_str());
    size_t amt = strlen(str);
    for (size_t i = 0 ; i < isolevels.size() ; i++)
    {
        snprintf(str+amt, 1024-amt, "_%f", isolevels[i]);
        amt += strlen(str);
    }
    std::string s2 = str;
    return s2;
}


