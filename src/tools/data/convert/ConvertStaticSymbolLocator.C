// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <iostream>
#include <vector>

#include <DebugStream.h>
#include <plugin_entry_point.h>

class GeneralDatabasePluginInfo;
class EngineDatabasePluginInfo;
class GeneralPlotPluginInfo;
class EnginePlotPluginInfo;
class GeneralOperatorPluginInfo;
class EngineOperatorPluginInfo;

#define DECLARE_DATABASE(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralDatabasePluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,General)(VISIT_PLUGIN_ENTRY_ARGS); \
   extern "C" EngineDatabasePluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,Engine)(VISIT_PLUGIN_ENTRY_ARGS);

// Declare the databases.
#define PLUGIN_VERB DECLARE_DATABASE
#include <enabled_databases.h>
#undef PLUGIN_VERB


#define CHECK_PLUGIN(X) \
   if (sym == #X"VisItPluginVersion") \
       retval = (void *) &X##VisItPluginVersion; \
   else if (sym == VISIT_PLUGIN_ENTRY_FUNC_STR(X,General)) \
       retval = (void *) &VISIT_PLUGIN_ENTRY_FUNC(X,General); \
   else if (sym == VISIT_PLUGIN_ENTRY_FUNC_STR(X,Engine)) \
       retval = (void *) &VISIT_PLUGIN_ENTRY_FUNC(X,Engine); \


// Split apart to make it compile faster
#define PLUGIN_VERB CHECK_PLUGIN
static void *
database_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_databases.h>
    return retval;
}
#undef PLUGIN_VERB

void *
fake_dlsym(const std::string &sym)
{
    debug1 << "Asked for " << sym << endl;
    void *ptr = NULL;
    if((ptr = database_dlsym(sym)) != NULL)
        return ptr;

    debug1 << "fake_dlsym can't find symbol " << sym << endl;
    return NULL;
}


#ifdef PARALLEL
  #define ADD_DATABASE_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Database_par.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Database.a"));
#else
  #define ADD_DATABASE_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Database_ser.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Database.a"));
#endif

void
StaticGetSupportedLibs(std::vector<std::pair<std::string, std::string> > &libs,
                       const std::string &pluginType)
{   
    if (pluginType == "database")
    {
        // Add the databases
#define PLUGIN_VERB ADD_DATABASE_PLUGIN
#include <enabled_databases.h>
#undef PLUGIN_VERB
    }
}


