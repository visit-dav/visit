// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDynamicAttribute.C                           //
// ************************************************************************* //

#include <avtDynamicAttribute.h>


// ****************************************************************************
//  Method: avtDynamicAttribute constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDynamicAttribute::avtDynamicAttribute()
{
    ;
}


// ****************************************************************************
//  Method: avtDynamicAttribute destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDynamicAttribute::~avtDynamicAttribute()
{
    ;
}


// ****************************************************************************
//  Method: avtDynamicAttribute::ModifyContract
//
//  Purpose:
//      This gives derived types an opportunity to affect what is going to be
//      coming down in the data object.  They can add additional variable to
//      be read in, for example.
//      
//  Arguments:
//      spec    The current pipeline specification.
//
//  Returns:    The appropriate pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

avtContract_p
avtDynamicAttribute::ModifyContract(avtContract_p spec)
{
    return spec;
}


