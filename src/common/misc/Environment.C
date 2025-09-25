// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              Environment.C                                //
// ************************************************************************* //
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif
#include <cstdlib>
#include <cstring>
#include <errno.h>
#ifdef  _WIN32
# include <windows.h>
#else
// access `environ` pointer for env var strings in non windows oses
#include <unistd.h>
extern char **environ;
#endif

#include <Environment.h>

#include <DebugStream.h>


namespace Environment {

// ****************************************************************************
//  Function: Environment::get
//
//  Purpose:  Obtains a value from the environment.
//            `getenv' appears to be pretty standard; we don't do anything
//            special here.
//
//  Programmer: Tom Fogal
//
// ****************************************************************************
std::string
get(const char *variable)
{
    const char *value = getenv(variable);
    if (value == NULL)
    {
        std::string str;
        return str;
    }
    return std::string(value);
}

// ****************************************************************************
//  Function: Environment::get
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
//  Function: Environment::set
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
//  Function: Environment::unset
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



// ****************************************************************************
//  Function: Environment::variable_strings
//
//  Purpose: Lists env var strings
//
//  Programmer: Cyrus Harrison
//
//  Modifications:
//    Kathleen Biagas, Thu Sep 25, 2025
//    Ensure lp_env_strs is not modified. Use FreeEnvironemntStringsA to
//    match GetEnvironmentStringsA.
//
// ****************************************************************************
void
variable_strings(std::vector<std::string> &var_strs)
{
    var_strs.clear();
#ifdef  _WIN32 // windows case
// Interesting notes on Windows GetEnvironmentStrings
//   https://www.os2museum.com/wp/the-strange-case-of-getenvironmentstringsa/
//   https://devblogs.microsoft.com/oldnewthing/20130117-00/?p=5533
//
// I decided best course is to use ANSI version: GetEnvironmentStringsA

    LPTCH lp_env_strs = GetEnvironmentStringsA();
    LPTSTR lpszVar;

    if(lp_env_strs == NULL)
    {
        debug1 << "GetEnvironmentStringsA returned NULL." << endl;
        return;
    }

    lpszVar = (LPTSTR) lp_env_strs;
    while (*lpszVar)
    {
        var_strs.push_back(std::string(lpszVar));
        lpszVar += lstrlen(lpszVar)+1;
    }
    
    // Need to call the same version of FreeEnvironmentStrings as
    // GetEnvironmentStrings.
    FreeEnvironmentStringsA(lp_env_strs);

#else // all other oses

    char **env_var_ptr = environ;
    while(*env_var_ptr != NULL)
    {
        var_strs.push_back(std::string(*env_var_ptr));
        env_var_ptr++;
    }
#endif
    
}

// ****************************************************************************
//  Function: Environment::variable_names
//
//  Purpose: Lists env var names
//
//  Programmer: Cyrus Harrison
//
// ****************************************************************************
void
variable_names(std::vector<std::string> &names)
{
    names.clear();
    // get full env var strings
    std::vector<std::string> var_strs;
    variable_strings(var_strs);
    // for each strip, the var name portion is before the `=`
    for(auto var_str : var_strs)
    {
        size_t eq_loc = var_str.find("=");
        if(eq_loc!= std::string::npos)
        {
            names.push_back(var_str.substr(0,eq_loc));
        }
        else 
        {
            // error?
        }
    }
    
}

}  /* namespace Environment */
