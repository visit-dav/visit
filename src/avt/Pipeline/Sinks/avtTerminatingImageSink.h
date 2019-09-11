// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTerminatingImageSink.h                       //
// ************************************************************************* //

#ifndef AVT_TERMINATING_IMAGE_SINK_H
#define AVT_TERMINATING_IMAGE_SINK_H

#include <pipeline_exports.h>


#include <avtImageSink.h>
#include <avtTerminatingSink.h>


// ****************************************************************************
//  Class: avtTerminatingImageSink
//
//  Purpose:
//      A image sink that terminates pipeline execution.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingImageSink
    : virtual public avtImageSink, virtual public avtTerminatingSink
{
  public:
                      avtTerminatingImageSink();
    virtual          ~avtTerminatingImageSink();
};


#endif


