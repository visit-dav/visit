// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtCompactTreeFilter.h                          //
// ************************************************************************* //

#ifndef AVT_COMPACT_TREE_FILTER_H
#define AVT_COMPACT_TREE_FILTER_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>

class avtParallelContext;

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
//    Dave Pugmire, Tue Aug 24 11:32:12 EDT 2010
//    Add compact domain options.
//
//    Brad Whitlock, Wed Mar  5 12:46:41 PST 2014
//    I exposed a static Execute method that can compact the data object and
//    return a data tree.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Thu Aug  6 14:49:34 PDT 2015
//    Use avtParallelContext.
//
// ****************************************************************************

class PIPELINE_API avtCompactTreeFilter : public avtDatasetToDatasetFilter
{
  public:
    enum CompactDomainsMode
        {
            Never  = 0,
            Always = 1,
            Auto   = 2
        };

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

    void                  SetCompactDomainsMode(CompactDomainsMode mode, int threshold=-1)
                              { compactDomainMode = mode; compactDomainThreshold = threshold; }

    static avtDataTree_p  Execute(avtParallelContext &context,
                                  avtDataObject_p    input, 
                                  bool               executionDependsOnDLB,
                                  bool               parallelMerge,
                                  bool               skipCompact,
                                  bool               createCleanPolyData,
                                  double             tolerance,
                                  CompactDomainsMode compactDomainMode,
                                  int                compactDomainThreshold);
  protected:
    virtual void          Execute(void);
    bool                  executionDependsOnDLB;
    bool                  parallelMerge;
    bool                  createCleanPolyData;
    double                tolerance;
    CompactDomainsMode    compactDomainMode;
    int                   compactDomainThreshold;

    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif


