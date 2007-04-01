// ************************************************************************* //
//                      avtStructuredChunkStreamer.h                         //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_CHUNK_STREAMER_H
#define AVT_STRUCTURED_CHUNK_STREAMER_H

#include <pipeline_exports.h>

#include <avtDataTreeStreamer.h>
#include <avtGhostData.h>
#include <avtStructuredMeshChunker.h>


// ****************************************************************************
//  Class: avtStructuredChunkStreamer
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
// ****************************************************************************

class PIPELINE_API avtStructuredChunkStreamer : public avtDataTreeStreamer
{
  public:
                               avtStructuredChunkStreamer();
    virtual                   ~avtStructuredChunkStreamer();

  protected:
    bool                       downstreamRectilinearMeshOptimizations;
    bool                       downstreamCurvilinearMeshOptimizations;
    avtGhostDataType           downstreamGhostType;
    bool                       chunkedStructuredMeshes;

    virtual avtDataTree_p      ExecuteDataTree(vtkDataSet *,int,std::string);
    virtual vtkDataSet        *ProcessOneChunk(vtkDataSet *,int,std::string,
                                               bool) = 0;
    virtual void               GetAssignments(vtkDataSet *, const int *,
                    std::vector<avtStructuredMeshChunker::ZoneDesignation>&)=0;

    virtual avtPipelineSpecification_p
                               PerformRestriction(avtPipelineSpecification_p);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
};


#endif


