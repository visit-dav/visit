// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                SysCall.h                                  //
// ************************************************************************* //
// Wrappers around system calls to protect against being interrupted.
#ifndef VISIT_SYSCALL_H
#define VISIT_SYSCALL_H

#include <cerrno>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif

inline int
RESTART_SELECT(int n_fds, fd_set *rd_fds, fd_set *wr_fds, fd_set *except_fds,
       struct timeval *tv)
{
    int retval;
    do {
        retval = select(n_fds, rd_fds, wr_fds, except_fds, tv);
    } while(retval == -1 && errno == EINTR);
    return retval;
}

#endif /* VISIT_SYSCALL_H */
