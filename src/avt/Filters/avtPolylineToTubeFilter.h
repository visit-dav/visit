// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPolylineToTubeFilter.h                       //
// ************************************************************************* //

#ifndef AVT_POLYLINE_TO_TUBE_FILTER_H
#define AVT_POLYLINE_TO_TUBE_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

// ****************************************************************************
//  Class: avtPolylineToTubeFilter
//
//  Purpose:
//    Creates a tube from a polyline
//
//  Programmer: Allen Sanderson
//  Creation:   Feb 12 2016
//
// ****************************************************************************

class AVTFILTERS_API avtPolylineToTubeFilter : virtual public avtDataTreeIterator
{
  public:
                             avtPolylineToTubeFilter();
    virtual                 ~avtPolylineToTubeFilter();

    virtual const char      *GetType(void) { return "avtPolylineToTubeFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Converts polylines into tubes"; };

    int numberOfSides;
              
    double radius;
    bool varyRadius;
    std::string radiusVar;
    double radiusFactor;
              
  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void                   UpdateDataObjectInfo(void);
};

#endif
