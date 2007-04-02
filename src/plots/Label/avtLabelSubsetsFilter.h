// ************************************************************************* //
//                         avtLabelSubsetsFilter.h                           //
// ************************************************************************* //

#ifndef AVT_LABEL_SUBSETS_FILTER_H
#define AVT_LABEL_SUBSETS_FILTER_H

#include <avtDataTreeStreamer.h>

// ****************************************************************************
//  Class: avtLabelSubsetsFilter
//
//  Purpose:  Ensures that the correct subset names are passed along
//            as labels.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Brad Whitlock, Wed Aug 3 17:58:50 PST 2005
//    Copied from the Subset plot and made its only job to split up materials
//    and pass along other subset variables.
//
// ****************************************************************************

class avtLabelSubsetsFilter : public avtDataTreeStreamer
{
  public:
                          avtLabelSubsetsFilter();
    virtual              ~avtLabelSubsetsFilter(){}; 

    virtual const char   *GetType(void) {return "avtLabelSubsetsFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Setting subset names"; };

    void                  SetNeedMIR(bool val) { needMIR = val; };
  protected:
    virtual avtDataTree_p ExecuteDataTree(vtkDataSet *, int, string);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);

    virtual void          PostExecute(void);

    bool needMIR;
};


#endif
