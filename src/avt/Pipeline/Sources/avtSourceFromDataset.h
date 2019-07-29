// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSourceFromDataset.h                           //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_DATASET_H
#define AVT_SOURCE_FROM_DATASET_H
#include <pipeline_exports.h>


#include <avtOriginatingDatasetSource.h>


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

class PIPELINE_API avtSourceFromDataset : public avtOriginatingDatasetSource
{
  public:
                          avtSourceFromDataset(vtkDataSet **, int);
    virtual              ~avtSourceFromDataset();

    virtual bool          FetchDataset(avtDataRequest_p,avtDataTree_p &);

  protected:
    vtkDataSet          **datasets;
    int                   nDataset;
};


#endif


