#ifndef AVT_PLUGIN_STREAMER_H
#define AVT_PLUGIN_STREAMER_H
#include <pipeline_exports.h>

#include <avtStreamer.h>
#include <avtPluginFilter.h>

// ****************************************************************************
//  Class:  avtPluginStreamer
//
//  Purpose:
//    A base class for plugin-style operators that are avtStreamers.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Apr 24 17:16:02 PST 2002
//    Added constructor/destructor.
//
// ****************************************************************************

class PIPELINE_API avtPluginStreamer : 
    public virtual avtStreamer, 
    public virtual avtPluginFilter
{
public:
    avtPluginStreamer();
    virtual ~avtPluginStreamer();
};

#endif
