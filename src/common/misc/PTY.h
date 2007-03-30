#ifndef PTY_H
#define PTY_H

#include <visit-config.h>
#ifdef USE_PTY

#include <stdlib.h>
#include <unistd.h>

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
// ****************************************************************************

int    ptym_open(char*);
int    ptys_open(int, char*);
pid_t  pty_fork(int&, void (*)(int) =NULL, char * =NULL);

#endif

#endif
