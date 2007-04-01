#ifndef PLOT_PLUGIN_EXPORTS_H
#define PLOT_PLUGIN_EXPORTS_H

#if defined(_WIN32)
//
// This file makes sure that the entry point to each plugin is exported
// in the DLL. It must be exported to be visible to GetProcAddress.
//
extern "C" __declspec(dllexport) const char *VisItPluginVersion;
#ifdef GENERAL_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) GeneralPlotPluginInfo* GetGeneralInfo();
#endif
#ifdef GUI_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) GUIPlotPluginInfo* GetGUIInfo();
#endif
#ifdef VIEWER_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) ViewerPlotPluginInfo* GetViewerInfo();
#endif
#ifdef ENGINE_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) EnginePlotPluginInfo* GetEngineInfo();
#endif
#ifdef SCRIPTING_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) ScriptingPlotPluginInfo* GetScriptingInfo();
#endif
#endif

#endif
