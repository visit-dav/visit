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
// ****************************************************************************

avtR2Foperator::avtR2Foperator(int nb)
{
    nBins = nb;
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


