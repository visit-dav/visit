// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           SimPlotPluginManager.h                          //
// ************************************************************************* //

#ifndef SIM_PLOT_PLUGIN_MANAGER_H
#define SIM_PLOT_PLUGIN_MANAGER_H
#include <PlotPluginManager.h>
#include <map>

class ViewerWrappedEnginePlotPluginInfo;

// ****************************************************************************
//  Class: SimPlotPluginManager
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

class SimPlotPluginManager : public PlotPluginManager
{
public:
    SimPlotPluginManager();
    virtual ~SimPlotPluginManager();

    virtual ViewerPlotPluginInfo   *GetViewerPluginInfo(const std::string&);

private:
    std::map<std::string, ViewerWrappedEnginePlotPluginInfo *> wrapped;
};

#endif
