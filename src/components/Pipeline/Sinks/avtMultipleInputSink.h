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
    virtual void                  SetTypedInput(avtDataObject_p);
    virtual bool                  UpdateInput(avtPipelineSpecification_p);

    avtDataObject_p               GetNthInput(int);
    void                          SetNumSinks(int);
    void                          SetSink(avtDataObjectSink *, int);

    avtDataTree_p                 GetTreeFromSink(avtDatasetSink *);
};


#endif


