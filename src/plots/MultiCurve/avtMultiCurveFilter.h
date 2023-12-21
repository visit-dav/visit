// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtMultiCurveFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MultiCurve_FILTER_H
#define AVT_MultiCurve_FILTER_H


#include <avtDatasetToDatasetFilter.h>
#include <MultiCurveAttributes.h>
#include <MapNode.h>


// ****************************************************************************
//  Class: avtMultiCurveFilter
//
//  Purpose:
//      This operator is the implied operator associated with a MultiCurve
//      plot.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Tue Mar  3 15:06:26 PST 2009
//    I added yAxisTickSpacing, so that I could pass it along in the
//    plot information, so that the plot could include it in the legend.
//
//    Eric Brugger, Fri Mar  6 08:19:58 PST 2009
//    I modified the filter could would also accept as input a collection
//    of poly data data sets representing the individual curves to display.
//
//    Kathleen Biagas, Tue Dec 19, 2023
//    Add outputInfo, for storing curves in PlotInformation.
//
// ****************************************************************************

class avtMultiCurveFilter : public avtDatasetToDatasetFilter
{
  public:
                              avtMultiCurveFilter(MultiCurveAttributes &);
    virtual                  ~avtMultiCurveFilter();

    virtual const char       *GetType(void)   { return "avtMultiCurveFilter"; }
    virtual const char       *GetDescription(void)
                                  { return "Performing MultiCurve"; }

    void                      SetAttributes(const MultiCurveAttributes &);

  protected:
    MultiCurveAttributes      atts;
    bool                      setYAxisTickSpacing;
    double                    yAxisTickSpacing;
    MapNode                   outputInfo;

    virtual void              Execute(void);

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
};


#endif
