// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtOriginatingImageSource.C                       //
// ************************************************************************* //

#include <avtOriginatingImageSource.h>


// ****************************************************************************
//  Method: avtOriginatingImageSource constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtOriginatingImageSource::avtOriginatingImageSource()
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingImageSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtOriginatingImageSource::~avtOriginatingImageSource()
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingImageSource::FetchData
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
avtOriginatingImageSource::FetchData(avtDataRequest_p spec)
{
    return FetchImage(spec, GetOutputImage());
}


