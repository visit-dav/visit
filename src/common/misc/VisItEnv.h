// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              VisItEnv.h                                //
// ************************************************************************* //
#ifndef ENV_H
#define ENV_H
#include <misc_exports.h>

#include <string>

namespace VisItEnv
{
    // ***********************************************************************
    //  Function: VisItEnv::get
    //
    //  Purpose:  Obtains a value from the environment.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    const char *MISC_API get(const char *);

    // ***********************************************************************
    //  Function: VisItEnv::get
    //
    //  Purpose: Predicate to determine whether a variable is defined.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    bool MISC_API exists(const char *);

    // ***********************************************************************
    //  Function: VisItEnv::set
    //
    //  Purpose: Sets a value in the enviroment.  Avoid using putenv, since it
    //           requires static memory.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    void MISC_API set(const char *k, const char *v);

    // ***********************************************************************
    //  Function: VisItEnv::unset
    //
    //  Purpose: Removes a variable definition from the environment.
    //
    //  Programmer: Tom Fogal
    //
    // ***********************************************************************
    void MISC_API unset(const char *);
}
#endif
