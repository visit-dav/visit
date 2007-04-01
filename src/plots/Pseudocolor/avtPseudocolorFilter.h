// ************************************************************************* //
//                           avtPseudocolorFilter.h                          //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_FILTER_H
#define AVT_PSEUDOCOLOR_FILTER_H


#include <avtStreamer.h>
#include <PseudocolorAttributes.h>


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
//    Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004
//    Added PlotAtts.
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
    void                      SetPlotAtts(const PseudocolorAttributes *);

  protected:
    bool                      keepNodeZone;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);
  private:
    PseudocolorAttributes     plotAtts;
};


#endif


