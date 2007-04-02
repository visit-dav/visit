// ************************************************************************* //
//                            avtResampleFilter.h                            //
// ************************************************************************* //

#ifndef AVT_RESAMPLE_FILTER_H
#define AVT_RESAMPLE_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

#include <ResampleAttributes.h>


// ****************************************************************************
//  Class: avtResampleFilter
//
//  Purpose:
//      Resamples a dataset onto a rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Apr  6 17:39:40 PDT 2001
//    Added ability to bypass filter with already valid rectilinear grids.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added selID to support data selections
// ****************************************************************************

class AVTFILTERS_API avtResampleFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtResampleFilter(const AttributeGroup*);
    virtual              ~avtResampleFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtResampleFilter"; };
    virtual const char   *GetDescription(void) { return "Resampling"; };

  protected:
    ResampleAttributes    atts;
    char                 *primaryVariable;
    int                   selID;

    virtual void          Execute(void);
    virtual void          RefashionDataObjectInfo(void);

    void                  GetDimensions(int &, int &, int &, const double *);
    bool                  InputNeedsNoResampling(void);
    void                  ResampleInput(void);
    void                  BypassResample(void);

    virtual int           AdditionalPipelineFilters(void) { return 2; };

    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
};


#endif


