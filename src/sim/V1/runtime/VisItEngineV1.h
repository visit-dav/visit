/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef VISIT_ENGINE_H
#define VISIT_ENGINE_H

#if defined(_WIN32)
# if defined(visitenginev1_ser_EXPORTS) || defined(visitenginev1_par_EXPORTS)
#   define SIMV1_API  __declspec(dllexport)
# else
#   define SIMV1_API  __declspec(dllimport)
# endif
# if defined(_MSC_VER)
/* Turn off warning about lack of DLL interface */
#   pragma warning(disable:4251)
/* Turn off warning non-dll class is base for dll-interface class. */
#   pragma warning(disable:4275)
/* Turn off warning about identifier truncation */
#   pragma warning(disable:4786)
# endif
#else
# if __GNUC__ >= 4
#    if defined(visitenginev1_ser_EXPORTS) || defined(visitenginev1_par_EXPORTS)
#      define SIMV1_API __attribute__ ((visibility("default")))
#    else
#      define SIMV1_API /* hidden by default */
#    endif
# else
#   define SIMV1_API  /* hidden by default */
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ****************************************************************************
//  Library:  VisItEngine
//
//  Purpose:
//    Wrapper for simulations to control an engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Nov  1 17:19:02 PST 2004
//    Added parallel simulation support.
//
//    Jeremy Meredith, Fri Mar 18 08:28:33 PST 2005
//    Added simulation control support.
//
//    Brad Whitlock, Thu Jan 25 17:55:04 PST 2007
//    Added update_plots and execute_command.
//
// ****************************************************************************

SIMV1_API void   *get_engine();
SIMV1_API int     get_descriptor(void*);
SIMV1_API int     initialize(void*, int argc, char *argv[]);
SIMV1_API int     connect_to_viewer(void*, int argc, char *argv[]);
SIMV1_API int     process_input(void*);
SIMV1_API void    time_step_changed(void*);
SIMV1_API void    update_plots(void *);
SIMV1_API void    execute_command(void *, const char *);
SIMV1_API void    disconnect();
SIMV1_API void    set_slave_process_callback(void(*)());
SIMV1_API void    set_command_callback(void*,void(*)(const char*,int,float,const char*));

#ifdef __cplusplus
}
#endif

#endif
