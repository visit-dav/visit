#ifndef Operator_PLUGIN_EXPORTS_H
#define Operator_PLUGIN_EXPORTS_H

#if defined(_WIN32)
//
// This file makes sure that the entry point to each plugin is exported
// in the DLL. It must be exported to be visible to GetProcAddress.
//
#ifdef GENERAL_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) GeneralOperatorPluginInfo* GetGeneralInfo();
#endif
#ifdef GUI_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) GUIOperatorPluginInfo* GetGUIInfo();
#endif
#ifdef VIEWER_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) ViewerOperatorPluginInfo* GetViewerInfo();
#endif
#ifdef ENGINE_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) EngineOperatorPluginInfo* GetEngineInfo();
#endif
#ifdef SCRIPTING_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) ScriptingOperatorPluginInfo* GetScriptingInfo();
#endif
#endif

#endif
