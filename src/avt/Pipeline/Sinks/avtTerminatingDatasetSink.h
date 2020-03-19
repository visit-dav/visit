// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtTerminatingDatasetSink.h                      //
// ************************************************************************* //

#ifndef AVT_TERMINATING_DATASET_SINK_H
#define AVT_TERMINATING_DATASET_SINK_H

#include <pipeline_exports.h>

#include <avtDatasetSink.h>
#include <avtTerminatingSink.h>


// ****************************************************************************
//  Class: avtTerminatingDatasetSink
//
//  Purpose:
//      A dataset sink that terminates pipeline execution.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added DynamicLoadBalanceCleanUp.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingDatasetSink
    : virtual public avtDatasetSink, virtual public avtTerminatingSink
{
  public:
                      avtTerminatingDatasetSink();
    virtual          ~avtTerminatingDatasetSink();

  protected:
    virtual void      StreamingCleanUp(void);
};


#endif


