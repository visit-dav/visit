/* Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
   Project developers.  See the top-level LICENSE file for dates and other
   details.  No copyright assignment is required to contribute to VisIt.  */

/******************************************************************************
 *
 * Purpose: Small program with minimal dependencies, compileable to a Mach-O
 * binary executable that bootstraps VisIt launch process on macOS. This is the
 * executable that double-clicking the VisIt icon on macOS starts. It simply
 * turns around and launches VisIt as one ordinarily would on Linux.
 *
 * It is essential for this to be an actual compiled, Mach-O binary executable
 * and not a shell or python script for example, so that it is recognized by
 * macOS security framework as the VisIt application and can be managed by the
 * security framework as its own, unique application with its own unique
 * permissions/settings. See https://github.com/visit-dav/visit/issues/18182.
 *
 * It is assumed that argv[0], this executable's name which possibly includes
 * absolute or relative path components, is peer to ../Resources/bin/visit,
 * the place where the shell frontendlauncher bootstrap is installed on macOS
 * that really handles launching of VisIt.
 *
 * It aims to construct the path to that script and then start it with an
 * execv call.
 * 
 * Mark C. Miller, Wed Oct 12 09:48:36 PDT 2022
 *****************************************************************************/

#include <errno.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VISIT_MAX_ARGS 256

int
main(int argc, char **argv)
{
    char exe_tmp[PATH_MAX];
    char exe_real[PATH_MAX];
    char launcher[PATH_MAX];
    char *child_argv[VISIT_MAX_ARGS + 1];
    char *slash;
    uint32_t n;
    int i;

    if (argc > VISIT_MAX_ARGS)
        return E2BIG;

    n = sizeof(exe_tmp);
    if (_NSGetExecutablePath(exe_tmp, &n) != 0)
        return ENAMETOOLONG;

    if (realpath(exe_tmp, exe_real) == 0)
        return errno ? errno : EINVAL;

    slash = strrchr(exe_real, '/');
    if (slash == 0)
        return EINVAL;
    *slash = '\0';

    if (snprintf(launcher, sizeof(launcher),
                 "%s/../Resources/bin/visit", exe_real) >= (int) sizeof(launcher))
        return ENAMETOOLONG;

    child_argv[0] = launcher;
    for (i = 1; i < argc; i++)
        child_argv[i] = argv[i];
    child_argv[argc] = 0;

    execv(launcher, child_argv);

    return errno ? errno : EINVAL;
}
