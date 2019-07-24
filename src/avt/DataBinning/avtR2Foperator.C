// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtR2Foperator.C                             //
// ************************************************************************* //

#include <avtR2Foperator.h>


// ****************************************************************************
//  Method: avtR2Foperator constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:24:49 PST 2006
//    Added undefined value.
//
// ****************************************************************************

avtR2Foperator::avtR2Foperator(int nb, double uv)
{
    nBins = nb;
    undefinedVal = uv;
}


// ****************************************************************************
//  Method: avtR2Foperator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtR2Foperator::~avtR2Foperator()
{
    ;
}


