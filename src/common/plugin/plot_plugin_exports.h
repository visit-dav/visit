// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLOT_PLUGIN_EXPORTS_H
#define PLOT_PLUGIN_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
# define PLOT_EXPORT __declspec(dllexport)
#else
#define PLOT_EXPORT
#endif
#else
# if __GNUC__ >= 4
#   define PLOT_EXPORT __attribute__((visibility("default")))
# else
#   define PLOT_EXPORT /* nothing */
# endif
#endif

//
// This file makes sure that the entry point to each plugin is exported
// in the DLL. It must be exported to be visible to GetProcAddress.
//
extern "C" PLOT_EXPORT const char *VisItPluginVersion;
#ifdef GENERAL_PLUGIN_EXPORTS
extern "C" PLOT_EXPORT GeneralPlotPluginInfo* GetGeneralInfo();
#endif
#ifdef GUI_PLUGIN_EXPORTS
extern "C" PLOT_EXPORT GUIPlotPluginInfo* GetGUIInfo();
#endif
#ifdef VIEWER_PLUGIN_EXPORTS
extern "C" PLOT_EXPORT ViewerPlotPluginInfo* GetViewerInfo();
#endif
#ifdef ENGINE_PLUGIN_EXPORTS
extern "C" PLOT_EXPORT EnginePlotPluginInfo* GetEngineInfo();
#endif
#ifdef SCRIPTING_PLUGIN_EXPORTS
extern "C" PLOT_EXPORT ScriptingPlotPluginInfo* GetScriptingInfo();
#endif

#endif
