// ************************************************************************* //
//                        avtTerminatingDatasetSource.h                      //
// ************************************************************************* //

#ifndef AVT_TERMINATING_DATASET_SOURCE_H
#define AVT_TERMINATING_DATASET_SOURCE_H
#include <pipeline_exports.h>


#include <avtDatasetSource.h>
#include <avtDatasetVerifier.h>
#include <avtTerminatingSource.h>


// ****************************************************************************
//  Class: avtTerminatingDatasetSource
//
//  Purpose:
//      This defines what a terminating dataset source looks like.  Note that
//      this uses the dreaded "diamond shaped inheritance" since it looks like
//      a terminating source and it also looks like a dataset source.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingDatasetSource
    : virtual public avtDatasetSource, virtual public avtTerminatingSource
{
  public:
                              avtTerminatingDatasetSource();
    virtual                  ~avtTerminatingDatasetSource();

    void                      MergeExtents(vtkDataSet *);

  protected:
    avtDatasetVerifier        verifier;

    virtual bool              FetchData(avtDataSpecification_p);
    virtual bool              FetchDataset(avtDataSpecification_p,
                                           avtDataTree_p &) = 0;
};


#endif


