// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <iostream>
#include <vector>

#include <DebugStream.h>
#include <plugin_entry_point.h>

class GeneralDatabasePluginInfo;
class MDServerDatabasePluginInfo;

#define DECLARE_DATABASE(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralDatabasePluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,General)(VISIT_PLUGIN_ENTRY_ARGS); \
   extern "C" MDServerDatabasePluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,MDServer)(VISIT_PLUGIN_ENTRY_ARGS);

// Declare the databases.
#define PLUGIN_VERB DECLARE_DATABASE
#include <enabled_databases.h>
PLUGIN_VERB(SimV2)
#undef PLUGIN_VERB


#define CHECK_PLUGIN(X) \
   if (sym == #X"VisItPluginVersion") \
       retval = (void *) &X##VisItPluginVersion; \
   else if (sym == VISIT_PLUGIN_ENTRY_FUNC_STR(X,General)) \
       retval = (void *) &VISIT_PLUGIN_ENTRY_FUNC(X,General); \
   else if (sym == VISIT_PLUGIN_ENTRY_FUNC_STR(X,MDServer)) \
       retval = (void *) &VISIT_PLUGIN_ENTRY_FUNC(X,MDServer); \


#define PLUGIN_VERB CHECK_PLUGIN
static void *
database_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_databases.h>
    PLUGIN_VERB(SimV2)
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

#define ADD_DATABASE_PLUGIN(X) \
    libs.push_back(std::pair<std::string, std::string>("", "libM"#X"Database.a"));; \
    libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Database.a"));

void
StaticGetSupportedLibs(std::vector<std::pair<std::string, std::string> > &libs,
                       const std::string &pluginType)
{   
    if (pluginType == "database")
    {
        // Add the databases
#define PLUGIN_VERB ADD_DATABASE_PLUGIN
#include <enabled_databases.h>
        PLUGIN_VERB(SimV2)
#undef PLUGIN_VERB
    }
}


