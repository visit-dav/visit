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
};


#endif


