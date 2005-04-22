// ************************************************************************* //
//                     avtMultiResolutionPartitionStrategy.h                 //
// ************************************************************************* //

#ifndef AVT_MULTIRESOLUTION_PARTITION_STRATEGY_H
#define AVT_MULTIRESOLUTION_PARTITION_STRATEGY_H

#include <filters_exports.h>

#include <vector>

#include <avtStructuredMeshPartitionStrategy.h>


// ****************************************************************************
//  Class: avtMultiResolutionPartitionStrategy
//
//  Purpose:
//      An abstraction of a structured mesh partitioning strategy.  This is
//      used by the structured mesh chunker.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2004
//
// ****************************************************************************

class AVTFILTERS_API avtMultiResolutionPartitionStrategy 
    : public avtStructuredMeshPartitionStrategy
{
  public:
                            avtMultiResolutionPartitionStrategy();
    virtual                ~avtMultiResolutionPartitionStrategy();

    virtual void            ConstructPartition(const int *,
                                   avtStructuredMeshChunker::ZoneDesignation *,
                                   std::vector<int> &);

  protected:
    typedef struct
    {
        bool      allOn;
        bool      someOn;
    }   RegionDescription;

    RegionDescription        **reg_desc;
    int                        nLevels;
    int                        orig_dims[3];
    int                        full_dim_size;
    std::vector<int>           tmpBox;
    std::vector<int>           dims_per_level;
    std::vector<int>           cells_represented;

    void                       Level0Initialize(const int *,
                                  avtStructuredMeshChunker::ZoneDesignation *);
    void                       FinalInitialize(void);
    void                       FreeUpMemory(void);
    void                       FindBoxes(int, int);
    void                       SearchFor2x2Box(bool *, bool *, int , int *);
    void                       SearchFor2x1Line(bool *, bool *, int , int *);
    void                       Make2x2Box(int, int *);
    void                       Make2x1Line(int, int, int);
    void                       SearchFromCorners(void);
    void                       GetExtents(int, int, int *);
    void                       GetSubCells(int, int, int *);
    void                       AddBox(int *);
    bool                       GrowBox(int *, int axis, int direction);
};


#endif


