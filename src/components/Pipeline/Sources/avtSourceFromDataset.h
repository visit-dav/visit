// ************************************************************************* //
//                          avtSourceFromDataset.h                           //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_DATASET_H
#define AVT_SOURCE_FROM_DATASET_H
#include <pipeline_exports.h>


#include <avtTerminatingDatasetSource.h>


// ****************************************************************************
//  Class: avtSourceFromDataset
//
//  Purpose:
//      A source object (pipeline terminator) that is created from a vtk 
//      dataset.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 16:38:11 PDT 2001
//    Overhauled class and removed previous comments.
//
// ****************************************************************************

class PIPELINE_API avtSourceFromDataset : public avtTerminatingDatasetSource
{
  public:
                          avtSourceFromDataset(vtkDataSet **, int);
    virtual              ~avtSourceFromDataset();

    virtual bool          FetchDataset(avtDataSpecification_p,avtDataTree_p &);

  protected:
    vtkDataSet          **datasets;
    int                   nDataset;
};


#endif


