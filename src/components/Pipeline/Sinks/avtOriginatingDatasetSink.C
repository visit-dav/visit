// ************************************************************************* //
//                        avtOriginatingDatasetSink.C                        //
// ************************************************************************* //

#include <avtOriginatingDatasetSink.h>


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


