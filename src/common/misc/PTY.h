// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PTY_H
#define PTY_H

#include <visit-config.h>
#ifdef VISIT_USE_PTY

#include <stdlib.h>
#include <unistd.h>
#include <misc_exports.h>

// ****************************************************************************
//  Purpose: PTY Functions
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 22 14:59:54 PDT 2002
//    Added an optional child signal handler to pty_fork.
//
//    Tom Fogal, Tue Jun 30 21:05:32 MDT 2009
//    Add defines for symbol versioning.
//
// ****************************************************************************

int   MISC_API ptym_open(char*);
int   MISC_API ptys_open(int, char*);
pid_t MISC_API pty_fork(int&, void (*)(int) =NULL, char * =NULL);

#endif

#endif
