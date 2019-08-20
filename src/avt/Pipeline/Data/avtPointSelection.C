// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtPointSelection.C                          //
// ************************************************************************* //

#include <avtPointSelection.h>


// ****************************************************************************
//  Method: avtPointSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
// ****************************************************************************

avtPointSelection::avtPointSelection()
{
    pt[0] = 0.;
    pt[1] = 0.;
    pt[2] = 0.;
}


// ****************************************************************************
//  Method: avtPointSelection destructor
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
// ****************************************************************************

avtPointSelection::~avtPointSelection()
{
}


// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
// ****************************************************************************

bool
avtPointSelection::operator==(const avtPointSelection &sel) const
{
    return (pt[0] == pt[0] && pt[1] == pt[1] && pt[2] == pt[2]);
}


// ****************************************************************************
//  Method: avtPointSelection::Destruct
//
//  Purpose:
//      Calls the destructor for an identifier selection.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
// ****************************************************************************

void
avtPointSelection::Destruct(void *i)
{
    avtPointSelection *pt = (avtPointSelection *) i;
    delete pt;
}


// ****************************************************************************
//  Method: avtPointSelection::DescriptionString
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
avtPointSelection::DescriptionString(void)
{
    char str[1024];
    snprintf(str, sizeof(str), "avtPointSelection:%f_%f_%f",
                                pt[0], pt[1], pt[2]);
    std::string s2 = str;
    return s2;
}


