// ************************************************************************* //
//                           avtDataObjectSink.h                             //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_SINK_H
#define AVT_DATA_OBJECT_SINK_H
#include <pipeline_exports.h>


#include <avtDataObject.h>

#include <avtPipelineSpecification.h>


// ****************************************************************************
//  Class: avtDataObjectSink
//
//  Purpose:
//      The sink for a data object.  This can be either an originating sink
//      (one that originates a pipeline) or part of a filter.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Sep 12 10:08:46 PDT 2001
//    Added UpdateInput.
//
//    Hank Childs, Mon Oct  1 14:13:09 PDT 2001
//    Made GetInput public.  Needed for comparisons.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectSink
{
    friend class                  avtMultipleInputSink;

  public:
                                  avtDataObjectSink() {;};
    virtual                      ~avtDataObjectSink() {;};

    void                          SetInput(avtDataObject_p);
    virtual avtDataObject_p       GetInput(void) = 0;

  protected:
    virtual void                  SetTypedInput(avtDataObject_p) = 0;
    virtual void                  ChangedInput(void);
    virtual bool                  UpdateInput(avtPipelineSpecification_p);
};


#endif


