// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtQueryOverTimeFilter.C                          //
// ************************************************************************* //

#include <float.h>
#include <sstream>
#include <string>
#include <vector>

#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>
#include <avtQueryOverTimeFilter.h>


// ****************************************************************************
//  Method: avtQueryOverTimeFilter constructor
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019
//
//  Modifications:
//
// ****************************************************************************

avtQueryOverTimeFilter::avtQueryOverTimeFilter(const AttributeGroup *a)
{
    atts = *(QueryOverTimeAttributes*)a;
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter destructor
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 13:46:56 MST 2019 
//
// ****************************************************************************

avtQueryOverTimeFilter::~avtQueryOverTimeFilter()
{
}


// ****************************************************************************
//  Method: avtTimeLoopQOTFilter::SetSILAtts
//
//  Purpose:
//    Sets the SILRestriction atts necessary to create a SILRestriction.
//
//  Note: This was copied over from the previous version of 
//        avtQueryOverTimeFilter.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 13:46:56 MST 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtQueryOverTimeFilter::SetSILAtts(const SILRestrictionAttributes *silAtts)
{
    querySILAtts = *silAtts;
}
