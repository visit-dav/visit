// ************************************************************************* //
//                        avtOriginatingDatasetSink.C                        //
// ************************************************************************* //

#include <avtOriginatingDatasetSink.h>


// ****************************************************************************
//  Method: avtOriginatingDatasetSink constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtOriginatingDatasetSink::avtOriginatingDatasetSink()
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingDatasetSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtOriginatingDatasetSink::~avtOriginatingDatasetSink()
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingDatasetSink::DynamicLoadBalanceCleanUp
//
//  Purpose:
//      A hook from the base class that allows us to clean up the data tree
//      after dynamic load balancing.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2001
//
// ****************************************************************************

void
avtOriginatingDatasetSink::DynamicLoadBalanceCleanUp(void)
{
    avtDataset_p dataset = GetTypedInput();
    dataset->Compact();
}


