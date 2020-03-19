// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PLUGIN_DATA_TREE_ITERATOR_H
#define AVT_PLUGIN_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtDataTreeIterator.h>
#include <avtPluginFilter.h>

// ****************************************************************************
//  Class:  avtPluginDataTreeIterator
//
//  Purpose:
//    A base class for plugin-style operators that are avtDataTreeIterators.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Apr 24 17:16:02 PST 2002
//    Added constructor/destructor.
//
// ****************************************************************************

class PIPELINE_API avtPluginDataTreeIterator : 
    public virtual avtDataTreeIterator, 
    public virtual avtPluginFilter
{
public:
    avtPluginDataTreeIterator();
    virtual ~avtPluginDataTreeIterator();
};

#endif
