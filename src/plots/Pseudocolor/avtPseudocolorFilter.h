// ************************************************************************* //
//                           avtPseudocolorFilter.h                          //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_FILTER_H
#define AVT_PSEUDOCOLOR_FILTER_H


#include <avtStreamer.h>



// ****************************************************************************
//  Class: avtPseudocolorFilter
//
//  Purpose:  To set specific flags in the pipeline that may be necessary
//            when picking on point meshes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

class avtPseudocolorFilter : public avtStreamer
{
  public:
                              avtPseudocolorFilter();
    virtual                  ~avtPseudocolorFilter();

    virtual const char       *GetType(void)   { return "avtPseudocolorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Preparing pipeline"; };

  protected:
    bool                      keepNodeZone;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);
};


#endif


