// ************************************************************************* //
//                               avtDatasetSource.h                          //
// ************************************************************************* //

#ifndef AVT_DATASET_SOURCE_H
#define AVT_DATASET_SOURCE_H
#include <pipeline_exports.h>


#include <avtDataObjectSource.h>
#include <avtDataset.h>


// ****************************************************************************
//  Class: avtDatasetSource
//
//  Purpose:
//      A data object source who data object is a dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDatasetSource : virtual public avtDataObjectSource
{
  public:
                             avtDatasetSource();
    virtual                 ~avtDatasetSource();

    virtual avtDataObject_p  GetOutput(void);
    vtkDataSet              *GetVTKOutput(void);

    avtDataset_p             GetTypedOutput(void)  { return dataset; };

  protected:
    avtDataset_p             dataset;

    avtDataTree_p           &GetDataTree(void)
                                { return dataset->GetDataTree(); };
    void                     SetOutputDataTree(const avtDataTree_p);
};


#endif


