// ************************************************************************* //
//                              avtCurveFilter.h                             //
// ************************************************************************* //

#ifndef AVT_CURVE_FILTER_H
#define AVT_CURVE_FILTER_H


#include <avtSingleFilterFacade.h>

class     avtCurveConstructorFilter;

// ****************************************************************************
//  Class: avtCurveFilter
//
//  Purpose:
//      This operator is the implied operator associated with an Curve plot.
//
//  NOTES:      There was a previous incarnation of the curve filter written
//              by Kathleen Bonnell.  It is now called the
//              avtCurveConstructorFilter and lives in PublicFilters.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

class avtCurveFilter : public avtSingleFilterFacade
{
  public:
                              avtCurveFilter();
    virtual                  ~avtCurveFilter();

    virtual const char       *GetType(void)   { return "avtCurveFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Performing Curve"; };

  protected:
    avtCurveConstructorFilter    *ccf;

    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p spec);
    virtual void              RefashionDataObjectInfo(void);
    virtual avtFilter        *GetFacadedFilter(void);
};


#endif


