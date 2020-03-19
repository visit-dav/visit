// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//
//    Tom Fogal, Tue Jun 23 20:40:51 MDT 2009
//    Added const version of GetFacadedFilter.
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

    virtual avtContract_p     ModifyContract(avtContract_p spec);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtFilter        *GetFacadedFilter(void);
    virtual const avtFilter  *GetFacadedFilter(void) const;
};


#endif
