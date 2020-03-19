// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              Environment.h                                //
// ************************************************************************* //
#ifndef ENV_H
#define ENV_H
#include <misc_exports.h>

#include <string>

namespace Environment
{
    // ***********************************************************************
    //  Function: Environment::get
    //
    //  Purpose:  Obtains a value from the environment.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    std::string MISC_API get(const char *);

    // ***********************************************************************
    //  Function: Environment::get
    //
    //  Purpose: Predicate to determine whether a variable is defined.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    bool MISC_API exists(const char *);

    // ***********************************************************************
    //  Function: Environment::set
    //
    //  Purpose: Sets a value in the enviroment.  Avoid using putenv, since it
    //           requires static memory.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    void MISC_API set(const char *k, const char *v);

    // ***********************************************************************
    //  Function: Environment::unset
    //
    //  Purpose: Removes a variable definition from the environment.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    void MISC_API unset(const char *);
}
#endif
