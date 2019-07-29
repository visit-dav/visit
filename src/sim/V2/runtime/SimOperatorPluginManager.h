// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           SimOperatorPluginManager.h                      //
// ************************************************************************* //

#ifndef SIM_OPERATOR_PLUGIN_MANAGER_H
#define SIM_OPERATOR_PLUGIN_MANAGER_H
#include <OperatorPluginManager.h>
#include <map>

class ViewerWrappedEngineOperatorPluginInfo;

// ****************************************************************************
//  Class: SimOperatorPluginManager
//
//  Purpose:
//    Uses the engine's plugin manager to impersonate a plugin manager for
//    the viewer.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 17 16:09:08 PDT 2014
//
//  Modifications:
//
// ****************************************************************************

class SimOperatorPluginManager : public OperatorPluginManager
{
public:
    SimOperatorPluginManager();
    virtual ~SimOperatorPluginManager();

    virtual ViewerOperatorPluginInfo   *GetViewerPluginInfo(const std::string&);

private:
    std::map<std::string, ViewerWrappedEngineOperatorPluginInfo *> wrapped;
};

#endif
