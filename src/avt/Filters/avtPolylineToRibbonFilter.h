// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPolylineToRibbonFilter.h                     //
// ************************************************************************* //

#ifndef AVT_POLYLINE_TO_RIBBON_FILTER_H
#define AVT_POLYLINE_TO_RIBBON_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

// ****************************************************************************
//  Class: avtPolylineToRibbonFilter
//
//  Purpose:
//    Creates a ribbon from a polyline
//
//  Programmer: Allen Sanderson
//  Creation:   Feb 12 2016
//
// ****************************************************************************

class AVTFILTERS_API avtPolylineToRibbonFilter : virtual public avtDataTreeIterator
{
  public:
                             avtPolylineToRibbonFilter();
    virtual                 ~avtPolylineToRibbonFilter();

    virtual const char      *GetType(void) { return "avtPolylineToRibbonFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Converts polylines into ribbons"; };

    double width;
    bool varyWidth;
    std::string widthVar;
    double widthFactor;
              
  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void                   UpdateDataObjectInfo(void);
};

#endif
