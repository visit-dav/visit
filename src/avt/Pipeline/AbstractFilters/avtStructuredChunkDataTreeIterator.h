// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtStructuredChunkDataTreeIterator.h                    //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_CHUNK_DATA_TREE_ITERATOR_H
#define AVT_STRUCTURED_CHUNK_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtSIMODataTreeIterator.h>
#include <avtGhostData.h>
#include <avtStructuredMeshChunker.h>


// ****************************************************************************
//  Class: avtStructuredChunkDataTreeIterator
//
//  Purpose:
//      This is an abstract type that prepares a filter to interact with
//      the structured mesh chunker module.  It is possible to use the
//      structured mesh chunker without the help of this class -- it's sole
//      purpose is to do bookkeeping to ease the burden for derived types,
//      as well as remove redundant code.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 16:31:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class PIPELINE_API avtStructuredChunkDataTreeIterator : public avtSIMODataTreeIterator
{
  public:
                               avtStructuredChunkDataTreeIterator();
    virtual                   ~avtStructuredChunkDataTreeIterator();

  protected:
    bool                       downstreamRectilinearMeshOptimizations;
    bool                       downstreamCurvilinearMeshOptimizations;
    avtGhostDataType           downstreamGhostType;
    bool                       chunkedStructuredMeshes;

    virtual avtDataTree_p      ExecuteDataTree(avtDataRepresentation *);
    virtual avtDataRepresentation *ProcessOneChunk(avtDataRepresentation *,
                                               bool) = 0;
    virtual void               GetAssignments(vtkDataSet *, const int *,
                    std::vector<avtStructuredMeshChunker::ZoneDesignation>&)=0;

    virtual avtContract_p      ModifyContract(avtContract_p);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
};


#endif


