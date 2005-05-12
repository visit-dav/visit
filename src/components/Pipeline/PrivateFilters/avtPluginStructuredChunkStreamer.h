// ************************************************************************* //
//                     avtPluginStructuredChunkStreamer.h                    //
// ************************************************************************* //

#ifndef AVT_PLUGIN_STRUCTURED_CHUNK_STREAMER_H
#define AVT_PLUGIN_STRUCTURED_CHUNK_STREAMER_H

#include <pipeline_exports.h>

#include <avtStructuredChunkStreamer.h>
#include <avtPluginFilter.h>


// ****************************************************************************
//  Class:  avtPluginStructuredChunkStreamer
//
//  Purpose:
//    A base class for plugin-style operators that are 
//    avtStructuredChunkStreamers.
//
//  Programmer:  Hank Childs
//  Creation:    March 27, 2005
//
// ****************************************************************************
class PIPELINE_API avtPluginStructuredChunkStreamer : 
    public virtual avtStructuredChunkStreamer, 
    public virtual avtPluginFilter
{
public:
    avtPluginStructuredChunkStreamer();
    virtual ~avtPluginStructuredChunkStreamer();
};

#endif


