// ************************************************************************* //
//                       avtSweepPlanePartitionStrategy.h                    //
// ************************************************************************* //

#ifndef AVT_SWEEP_PLANE_PARTITION_STRATEGY_H
#define AVT_SWEEP_PLANE_PARTITION_STRATEGY_H

#include <pipeline_exports.h>

#include <vector>

#include <avtStructuredMeshPartitionStrategy.h>


// ****************************************************************************
//  Class: avtSweepPlanePartitionStrategy
//
//  Purpose:
//      An abstraction of a structured mesh partitioning strategy.  This is
//      used by the structured mesh chunker.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

class PIPELINE_API avtSweepPlanePartitionStrategy 
    : public avtStructuredMeshPartitionStrategy
{
  public:
                            avtSweepPlanePartitionStrategy();
    virtual                ~avtSweepPlanePartitionStrategy();

    virtual void            ConstructPartition(const int *,
                                   avtStructuredMeshChunker::ZoneDesignation *,
                                   std::vector<int> &);

  protected:
    typedef struct
    {
        int                 iStart;
        int                 iEnd;
        int                 jStart;
        int                 jEnd;
        int                 kStart;
        int                 kEnd;
        bool                stillCandidate;
        int                 gridSize;
    } GridCandidate;

    int                     dims[3];
    int                     plane_size;
    int                     line_size;
    int                     sweep_depth;
    int                     line_depth;
    int                    *dist;
    int                    *jDist;
    bool                   *haveDistance;
    bool                   *isAvailable;
    std::vector<GridCandidate>   proposedGrid;
    int                    *proposedGridIndex;

    void                    Initialize(const int *,
                                  avtStructuredMeshChunker::ZoneDesignation *);
    bool                    GreedyFindGrids(std::vector<int> &);
    bool                    CalculateRunLength(int);
    void                    FindBiggestGrids(int k);
    void                    FindBiggestGridWithDistance(int d, int k);
    void                    ProposeGrid(int, int, int, int, int, int);
    bool                    CommitProposedGrids(std::vector<int> &);

};


#endif


