// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtOriginatingDatasetSource.h                      //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_DATASET_SOURCE_H
#define AVT_ORIGINATING_DATASET_SOURCE_H

#include <pipeline_exports.h>

#include <avtDatasetSource.h>
#include <avtDatasetVerifier.h>
#include <avtOriginatingSource.h>


// ****************************************************************************
//  Class: avtOriginatingDatasetSource
//
//  Purpose:
//      This defines what an originating dataset source looks like.  Note that
//      this uses the dreaded "diamond shaped inheritance" since it looks like
//      an originating source and it also looks like a dataset source.
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

class PIPELINE_API avtOriginatingDatasetSource
    : virtual public avtDatasetSource, virtual public avtOriginatingSource
{
  public:
                              avtOriginatingDatasetSource();
    virtual                  ~avtOriginatingDatasetSource();

    void                      MergeExtents(vtkDataSet *, int dom, int ts, const char *);

  protected:
    avtDatasetVerifier        verifier;

    virtual bool              FetchData(avtDataRequest_p);
    virtual bool              FetchDataset(avtDataRequest_p,
                                           avtDataTree_p &) = 0;
};


#endif


