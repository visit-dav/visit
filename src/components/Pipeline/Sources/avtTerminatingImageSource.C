// ************************************************************************* //
//                         avtTerminatingImageSource.C                       //
// ************************************************************************* //

#include <avtTerminatingImageSource.h>


// ****************************************************************************
//  Method: avtTerminatingImageSource::FetchData
//
//  Purpose:
//      Defines FetchData, a method that is called when a terminating data
//      object does an Update.  This layer defines how the information obtained
//      (the image representation) should be associated with the data object.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
// ****************************************************************************

bool
avtTerminatingImageSource::FetchData(avtDataSpecification_p spec)
{
    return FetchImage(spec, GetOutputImage());
}


