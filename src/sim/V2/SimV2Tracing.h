/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#ifndef SIMV2_TRACING_H
#define SIMV2_TRACING_H
#include <stdio.h>

/* Internal libsim functions that let us write trace information to a log. */
FILE *simv2_trace_file(void);
void  simv2_set_trace_file(FILE *);
void  simv2_write_trace_indent(void);
void  simv2_begin_trace_indent(void);
void  simv2_end_trace_indent(void);

#ifdef DISABLE_LIBSIM_TRACING
/* When we're not tracing. */
#define LIBSIM_API_ENTER(FUNC)
#define LIBSIM_API_ENTER1(FUNC, FMT, A)
#define LIBSIM_API_ENTER2(FUNC, FMT, A, B)
#define LIBSIM_API_ENTER3(FUNC, FMT, A, B, C)
#define LIBSIM_API_LEAVE(FUNC)               
#define LIBSIM_API_LEAVE1(FUNC, FMT, A)      
#define LIBSIM_API_LEAVE2(FUNC, FMT, A, B)   
#define LIBSIM_API_LEAVE3(FUNC, FMT, A, B, C)
#define LIBSIM_MESSAGE(MSG)                  
#define LIBSIM_MESSAGE1(FMT, A)         
#define LIBSIM_MESSAGE_STRINGLIST(MSG, AC, AV)
#else
/* Tracing support is enabled by default. */
#define LIBSIM_API_ENTER(FUNC)                if(simv2_trace_file() != NULL) {\
                                                  simv2_begin_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s\n", #FUNC); \
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_API_ENTER1(FUNC, FMT, A)       if(simv2_trace_file() != NULL) {\
                                                  simv2_begin_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT, (A)); \
                                                  fprintf(simv2_trace_file(), "\n"); \
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_API_ENTER2(FUNC, FMT, A, B)    if(simv2_trace_file() != NULL) {\
                                                  simv2_begin_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT, (A), (B)); \
                                                  fprintf(simv2_trace_file(), "\n"); \
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_API_ENTER3(FUNC, FMT, A, B, C) if(simv2_trace_file() != NULL) {\
                                                  simv2_begin_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT, (A), (B), (C)); \
                                                  fprintf(simv2_trace_file(), "\n");\
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_API_LEAVE(FUNC)                if(simv2_trace_file() != NULL) {\
                                                  simv2_end_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s\n", #FUNC);\
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_API_LEAVE1(FUNC, FMT, A)       if(simv2_trace_file() != NULL) {\
                                                  simv2_end_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT, (A)); \
                                                  fprintf(simv2_trace_file(), "\n");\
                                                  fflush(simv2_trace_file()); \
                                              }


#define LIBSIM_API_LEAVE2(FUNC, FMT, A, B)    if(simv2_trace_file() != NULL) {\
                                                  simv2_end_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT, (A), (B)); \
                                                  fprintf(simv2_trace_file(), "\n");\
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_API_LEAVE3(FUNC, FMT, A, B, C) if(simv2_trace_file() != NULL) {\
                                                  simv2_end_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT, (A), (B), (C)); \
                                                  fprintf(simv2_trace_file(), "\n");\
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_MESSAGE(MSG)                   if(simv2_trace_file() != NULL) {\
                                                  simv2_write_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s\n", MSG); \
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_MESSAGE1(FMT, A)               if(simv2_trace_file() != NULL) {\
                                                  simv2_write_trace_indent(); \
                                                  fprintf(simv2_trace_file(), FMT, (A)); \
                                                  fprintf(simv2_trace_file(), "\n"); \
                                                  fflush(simv2_trace_file()); \
                                              }

#define LIBSIM_MESSAGE_STRINGLIST(MSG, AC, AV) if(simv2_trace_file() != NULL) {\
                                                   simv2_write_trace_indent(); \
                                                   fprintf(simv2_trace_file(), MSG); \
                                                   { int msli; \
                                                     for(msli = 0; msli < AC; ++msli) \
                                                         fprintf(simv2_trace_file(), "%s ", AV[msli]); \
                                                     fprintf(simv2_trace_file(), "\n"); \
                                                   }\
                                                   fflush(simv2_trace_file()); \
                                               }

#endif

#endif
