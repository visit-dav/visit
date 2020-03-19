// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#define LIBSIM_API_LEAVE0(FUNC, FMT)
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

#define LIBSIM_API_LEAVE0(FUNC, FMT)          if(simv2_trace_file() != NULL) {\
                                                  simv2_end_trace_indent(); \
                                                  fprintf(simv2_trace_file(), "%s ", #FUNC); \
                                                  fprintf(simv2_trace_file(), FMT); \
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
