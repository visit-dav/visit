// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtTerminatingNullDataSink.h                       //
// ************************************************************************* //

#ifndef AVT_TERMINATING_NULL_DATA_SINK_H
#define AVT_TERMINATING_NULL_DATA_SINK_H
#include <pipeline_exports.h>


#include <avtNullDataSink.h>
#include <avtTerminatingSink.h>


// ****************************************************************************
//  Class: avtTerminatingNullDataSink
//
//  Purpose:
//      A null data sink that terminates pipeline execution.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003 
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingNullDataSink
    : virtual public avtNullDataSink, virtual public avtTerminatingSink
{
  public:
                      avtTerminatingNullDataSink();
    virtual          ~avtTerminatingNullDataSink();
};


#endif


