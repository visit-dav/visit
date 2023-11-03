// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CONTROL_INTERFACE_RUNTIME_H
#define VISIT_CONTROL_INTERFACE_RUNTIME_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************
//  Library:  libsimV2runtime
//
//  Purpose:
//    Wrapper for simulations to control an engine.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 13 16:06:04 PST 2009
//
//  Modifications:
//    Brad Whitlock, Fri Aug 26 09:52:11 PDT 2011
//    I added simv2_set_mpicomm.
//
//    William T. Jones, Fri Jul 12 17:42:31 EST 2013
//    I added simv2_set_mpicomm_f.
//
//    Brad Whitlock, Thu Sep 18 16:16:29 PDT 2014
//    I added simv2_initialize_batch, simv2_exportdatabase, simv2_restoresession.
//
//    Brad Whitlock, Mon Feb  2 13:58:45 PST 2015
//    Added simv2_set_active_plots, changed set_plot_options, set_operator_options.
//
// ****************************************************************************

SIMV2_API void   *simv2_get_engine();
SIMV2_API int     simv2_get_descriptor(void*);
SIMV2_API int     simv2_initialize(void*, int argc, char *argv[]);
SIMV2_API int     simv2_initialize_batch(void*, int argc, char *argv[]);
SIMV2_API int     simv2_connect_viewer(void*, int argc, char *argv[]);
SIMV2_API int     simv2_process_input(void*);
SIMV2_API void    simv2_time_step_changed(void*);
SIMV2_API void    simv2_execute_command(void *, const char *);
SIMV2_API void    simv2_disconnect();
SIMV2_API void    simv2_set_worker_process_callback(void(*)());
SIMV2_API void    simv2_set_command_callback(void*,void(*)(const char*,const char*,void*),void*);
SIMV2_API int     simv2_save_window(void*, const char *, int, int, int);
SIMV2_API void    simv2_debug_logs(int level, const char *msg);
SIMV2_API int     simv2_set_mpicomm(void *);
SIMV2_API int     simv2_set_mpicomm_f(int *);

SIMV2_API int     simv2_add_plot(void *, const char *, const char *);
SIMV2_API int     simv2_add_operator(void *, const char *, int);
SIMV2_API int     simv2_draw_plots(void *);
SIMV2_API int     simv2_delete_active_plots(void *);
SIMV2_API int     simv2_set_active_plots(void *, const int *, int);
SIMV2_API int     simv2_change_plot_var(void *, const char *, int);

SIMV2_API int     simv2_set_plot_options(void *, const char *, int, void *, int);
SIMV2_API int     simv2_set_operator_options(void *, const char *, int, void *, int);

SIMV2_API int     simv2_exportdatabase(void *, const char *, const char *, visit_handle);
SIMV2_API int     simv2_exportdatabase_with_options(void *, const char *, const char *, visit_handle, visit_handle);
SIMV2_API int     simv2_restoresession(void *, const char *);

SIMV2_API int     simv2_set_view2D(void *, visit_handle);
SIMV2_API int     simv2_get_view2D(void *, visit_handle);
SIMV2_API int     simv2_set_view3D(void *, visit_handle);
SIMV2_API int     simv2_get_view3D(void *, visit_handle);

#ifdef __cplusplus
}
#endif

#endif
