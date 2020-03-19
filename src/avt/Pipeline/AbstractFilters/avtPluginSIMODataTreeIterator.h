// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PLUGIN_SIMO_DATATREE_ITERATOR_H
#define AVT_PLUGIN_SIMO_DATATREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtSIMODataTreeIterator.h>
#include <avtPluginFilter.h>

// ****************************************************************************
//  Class:  avtPluginSIMODataTreeIterator
//
//  Purpose:
//    A base class for plugin-style operators that are avtSIMODataTreeIterators.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
// ****************************************************************************

class PIPELINE_API avtPluginSIMODataTreeIterator : 
    public virtual avtSIMODataTreeIterator, 
    public virtual avtPluginFilter
{
};

#endif


