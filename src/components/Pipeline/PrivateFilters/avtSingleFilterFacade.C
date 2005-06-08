// ************************************************************************* //
//                             avtSingleFilterFacade.C                       //
// ************************************************************************* //

#include <avtSingleFilterFacade.h>

#include <ImproperUseException.h>

//
// Must be in here or won't build on Windows.
//
avtSingleFilterFacade::avtSingleFilterFacade() {;};
avtSingleFilterFacade::~avtSingleFilterFacade() {;};


// ****************************************************************************
//  Method: avtSingleFilterFacade::GetIthFacadedFilter
//
//  Purpose:
//      Gets one of the filters.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2005
//
// ****************************************************************************

avtFilter *
avtSingleFilterFacade::GetIthFacadedFilter(int f)
{
    if (f != 0)
        EXCEPTION0(ImproperUseException);

    return GetFacadedFilter();
}


