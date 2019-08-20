// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                 avtPluginStructuredChunkDataTreeIterator.h                //
// ************************************************************************* //

#ifndef AVT_PLUGIN_STRUCTURED_CHUNK_DATA_TREE_ITERATOR_H
#define AVT_PLUGIN_STRUCTURED_CHUNK_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtStructuredChunkDataTreeIterator.h>
#include <avtPluginFilter.h>


// ****************************************************************************
//  Class:  avtPluginStructuredChunkDataTreeIterator
//
//  Purpose:
//    A base class for plugin-style operators that are 
//    avtStructuredChunkDataTreeIterators.
//
//  Programmer:  Hank Childs
//  Creation:    March 27, 2005
//
// ****************************************************************************

class PIPELINE_API avtPluginStructuredChunkDataTreeIterator : 
    public virtual avtStructuredChunkDataTreeIterator, 
    public virtual avtPluginFilter
{
public:
    avtPluginStructuredChunkDataTreeIterator();
    virtual ~avtPluginStructuredChunkDataTreeIterator();
};

#endif


