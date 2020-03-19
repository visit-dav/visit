// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PLUGIN_DATATREESTREAMER_H
#define AVT_PLUGIN_DATATREESTREAMER_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtPluginFilter.h>

// ****************************************************************************
//  Class:  avtPluginDatasetToDatasetFilter
//
//  Purpose:
//    A base class for plugin-style operators that are avtDatasetToDatasetFilters.
//
//  Programmer:  Hank Childs
//  Creation:    January 24, 2008
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug 23 15:22:34 MST 2010
//    Added constructor/destructor.
//
// ****************************************************************************

class PIPELINE_API avtPluginDatasetToDatasetFilter : 
    public virtual avtDatasetToDatasetFilter, 
    public virtual avtPluginFilter
{
public:
    avtPluginDatasetToDatasetFilter();
    virtual ~avtPluginDatasetToDatasetFilter();
};

#endif
