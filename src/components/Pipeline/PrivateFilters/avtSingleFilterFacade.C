// ************************************************************************* //
//                             avtSingleFilterFacade.C                       //
// ************************************************************************* //

#include <avtSingleFilterFacade.h>


// ****************************************************************************
//  Method: avtSingleFilterFacade::GetFirstFilter
//
//  Purpose:
//      This returns the first filter in the pipeline that we will be a facade
//      for.  Since this class provides the information that there is only one
//      filter, we can just return that filter.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

avtFilter *
avtSingleFilterFacade::GetFirstFilter(void)
{
    return GetFacadedFilter();
}


// ****************************************************************************
//  Method: avtSingleFilterFacade::GetLastFilter
//
//  Purpose:
//      This returns the last filter in the pipeline that we will be a facade
//      for.  Since this class provides the information that there is only one
//      filter, we can just return that filter.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

avtFilter *
avtSingleFilterFacade::GetLastFilter(void)
{
    return GetFacadedFilter();
}


// ****************************************************************************
//  Method: avtSingleFilterFacade
//
//  Purpose:
//      Releases problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtSingleFilterFacade::ReleaseData(void)
{
    avtFacadeFilter::ReleaseData();
    GetFacadedFilter()->ReleaseData();
}


