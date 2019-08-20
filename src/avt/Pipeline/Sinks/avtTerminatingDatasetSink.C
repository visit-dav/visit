// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtTerminatingDatasetSink.C                        //
// ************************************************************************* //

#include <avtTerminatingDatasetSink.h>


// ****************************************************************************
//  Method: avtTerminatingDatasetSink constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTerminatingDatasetSink::avtTerminatingDatasetSink()
{
    ;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTerminatingDatasetSink::~avtTerminatingDatasetSink()
{
    ;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSink::StreamingCleanUp
//
//  Purpose:
//      A hook from the base class that allows us to clean up the data tree
//      after streaming.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

void
avtTerminatingDatasetSink::StreamingCleanUp(void)
{
    avtDataset_p dataset = GetTypedInput();
    dataset->Compact();
}


