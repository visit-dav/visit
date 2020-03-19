// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtDelaunayFilter.h
// ************************************************************************* //

#ifndef AVT_Delaunay_FILTER_H
#define AVT_Delaunay_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <DelaunayAttributes.h>


// ****************************************************************************
//  Class: avtDelaunayFilter
//
//  Purpose:
//      A plugin operator for Delaunay.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 25, 2009
//
//  Modifications:
//    Eric Brugger, Wed Jul 23 11:59:54 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtDelaunayFilter : public avtPluginDataTreeIterator
{
  public:
                         avtDelaunayFilter();
    virtual             ~avtDelaunayFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtDelaunayFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Delaunay"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    DelaunayAttributes   atts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    void                 UpdateDataObjectInfo();

};


#endif
