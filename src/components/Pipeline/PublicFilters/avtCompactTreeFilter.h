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
  protected:
    virtual void          Execute(void);
    bool                  executionDependsOnDLB;
};


#endif


