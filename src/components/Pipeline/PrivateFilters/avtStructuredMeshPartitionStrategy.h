// ************************************************************************* //
//                   avtStructuredMeshPartitionStrategy.h                    //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_MESH_PARTITION_STRATEGY_H
#define AVT_STRUCTURED_MESH_PARTITION_STRATEGY_H

#include <pipeline_exports.h>

#include <vector>

#include <avtStructuredMeshChunker.h>


// ****************************************************************************
//  Class: avtStructuredMeshPartitionStrategy
//
//  Purpose:
//      An abstraction of a structured mesh partitioning strategy.  This is
//      used by the structured mesh chunker.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

class PIPELINE_API avtStructuredMeshPartitionStrategy
{
  public:
                            avtStructuredMeshPartitionStrategy();
    virtual                ~avtStructuredMeshPartitionStrategy();

    void                    SetMinimumSize(int);

    virtual void            ConstructPartition(const int *,
                                   avtStructuredMeshChunker::ZoneDesignation *,
                                   std::vector<int> &) = 0;
  protected:
    int                     minimumSize;
};


#endif


