#ifndef AVT_PLUGIN_DATATREESTREAMER_H
#define AVT_PLUGIN_DATATREESTREAMER_H
#include <pipeline_exports.h>

#include <avtDataTreeStreamer.h>
#include <avtPluginFilter.h>

// ****************************************************************************
//  Class:  avtPluginDataTreeStreamer
//
//  Purpose:
//    A base class for plugin-style operators that are avtDataTreeStreamers.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
// ****************************************************************************
class PIPELINE_API avtPluginDataTreeStreamer : 
    public virtual avtDataTreeStreamer, 
    public virtual avtPluginFilter
{
};

#endif
