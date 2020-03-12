// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: ThresholdEnginePluginInfo.C
// ****************************************************************************

#include <ThresholdPluginInfo.h>
#include <avtThresholdFilter.h>

VISIT_OPERATOR_PLUGIN_ENTRY_EV(Threshold,Engine)

// ****************************************************************************
//  Method: ThresholdEnginePluginInfo::AllocAvtPluginFilter
//
//  Purpose:
//    Return a pointer to a newly allocated avtPluginFilter.
//
//  Returns:    A pointer to the newly allocated avtPluginFilter.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:40:13 PST 2010
//
// ****************************************************************************
#include <avtThresholdPluginFilter.h>
avtPluginFilter *
ThresholdEnginePluginInfo::AllocAvtPluginFilter()
{
    return new avtThresholdPluginFilter;
}

