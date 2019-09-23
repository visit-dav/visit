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
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004
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
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004
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
//  Programmer: Kathleen Bonnell
//  Creation:   May 4, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtQueryOverTimeFilter::SetSILAtts(const SILRestrictionAttributes *silAtts)
{
    querySILAtts = *silAtts;
}


