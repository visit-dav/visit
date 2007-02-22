/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                           avtCompactTreeFilter.h                          //
// ************************************************************************* //

#ifndef AVT_COMPACT_TREE_FILTER_H
#define AVT_COMPACT_TREE_FILTER_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>


// ****************************************************************************
//  Class: avtCompactTreeFilter
//
//  Purpose:
//    Compacts the data tree to reduce the number of leaves.
//    If labels are present, resulting tree will have nlabels leaves.
//    If no labels are present, resulting tree will have 1 leaf.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 18, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Fri Oct 12 11:38:41 PDT 2001
//    Added executionDependsOnDLB flag, and related set methods.
//    This flag specifies whether or not execution should take place
//    with dynamic load balancing.
//
//    Kathleen Bonnell, Wed Apr 10 09:45:43 PDT 2002  
//    Added PostExecute method. 
//    
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002  
//    Removed PostExecute method. It's purpose is better served 
//    in avtSubsetFilter.
//    
//    Hank Childs, Wed Aug 24 15:45:14 PDT 2005
//    Add an option for cleaning poly-data.
//
//    Hank Childs, Thu Sep 22 16:59:42 PDT 2005
//    Add tolerance for merging points when cleaning poly data.  Also add
//    an option to do merging in parallel.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
// ****************************************************************************

class PIPELINE_API avtCompactTreeFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtCompactTreeFilter();
    virtual              ~avtCompactTreeFilter(){}; 

    virtual const char   *GetType(void) {return "avtCompactTreeFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Compacting data tree"; };

    void                  DLBDependentExecutionON(void)
                              { executionDependsOnDLB = true; };
    void                  DLBDependentExecutionOFF(void)
                              { executionDependsOnDLB = false; };

    void                  CreateCleanPolyData(bool v)
                              { createCleanPolyData = v; };
    void                  SetToleranceWhenCleaningPolyData(double d)
                              { tolerance = d; };
    void                  SetParallelMerge(bool p) { parallelMerge = p; };

  protected:
    virtual void          Execute(void);
    bool                  executionDependsOnDLB;
    bool                  parallelMerge;
    bool                  createCleanPolyData;
    double                tolerance;

    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif


