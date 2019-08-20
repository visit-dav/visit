// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtIdentifierSelection.C                        //
// ************************************************************************* //

#include <avtIdentifierSelection.h>


// ****************************************************************************
//  Method: avtIdentifierSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
//  Modifications:
//    Brad Whitlock, Thu Mar 15 14:17:11 PDT 2012
//    Initialize ids, idVar.
//
// ****************************************************************************

avtIdentifierSelection::avtIdentifierSelection() : avtDataSelection(), 
    ids(), idVar()
{
}


// ****************************************************************************
//  Method: avtIdentifierSelection destructor
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
// ****************************************************************************

avtIdentifierSelection::~avtIdentifierSelection()
{
}


// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
// ****************************************************************************

bool
avtIdentifierSelection::operator==(const avtIdentifierSelection &sel) const
{
    return (ids == sel.ids) && (idVar == sel.idVar);
}


// ****************************************************************************
//  Method: avtIdentifierSelection::Destruct
//
//  Purpose:
//      Calls the destructor for an identifier selection.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
// ****************************************************************************

void
avtIdentifierSelection::Destruct(void *i)
{
    avtIdentifierSelection *ids = (avtIdentifierSelection *) i;
    delete ids;
}


// ****************************************************************************
//  Method: avtIdentifierRangeSelection::DescriptionString
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
avtIdentifierSelection::DescriptionString(void)
{
    // This one is going to take some thought about how to build the
    // string in a way that doesn't get too long.  Apologies to the next
    // developer, but it isn't the problem I'm trying to solve right now.
    // The short cut is just to have file formats that do these selections
    // to just declare they can't do caching.
    EXCEPTION1(VisItException, "Exception due to unimplemented code: avtIdentifiedRangeSelection");
}


