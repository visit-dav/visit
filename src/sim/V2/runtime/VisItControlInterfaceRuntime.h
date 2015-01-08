/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//    Brad Whitlock, Thu Sep 18 16:16:29 PDT 2014
//    I added simv2_initialize_batch, simv2_exportdatabase, simv2_restoresession.
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
SIMV2_API void    simv2_set_slave_process_callback(void(*)());
SIMV2_API void    simv2_set_command_callback(void*,void(*)(const char*,const char*,void*),void*);
SIMV2_API int     simv2_save_window(void*, const char *, int, int, int);
SIMV2_API void    simv2_debug_logs(int level, const char *msg);
SIMV2_API int     simv2_set_mpicomm(void *);

SIMV2_API int     simv2_add_plot(void *, const char *, const char *);
SIMV2_API int     simv2_add_operator(void *, const char *, int);
SIMV2_API int     simv2_draw_plots(void *);
SIMV2_API int     simv2_delete_active_plots(void *);
SIMV2_API int     simv2_set_plot_options(void *, int, const char *, int, void *, int);
SIMV2_API int     simv2_set_operator_options(void *, int, int, const char *, int, void *, int);

SIMV2_API int     simv2_exportdatabase(void *, const char *, const char *, visit_handle);
SIMV2_API int     simv2_restoresession(void *, const char *);

#ifdef __cplusplus
}
#endif

#endif
