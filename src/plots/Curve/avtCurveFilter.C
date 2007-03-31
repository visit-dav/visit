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
//  Method: avtCurveFilter::PerformRestriction
//
//  Purpose:
//    Indicates that we cannot do dynamic load balancing with this filter.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 26, 2002 
//
// ****************************************************************************

avtPipelineSpecification_p
avtCurveFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    spec->NoDynamicLoadBalancing();
    return spec;
}

// ****************************************************************************
//  Method: avtCurveFilter::PerformRestriction
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
avtCurveFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetAttributes().SetSpatialDimension(2);
}

