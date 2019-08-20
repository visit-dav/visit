// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtDataSetReader.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_READER_H
#define AVT_DATASET_READER_H
#include <pipeline_exports.h>

#include <array_ref_ptr.h>
#include <ref_ptr.h>

#include <avtOriginatingDatasetSource.h>


// ****************************************************************************
//  Class: avtDataSetReader
//
//  Purpose:
//      A class which takes as input an avtDataSet and can serialize it.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 09:07:37 PDT 2001
//    Blew away old comments.  Re-wrote for new inheritance structure.
//
//    Hank Childs, Mon Sep 17 10:58:02 PDT 2001
//    Made read also accept a character string reference to prevent unneeded
//    copying.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataSetReader : public avtOriginatingDatasetSource
{
  public:
                           avtDataSetReader();
    virtual               ~avtDataSetReader();

    virtual bool           ArtificialPipeline(void) { return true; };
    int                    Read(char *input, CharStrRef &);
    avtDataTree_p          ReadDataTree(char *&, int &, CharStrRef &);

    virtual bool           UseLoadBalancer(void) { return false; };

  protected:
    avtDataTree_p          dataTree;
    bool                   haveFetchedThisData;

    virtual bool           FetchDataset(avtDataRequest_p,avtDataTree_p&);
};


typedef ref_ptr<avtDataSetReader>  avtDataSetReader_p;

#endif


