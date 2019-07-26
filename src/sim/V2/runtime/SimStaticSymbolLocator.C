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

#define DECLARE_OPERATOR(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralOperatorPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,General)(VISIT_PLUGIN_ENTRY_ARGS); \
   extern "C" EngineOperatorPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,Engine)(VISIT_PLUGIN_ENTRY_ARGS);

#define DECLARE_PLOT(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralPlotPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,General)(VISIT_PLUGIN_ENTRY_ARGS); \
   extern "C" EnginePlotPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,Engine)(VISIT_PLUGIN_ENTRY_ARGS);

// Declare the plots.
#define PLUGIN_VERB DECLARE_PLOT
#include <enabled_plots.h>
#undef PLUGIN_VERB

// Declare the operators.
#define PLUGIN_VERB DECLARE_OPERATOR
#include <enabled_operators.h>
#undef PLUGIN_VERB

// Declare the databases.
#define PLUGIN_VERB DECLARE_DATABASE
#ifdef HAVE_XDB
PLUGIN_VERB(FieldViewXDB)
#endif
PLUGIN_VERB(VTK)
PLUGIN_VERB(SimV2)
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
plot_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_plots.h>
    return retval;
}

static void *
operator_dlsym(const std::string &sym)
{
    void *retval = NULL;
    #include <enabled_operators.h>
    return retval;
}

static void *
database_dlsym(const std::string &sym)
{
    void *retval = NULL;
#ifdef HAVE_XDB
    PLUGIN_VERB(FieldViewXDB)
#endif
    PLUGIN_VERB(VTK)
    PLUGIN_VERB(SimV2)
    return retval;
}
#undef PLUGIN_VERB

void *
fake_dlsym(const std::string &sym)
{
    void *ptr = NULL;
    if((ptr = plot_dlsym(sym)) != NULL)
        return ptr;
    if((ptr = operator_dlsym(sym)) != NULL)
        return ptr;
    if((ptr = database_dlsym(sym)) != NULL)
        return ptr;

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

#ifdef PARALLEL
  #define ADD_OPERATOR_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Operator_par.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));
#else
  #define ADD_OPERATOR_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Operator_ser.a"));; \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));
#endif

#ifdef PARALLEL
  #define ADD_PLOT_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Plot_par.a")); \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Plot.a"));
#else
  #define ADD_PLOT_PLUGIN(X) \
       libs.push_back(std::pair<std::string, std::string>("", "libE"#X"Plot_ser.a")); \
       libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Plot.a"));
#endif

void
StaticGetSupportedLibs(std::vector<std::pair<std::string, std::string> > &libs,
                       const std::string &pluginType)
{
    if (pluginType == "plot")
    {
        // Add the plots.
#define PLUGIN_VERB ADD_PLOT_PLUGIN
#include <enabled_plots.h>
#undef PLUGIN_VERB
    }

    if (pluginType == "operator")
    {
        // Add the operators
#define PLUGIN_VERB ADD_OPERATOR_PLUGIN
#include <enabled_operators.h>
#undef PLUGIN_VERB
    }
    
    if (pluginType == "database")
    {
        // Add the databases
#define PLUGIN_VERB ADD_DATABASE_PLUGIN
#ifdef HAVE_XDB
PLUGIN_VERB(FieldViewXDB)
#endif
PLUGIN_VERB(VTK)
PLUGIN_VERB(SimV2)
#undef PLUGIN_VERB
    }
}


