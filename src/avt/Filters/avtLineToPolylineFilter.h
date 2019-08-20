// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtLineToPolylineFilter.h                       //
// ************************************************************************* //

#ifndef AVT_LINE_TO_POLYLINE_FILTER_H
#define AVT_LINE_TO_POLYLINE_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtLineToPolylineFilter
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
// ****************************************************************************

class AVTFILTERS_API avtLineToPolylineFilter : virtual public avtDataTreeIterator
{
  public:
                             avtLineToPolylineFilter();
    virtual                 ~avtLineToPolylineFilter();

    virtual const char      *GetType(void) { return "avtLineToPolylineFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Grouping lines into polylines"; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void             UpdateDataObjectInfo(void);
};


#endif


