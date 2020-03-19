// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <iostream>
#include <vector>

#include <DebugStream.h>
#include <plugin_entry_point.h>

class GeneralPlotPluginInfo;
class ViewerPlotPluginInfo;
class GeneralOperatorPluginInfo;
class ViewerOperatorPluginInfo;

#define DECLARE_OPERATOR(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralOperatorPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,General)(VISIT_PLUGIN_ENTRY_ARGS); \
   extern "C" ViewerOperatorPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,Viewer)(VISIT_PLUGIN_ENTRY_ARGS);

#define DECLARE_PLOT(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralPlotPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,General)(VISIT_PLUGIN_ENTRY_ARGS); \
   extern "C" ViewerPlotPluginInfo *VISIT_PLUGIN_ENTRY_FUNC(X,Viewer)(VISIT_PLUGIN_ENTRY_ARGS);

// Declare the plots.
#define PLUGIN_VERB DECLARE_PLOT
#include <enabled_plots.h>
#undef PLUGIN_VERB

// Declare the operators.
#define PLUGIN_VERB DECLARE_OPERATOR
#include <enabled_operators.h>
#undef PLUGIN_VERB

#define CHECK_PLUGIN(X) \
   if (sym == #X"VisItPluginVersion") \
       retval = (void *) &X##VisItPluginVersion; \
   else if (sym == VISIT_PLUGIN_ENTRY_FUNC_STR(X,General)) \
       retval = (void *) &VISIT_PLUGIN_ENTRY_FUNC(X,General); \
   else if (sym == VISIT_PLUGIN_ENTRY_FUNC_STR(X,Viewer)) \
       retval = (void *) &VISIT_PLUGIN_ENTRY_FUNC(X,Viewer); \


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
#undef PLUGIN_VERB

void *
fake_dlsym(const std::string &sym)
{
    debug1 << "Asked for " << sym << endl;
    void *ptr = NULL;
    if((ptr = plot_dlsym(sym)) != NULL)
        return ptr;
    if((ptr = operator_dlsym(sym)) != NULL)
        return ptr;

    debug1 << "fake_dlsym can't find symbol " << sym << endl;
    return NULL;
}


#define ADD_OPERATOR_PLUGIN(X) \
    libs.push_back(std::pair<std::string, std::string>("", "libV"#X"Operator.a"));; \
    libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Operator.a"));

#define ADD_PLOT_PLUGIN(X) \
    libs.push_back(std::pair<std::string, std::string>("", "libV"#X"Plot.a")); \
    libs.push_back(std::pair<std::string, std::string>("", "libI"#X"Plot.a"));

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
}


