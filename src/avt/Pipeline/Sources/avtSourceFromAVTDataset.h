// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSourceFromAVTDataset.h                         //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_AVT_DATASET_H
#define AVT_SOURCE_FROM_AVT_DATASET_H

#include <pipeline_exports.h>


#include <avtInlinePipelineSource.h>
#include <avtOriginatingDatasetSource.h>


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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Fri Dec 31 11:02:02 PST 2004
//    Allow the data tree to be reset.
//
// ****************************************************************************

class PIPELINE_API avtSourceFromAVTDataset
     : virtual public avtOriginatingDatasetSource,
       virtual public avtInlinePipelineSource
{
  public:
                          avtSourceFromAVTDataset(avtDataset_p ds);
    virtual              ~avtSourceFromAVTDataset();

    void                  ResetTree(avtDataTree_p t) { tree = t; };

  protected:
    avtDataTree_p         tree;

    virtual bool          FetchDataset(avtDataRequest_p,avtDataTree_p &);
};


#endif


