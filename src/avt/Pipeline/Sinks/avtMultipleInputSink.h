// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtMultipleInputSink.h                           //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_SINK_H
#define AVT_MULTIPLE_INPUT_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtDataTree.h>

class     avtDatasetSink;


// ****************************************************************************
//  Class: avtMultipleInputSink
//
//  Purpose:
//      The sink for a data object.  This can be either an originating sink
//      (one that originates a pipeline) or part of a filter.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
// ****************************************************************************

class PIPELINE_API avtMultipleInputSink : virtual public avtDataObjectSink
{
  public:
                                  avtMultipleInputSink();
    virtual                      ~avtMultipleInputSink();

    void                          SetNthInput(avtDataObject_p, int);

  protected:
    avtDataObjectSink           **sinks;
    int                           nSinks;

    virtual avtDataObject_p       GetInput(void);
    virtual const avtDataObject_p GetInput(void) const;
    virtual void                  SetTypedInput(avtDataObject_p);
    virtual bool                  UpdateInput(avtContract_p);

    avtDataObject_p               GetNthInput(int);
    void                          SetNumSinks(int);
    void                          SetSink(avtDataObjectSink *, int);

    avtDataTree_p                 GetTreeFromSink(avtDatasetSink *);
};


#endif


