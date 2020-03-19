// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtBinningScheme.C                            //
// ************************************************************************* //

#include <avtBinningScheme.h>


// ****************************************************************************
//  Method: avtBinningScheme constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the 
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Initialize oobb (out-of-bounds behavior).
//
// ****************************************************************************

avtBinningScheme::avtBinningScheme()
{
    oobb = ConstructDataBinningAttributes::Clamp;
}

// ****************************************************************************
//  Method: avtBinningScheme destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtBinningScheme::~avtBinningScheme()
{
    ;
}


