// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPolylineAddEndPointsFilter.h                 //
// ************************************************************************* //

#ifndef AVT_POLYLINE_ADD_END_POINTS_FILTER_H
#define AVT_POLYLINE_ADD_END_POINTS_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

// ****************************************************************************
//  Class: avtPolylineAddEndPointsFilter
//
//  Purpose:
//    Connects individual line cells into a polyline cell.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:50:35 PDT 2009
//
//  Modifications:
//    Eric Brugger, Mon Jul 21 13:51:03 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Tue Oct 25 14:16:39 PDT 2016
//    I modified the class to support independently setting the point
//    style for two end points.
//
// ****************************************************************************

class AVTFILTERS_API avtPolylineAddEndPointsFilter : virtual public avtDataTreeIterator
{
  public:
    enum EndPointStyle
    {
        None,
        Spheres,
        Cones
    };
    
                             avtPolylineAddEndPointsFilter();
    virtual                 ~avtPolylineAddEndPointsFilter();

    virtual const char      *GetType(void) { return "avtPolylineAddEndPointsFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Add end points to polylines"; };

    int tailStyle;
    int headStyle;
    int resolution;

    double radius;
    double ratio;

    bool varyRadius;
    std::string radiusVar;
    double radiusFactor;
              
  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void                   UpdateDataObjectInfo(void);
};

#endif
