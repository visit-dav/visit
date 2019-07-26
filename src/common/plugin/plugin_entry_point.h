// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLUGIN_ENTRY_POINT_H
#define PLUGIN_ENTRY_POINT_H

/* Plugin entry point macros. This enables easy customization of plugin entry
 * point functions, etc.
 */
#define VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)     NAME##_Get##FLAVOR##Info
#define VISIT_PLUGIN_ENTRY_FUNC_STR(NAME,FLAVOR) #NAME"_Get"#FLAVOR"Info"
#define VISIT_PLUGIN_ENTRY_ARGS void
#define VISIT_PLUGIN_ENTRY_NAMED_ARGS void
#define VISIT_PLUGIN_ENTRY_ARGS_DECLARE
#define VISIT_PLUGIN_ENTRY_ARGS_CALL

#define VISIT_PLOT_PLUGIN_ENTRY(NAME, FLAVOR) \
extern "C" PLOT_EXPORT FLAVOR##PlotPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    return new NAME##FLAVOR##PluginInfo; \
}
#define VISIT_PLOT_PLUGIN_ENTRY_EV(NAME, FLAVOR) \
extern "C" PLOT_EXPORT FLAVOR##PlotPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    NAME##FLAVOR##PluginInfo::InitializeGlobalObjects(); \
    return new NAME##FLAVOR##PluginInfo; \
}

#define VISIT_OPERATOR_PLUGIN_ENTRY(NAME, FLAVOR) \
extern "C" OP_EXPORT FLAVOR##OperatorPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    return new NAME##FLAVOR##PluginInfo; \
}
#define VISIT_OPERATOR_PLUGIN_ENTRY_EV(NAME, FLAVOR) \
extern "C" OP_EXPORT FLAVOR##OperatorPluginInfo* VISIT_PLUGIN_ENTRY_FUNC(NAME,FLAVOR)(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    NAME##FLAVOR##PluginInfo::InitializeGlobalObjects(); \
    return new NAME##FLAVOR##PluginInfo; \
}

/* NOTE: We can't seem to use VISIT_PLUGIN_ENTRY_FUNC in this one because
 *       it keeps evaluating to 1_GetEngineInfo when we do ITAPS.
 */
#define VISIT_DATABASE_PLUGIN_ENTRY(NAME, FLAVOR) \
extern "C" DBP_EXPORT FLAVOR##DatabasePluginInfo* NAME##_Get##FLAVOR##Info(VISIT_PLUGIN_ENTRY_NAMED_ARGS) \
{ \
    return new NAME##FLAVOR##PluginInfo; \
}

#endif
