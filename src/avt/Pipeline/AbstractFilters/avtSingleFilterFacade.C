// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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


// ****************************************************************************
//  Method: avtSingleFilterFacade::GetIthFacadedFilter
//
//  Purpose:
//      Gets one of the filters.
//
//  Programmer: Tom Fogal
//  Creation:   June 23, 2009
//
// ****************************************************************************

const avtFilter *
avtSingleFilterFacade::GetIthFacadedFilter(int f) const
{
    if (f != 0)
        EXCEPTION0(ImproperUseException);

    return GetFacadedFilter();
}
