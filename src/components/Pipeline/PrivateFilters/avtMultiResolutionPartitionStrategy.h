/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                     avtMultiResolutionPartitionStrategy.h                 //
// ************************************************************************* //

#ifndef AVT_MULTIRESOLUTION_PARTITION_STRATEGY_H
#define AVT_MULTIRESOLUTION_PARTITION_STRATEGY_H

#include <pipeline_exports.h>

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

class PIPELINE_API avtMultiResolutionPartitionStrategy 
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


