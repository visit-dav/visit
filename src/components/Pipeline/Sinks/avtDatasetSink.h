// ************************************************************************* //
//                               avtDatasetSink.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_SINK_H
#define AVT_DATASET_SINK_H
#include <pipeline_exports.h>


#include <avtDataObjectSink.h>
#include <avtDataset.h>


// ****************************************************************************
//  Class: avtDatasetSink
//
//  Purpose:
//      This is a data object sink whose input is a dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Sun May 26 18:55:01 PDT 2002
//    Make GetInput a public method like it is in the base type.
//
// ****************************************************************************

class PIPELINE_API avtDatasetSink : virtual public avtDataObjectSink
{
  public:
                              avtDatasetSink();
    virtual                  ~avtDatasetSink() {;};

    virtual avtDataObject_p   GetInput(void);

  protected:
    avtDataTree_p             GetInputDataTree();
    avtDataset_p              GetTypedInput(void) { return input; };

    virtual void              SetTypedInput(avtDataObject_p);

  private:
    avtDataset_p              input;
};


#endif


