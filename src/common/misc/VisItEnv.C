// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              VisItEnv.C                                //
// ************************************************************************* //
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif
#include <cstdlib>
#include <cstring>
#include <errno.h>
#ifdef  _WIN32
# include <windows.h>
#endif

#include <VisItEnv.h>

#include <DebugStream.h>


namespace VisItEnv {

// ****************************************************************************
//  Function: VisItEnv::get
//
//  Purpose:  Obtains a value from the environment.
//            `getenv' appears to be pretty standard; we don't do anything
//            special here.
//
//  Programmer: Tom Fogal
//
//    Alister Maguire, Tue Aug 18 14:10:07 PDT 2020
//    Changed to return a const char *.
//
// ****************************************************************************
const char *
get(const char *variable)
{
    return getenv(variable);
}

// ****************************************************************************
//  Function: VisItEnv::get
//
//  Purpose: Predicate to determine whether a variable is defined.
//
//  Programmer: Tom Fogal
//
// ****************************************************************************
bool
exists(const char *variable)
{
    char *value = getenv(variable);
    if(value == NULL)
    {
        return false;
    }
    return true;
}

// ****************************************************************************
//  Function: VisItEnv::set
//
//  Purpose: Sets a value in the enviroment.  Avoid using putenv, since it
//           requires static memory.
//
//  Programmer: Tom Fogal
//
//  Modifications:
//
//    Tom Fogal, Thu Apr 30 12:08:07 MDT 2009
//    Do the right thing on Windows (untested..)
//
// ****************************************************************************
void
set(const char *k, const char *v)
{
#ifdef _WIN32
    if(SetEnvironmentVariable(k, v) == 0)
#else
    if(setenv(k, v, 1) != 0)
#endif
    {
        debug1 << "setenv(" << k << " = " << v << ") failed!" << std::endl
#ifdef _WIN32
               << "Error: " << GetLastError() << std::endl;
#else
               << "Error: " << errno << ": '" << strerror(errno) << std::endl;
#endif
    }
}

// ****************************************************************************
//  Function: VisItEnv::unset
//
//  Purpose: Removes a variable definition from the environment.
//
//  Programmer: Tom Fogal
//
// ****************************************************************************
void
unset(const char *variable)
{
#ifdef HAVE_UNSETENV
# ifdef __APPLE__
    // Apple's unsetenv returns void; no error checking is possible.
    unsetenv(variable);
# else
    if(unsetenv(variable) != 0)
    {
        debug1 << "unsetenv(" << variable << ") failed!" << std::endl
               << "Error: " << errno << ": '" << strerror(errno) << std::endl;
    }
# endif
#else
    // level 5 because it doesn't usually matter if we clean up our environment
    // correctly; OS will do it when our process exits anyway.
    debug5 << "unsetenv(" << variable << ") ignored; unsetenv not supported "
           << "on this platform." << std::endl;
#endif
}

}  /* namespace VisItEnv */
