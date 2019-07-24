// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtCurveFilter.C                             //
// ************************************************************************* //

#include <avtCurveFilter.h>

#include <avtCurveConstructorFilter.h>

// ****************************************************************************
//  Method: avtCurveFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

avtCurveFilter::avtCurveFilter()
{
    ccf = new avtCurveConstructorFilter;
}


// ****************************************************************************
//  Method: avtCurveFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

avtCurveFilter::~avtCurveFilter()
{
    delete ccf;
}


// ****************************************************************************
//  Method: avtCurveFilter::GetFacadedFilter
//
//  Purpose:
//      Gets the facaded filter
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

avtFilter *
avtCurveFilter::GetFacadedFilter(void)
{
    return ccf;
}

// ****************************************************************************
//  Method: avtCurveFilter::GetFacadedFilter
//
//  Purpose:
//      Gets the facaded filter
//
//  Programmer: Tom Fogal
//  Creation:   June 23, 2009
//
// ****************************************************************************

const avtFilter *
avtCurveFilter::GetFacadedFilter(void) const
{
    return ccf;
}

// ****************************************************************************
//  Method: avtCurveFilter::ModifyContract
//
//  Purpose:
//    Indicates that we cannot do streaming with this filter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 26, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

avtContract_p
avtCurveFilter::ModifyContract(avtContract_p spec)
{
    spec->NoStreaming();
    return spec;
}

// ****************************************************************************
//  Method: avtCurveFilter::ModifyContract
//
//  Purpose:
//      Allows the filter to change its output's data object information, which
//      is a description of the data object.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 23, 2002
//
// ****************************************************************************

void
avtCurveFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetAttributes().SetSpatialDimension(2);
}

