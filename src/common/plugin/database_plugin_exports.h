// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DATABASE_PLUGIN_EXPORTS_H
#define DATABASE_PLUGIN_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
# define DBP_EXPORT __declspec(dllexport)
#else
#define DBP_EXPORT
#endif
#else
# if __GNUC__ >= 4
#   define DBP_EXPORT __attribute__((visibility("default")))
# else
#   define DBP_EXPORT /* nothing */
# endif
#endif

#if defined(_WIN32) && defined(_MSC_VER)
// Disable inheritance by dominance warning message.
# pragma warning(disable:4250)
// Disable DLL interface warning.
# pragma warning(disable:4251)
#endif

//
// This file makes sure that the entry point to each plugin is exported
// in the DLL. It must be exported to be visible to GetProcAddress.
//
extern "C" DBP_EXPORT const char *VisItPluginVersion;
#ifdef GENERAL_PLUGIN_EXPORTS
extern "C" DBP_EXPORT GeneralDatabasePluginInfo* GetGeneralInfo();
#endif
#ifdef MDSERVER_PLUGIN_EXPORTS
extern "C" DBP_EXPORT MDServerDatabasePluginInfo* GetMDServerInfo();
#endif
#ifdef ENGINE_PLUGIN_EXPORTS
extern "C" DBP_EXPORT EngineDatabasePluginInfo* GetEngineInfo();
#endif

#endif
