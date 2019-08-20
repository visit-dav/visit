// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtVTKDatasetToDatasetFilter.h                       //
// ************************************************************************* //

#ifndef AVT_VTK_DATASET_TO_DATASET_FILTER_H
#define AVT_VTK_DATASET_TO_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDatasetFilter.h>
#include <avtSourceFromDataset.h>


// ****************************************************************************
//  Class: avtVTKDatasetToDatasetFilter
//
//  Purpose:
//      A filter that converts a group of VTK datasets into an avtDataset.  
//      Since avtDatasets are really collections of vtkDataSets, this is
//      really only magic with the pipeline updates.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.
//
//    Kathleen Biagas, Mon Jan 28 11:24:36 PST 2013
//    Remove no longer used method 'UpdateInput'.
//
// ****************************************************************************

class PIPELINE_API avtVTKDatasetToDatasetFilter : public avtDatasetToDatasetFilter
{
  public:
                               avtVTKDatasetToDatasetFilter();
    virtual                   ~avtVTKDatasetToDatasetFilter();

    virtual const char        *GetType(void)
                                    { return "avtVTKDatasetToDatasetFilter"; };

    virtual void               SetInput(vtkDataSet **, int);

  protected:
    avtSourceFromDataset      *source;

    virtual void               CalcDomainList(void);
    virtual void               Execute(avtDomainList *);
};


#endif


