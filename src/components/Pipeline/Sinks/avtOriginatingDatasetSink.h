// ************************************************************************* //
//                          avtOriginatingDatasetSink.h                      //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_DATASET_SINK_H
#define AVT_ORIGINATING_DATASET_SINK_H
#include <pipeline_exports.h>


#include <avtDatasetSink.h>
#include <avtOriginatingSink.h>


// ****************************************************************************
//  Class: avtOriginatingDatasetSink
//
//  Purpose:
//      A dataset sink that originates pipeline execution.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added DynamicLoadBalanceCleanUp.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingDatasetSink
    : virtual public avtDatasetSink, virtual public avtOriginatingSink
{
  public:
                      avtOriginatingDatasetSink()  {;};
    virtual          ~avtOriginatingDatasetSink()  {;};

  protected:
    virtual void      DynamicLoadBalanceCleanUp(void);
};


#endif


