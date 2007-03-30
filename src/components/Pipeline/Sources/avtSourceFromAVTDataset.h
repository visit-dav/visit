// ************************************************************************* //
//                         avtSourceFromAVTDataset.h                         //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_AVT_DATASET_H
#define AVT_SOURCE_FROM_AVT_DATASET_H
#include <pipeline_exports.h>


#include <avtInlinePipelineSource.h>
#include <avtTerminatingDatasetSource.h>


// ****************************************************************************
//  Class: avtSourceFromAVTDataset
//
//  Purpose:
//      A source object (pipeline terminator) that is created from an AVT
//      dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 19, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 11 14:58:14 PDT 2001
//    Re-worked inheritance hierarchy.
//
// ****************************************************************************

class PIPELINE_API avtSourceFromAVTDataset : virtual public avtTerminatingDatasetSource,
                                virtual public avtInlinePipelineSource
{
  public:
                          avtSourceFromAVTDataset(avtDataset_p ds);
    virtual              ~avtSourceFromAVTDataset() {;};

  protected:
    avtDataTree_p         tree;

    virtual bool          FetchDataset(avtDataSpecification_p,avtDataTree_p &);
};


#endif


