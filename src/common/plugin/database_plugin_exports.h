#ifndef DATABASE_PLUGIN_EXPORTS_H
#define DATABASE_PLUGIN_EXPORTS_H

#if defined(_WIN32)

#if defined(USING_MSVC6) || defined(USING_MSVC7)
// Disable inheritance by dominance warning message.
#pragma warning(disable:4250)
// Disable DLL interface warning.
#pragma warning(disable:4251)
#endif

//
// This file makes sure that the entry point to each plugin is exported
// in the DLL. It must be exported to be visible to GetProcAddress.
//
extern "C" __declspec(dllexport) const char *VisItPluginVersion;
#ifdef GENERAL_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) GeneralDatabasePluginInfo* GetGeneralInfo();
#endif
#ifdef MDSERVER_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) MDServerDatabasePluginInfo* GetMDServerInfo();
#endif
#ifdef ENGINE_PLUGIN_EXPORTS
extern "C" __declspec(dllexport) EngineDatabasePluginInfo* GetEngineInfo();
#endif
#endif

#endif
