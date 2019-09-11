// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPolylineCleanupFilter.h                       //
// ************************************************************************* //

#ifndef AVT_POLYLINE_CLEANUP_FILTER_H
#define AVT_POLYLINE_CLEANUP_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

// ****************************************************************************
//  Class: avtPolylineCleanupFilter
//
//  Purpose:
//    Cleans up duplicate points from a poly line
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Nov  7 10:00:34 PST 2016
//
// ****************************************************************************

class AVTFILTERS_API avtPolylineCleanupFilter : virtual public avtDataTreeIterator
{
  public:
                             avtPolylineCleanupFilter();
    virtual                 ~avtPolylineCleanupFilter();

    virtual const char      *GetType(void)
                                { return "avtPolylineCleanupFilter"; };
    virtual const char      *GetDescription(void)
                                { return "Cleans up duplicate points from a poly line"; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void                   UpdateDataObjectInfo(void);
};

#endif
