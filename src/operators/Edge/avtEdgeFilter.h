// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtEdgeFilter.h
// ************************************************************************* //

#ifndef AVT_Edge_FILTER_H
#define AVT_Edge_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <EdgeAttributes.h>


// ****************************************************************************
//  Class: avtEdgeFilter
//
//  Purpose:
//      Extract the edges from polygons.  Like the mesh filter, but
//      relies purely on VTK and makes no use of visit-specific
//      optimizations.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 23, 2009
//
//  Modifications:
//    Eric Brugger, Thu Jul 24 09:28:40 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtEdgeFilter : public avtPluginDataTreeIterator
{
  public:
                         avtEdgeFilter();
    virtual             ~avtEdgeFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtEdgeFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Edge"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    EdgeAttributes   atts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    void                  UpdateDataObjectInfo();
};


#endif
