// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef Operator_PLUGIN_EXPORTS_H
#define Operator_PLUGIN_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
# define OP_EXPORT __declspec(dllexport)
#else
#define OP_EXPORT
#endif
#else
# if __GNUC__ >= 4
#   define OP_EXPORT __attribute__((visibility("default")))
# else
#   define OP_EXPORT /* nothing */
# endif
#endif

//
// This file makes sure that the entry point to each plugin is exported
// in the DLL. It must be exported to be visible to GetProcAddress.
//
extern "C" OP_EXPORT const char *VisItPluginVersion;
#ifdef GENERAL_PLUGIN_EXPORTS
extern "C" OP_EXPORT GeneralOperatorPluginInfo* GetGeneralInfo();
#endif
#ifdef GUI_PLUGIN_EXPORTS
extern "C" OP_EXPORT GUIOperatorPluginInfo* GetGUIInfo();
#endif
#ifdef VIEWER_PLUGIN_EXPORTS
extern "C" OP_EXPORT ViewerOperatorPluginInfo* GetViewerInfo();
#endif
#ifdef ENGINE_PLUGIN_EXPORTS
extern "C" OP_EXPORT EngineOperatorPluginInfo* GetEngineInfo();
#endif
#ifdef SCRIPTING_PLUGIN_EXPORTS
extern "C" OP_EXPORT ScriptingOperatorPluginInfo* GetScriptingInfo();
#endif

#endif
