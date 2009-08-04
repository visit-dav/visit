/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

    virtual avtDataTree_p      ExecuteDataTree(vtkDataSet *,int,std::string);
    virtual vtkDataSet        *ProcessOneChunk(vtkDataSet *,int,std::string,
                                               bool) = 0;
    virtual void               GetAssignments(vtkDataSet *, const int *,
                    std::vector<avtStructuredMeshChunker::ZoneDesignation>&)=0;

    virtual avtContract_p      ModifyContract(avtContract_p);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
};


#endif


