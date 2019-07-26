// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLUGIN_BROADCASTER_H
#define PLUGIN_BROADCASTER_H
#include <plugin_exports.h>
#include <vectortypes.h>

// ****************************************************************************
// Class: PluginBroadcaster
//
// Purpose:
//   This class defines an interface that the plugin manager can use for
//   broadcasting plugin data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 18 11:18:21 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class PLUGIN_API PluginBroadcaster
{
public:
    PluginBroadcaster();
    virtual ~PluginBroadcaster();

    virtual void BroadcastStringVector(stringVector &) = 0;
    virtual void BroadcastBoolVector(boolVector &) = 0;
    virtual void BroadcastStringVectorVector(std::vector<std::vector<std::string> > &) = 0;
};

#endif
