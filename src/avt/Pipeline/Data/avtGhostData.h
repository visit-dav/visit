// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtGhostData.h                                //
// ************************************************************************* //

#ifndef AVT_GHOST_DATA_H
#define AVT_GHOST_DATA_H
#include <pipeline_exports.h>

typedef enum
{
    DUPLICATED_ZONE_INTERNAL_TO_PROBLEM = 0,
    ENHANCED_CONNECTIVITY_ZONE = 1,
    REDUCED_CONNECTIVITY_ZONE = 2,
    REFINED_ZONE_IN_AMR_GRID = 3,
    ZONE_EXTERIOR_TO_PROBLEM = 4,
    ZONE_NOT_APPLICABLE_TO_PROBLEM = 5
} avtGhostZoneTypes;

typedef enum
{
    DUPLICATED_NODE = 0,
    NODE_NOT_APPLICABLE_TO_PROBLEM = 1,
    NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY = 2,
    NODE_IS_ON_FINE_SIDE_OF_COARSE_FINE_BOUNDARY = 3
} avtGhostNodeTypes;

typedef enum
{
    NO_GHOST_DATA = 0,
    GHOST_NODE_DATA = 1,
    GHOST_ZONE_DATA = 2
} avtGhostDataType;


// ****************************************************************************
//  Class: avtGhostData
//
//  Purpose:
//      This class contains inlined methods that help centralize the logic
//      for handling ghost data.
//
//  Programmer: Hank Childs
//  Creation:   August 10, 2004
//
//  Modifications:
//
//     Hank Childs, Fri Aug  3 13:22:40 PDT 2007
//     Added ghost node types for coarse/fine boundaries.
//
//     Gunther H. Weber, Mon Aug 30 21:27:37 PDT 2010
//     Added const qualifiers where possible
//
// ****************************************************************************

class avtGhostData
{
  public:
    static inline void AddGhostZoneType(unsigned char &u, avtGhostZoneTypes t)
    {
        unsigned char bit = (1 << t);
        if (!(u & bit))
            u += bit;
    }

    static inline void RemoveGhostZoneType(unsigned char &u, 
                                           avtGhostZoneTypes t)
    {
        unsigned char bit = (1 << t);
        if (u & bit)
            u -= bit;
    }

    static inline bool IsGhostZoneType(const unsigned char &u, avtGhostZoneTypes t)
    {
        unsigned char bit = (1 << t);
        return (u & bit);
    }

    static inline bool IsGhostZone(const unsigned char &u)
    {
        return (u != 0);
    }

    static inline void AddGhostNodeType(unsigned char &u, avtGhostNodeTypes t)
    {
        unsigned char bit = (1 << t);
        if (!(u & bit))
            u += bit;
    }

    static inline void RemoveGhostNodeType(unsigned char &u, 
                                           avtGhostNodeTypes t)
    {
        unsigned char bit = (1 << t);
        if (u & bit)
            u -= bit;
    }

    static inline bool IsGhostNodeType(const unsigned char &u, avtGhostNodeTypes t)
    {
        unsigned char bit = (1 << t);
        return (u & bit);
    }

    static inline bool IsGhostNode(const unsigned char &u)
    {
        return (u != 0);
    }

    static inline bool UseZoneForInterpolation(const unsigned char &u)
    {
        unsigned char bit = (1 << ZONE_NOT_APPLICABLE_TO_PROBLEM);
        return (u & bit);
    }

    static inline bool RemoveThisZoneBeforeFindingFacelist(const unsigned char &u)
    {
        unsigned char bit1 = (1 << ZONE_EXTERIOR_TO_PROBLEM);
        unsigned char bit2 = (1 << ZONE_NOT_APPLICABLE_TO_PROBLEM);
        unsigned char bit = bit1 | bit2;
        return (u & bit);
    }

    static inline bool IsZoneDuplicated(const unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit = bit1 | bit2 | bit3;
        return (u & bit);
    }

    static inline bool IsZoneDuplicatedByCombinationOfZones(const unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit4 = (1 << REFINED_ZONE_IN_AMR_GRID);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool IsZoneOnExteriorOfDomain(const unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit4 = (1 << ZONE_EXTERIOR_TO_PROBLEM);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool CanZoneBeUsedForStatistics(const unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit4 = (1 << REFINED_ZONE_IN_AMR_GRID);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool DiscardFaceIfAllNodesAreOfThisType(const unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_NODE);
        unsigned char bit2 = (1 << NODE_NOT_APPLICABLE_TO_PROBLEM);
        unsigned char bit3 = (1 << NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit4 = (1 << NODE_IS_ON_FINE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool DiscardFaceIfOneNodeIsOfThisType(const unsigned char &u)
    {
        unsigned char bit = (1 << NODE_NOT_APPLICABLE_TO_PROBLEM);
        return (u & bit);
    }

    static inline bool UseNodeForInterpolation(const unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_NODE);
        unsigned char bit2 = (1 << NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit3 = (1 << NODE_IS_ON_FINE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit = bit1 | bit2 | bit3;
        return (u & bit);
    }
};


#endif


