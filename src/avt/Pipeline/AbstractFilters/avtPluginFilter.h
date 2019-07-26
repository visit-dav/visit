// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PLUGIN_FILTER_H
#define AVT_PLUGIN_FILTER_H
#include <pipeline_exports.h>

#include <avtFilter.h>
class AttributeGroup;

// ****************************************************************************
//  Class:  avtPluginSIMODataTreeIterator
//
//  Purpose:
//    A base class for plugin-style operators.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
// ****************************************************************************
class PIPELINE_API avtPluginFilter : public virtual avtFilter
{
  public:
    virtual void                     SetAtts(const AttributeGroup*) = 0;
};

#endif
