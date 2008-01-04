/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

    static inline bool IsGhostZoneType(unsigned char &u, avtGhostZoneTypes t)
    {
        unsigned char bit = (1 << t);
        return (u & bit);
    }

    static inline bool IsGhostZone(unsigned char &u)
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

    static inline bool IsGhostNodeType(unsigned char &u, avtGhostNodeTypes t)
    {
        unsigned char bit = (1 << t);
        return (u & bit);
    }

    static inline bool IsGhostNode(unsigned char &u)
    {
        return (u != 0);
    }

    static inline bool UseZoneForInterpolation(unsigned char &u)
    {
        unsigned char bit = (1 << ZONE_NOT_APPLICABLE_TO_PROBLEM);
        return (u & bit);
    }

    static inline bool RemoveThisZoneBeforeFindingFacelist(unsigned char &u)
    {
        unsigned char bit1 = (1 << ZONE_EXTERIOR_TO_PROBLEM);
        unsigned char bit2 = (1 << ZONE_NOT_APPLICABLE_TO_PROBLEM);
        unsigned char bit = bit1 | bit2;
        return (u & bit);
    }

    static inline bool IsZoneDuplicated(unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit = bit1 | bit2 | bit3;
        return (u & bit);
    }

    static inline bool IsZoneDuplicatedByCombinationOfZones(unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit4 = (1 << REFINED_ZONE_IN_AMR_GRID);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool IsZoneOnExteriorOfDomain(unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit4 = (1 << ZONE_EXTERIOR_TO_PROBLEM);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool CanZoneBeUsedForStatistics(unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        unsigned char bit2 = (1 << ENHANCED_CONNECTIVITY_ZONE);
        unsigned char bit3 = (1 << REDUCED_CONNECTIVITY_ZONE);
        unsigned char bit4 = (1 << REFINED_ZONE_IN_AMR_GRID);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool DiscardFaceIfAllNodesAreOfThisType(unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_NODE);
        unsigned char bit2 = (1 << NODE_NOT_APPLICABLE_TO_PROBLEM);
        unsigned char bit3 = (1 << NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit4 = (1 << NODE_IS_ON_FINE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit = bit1 | bit2 | bit3 | bit4;
        return (u & bit);
    }

    static inline bool DiscardFaceIfOneNodeIsOfThisType(unsigned char &u)
    {
        unsigned char bit = (1 << NODE_NOT_APPLICABLE_TO_PROBLEM);
        return (u & bit);
    }

    static inline bool UseNodeForInterpolation(unsigned char &u)
    {
        unsigned char bit1 = (1 << DUPLICATED_NODE);
        unsigned char bit2 = (1 << NODE_IS_ON_COARSE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit3 = (1 << NODE_IS_ON_FINE_SIDE_OF_COARSE_FINE_BOUNDARY);
        unsigned char bit = bit1 | bit2 | bit3;
        return (u & bit);
    }
};


#endif


