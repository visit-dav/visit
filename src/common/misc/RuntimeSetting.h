// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              RuntimeSetting.h                             //
// ************************************************************************* //
#ifndef VISIT_RUNTIME_SETTING_H
#define VISIT_RUNTIME_SETTING_H
#include <misc_exports.h>

#include <string>

// A runtime setting is an abstraction for obtaining configuration information.
// There is a precedence order for obtaining preferences:
//    1. defaults hardcoded at compile time
//    2. values set in a configuration file
//    3. obtained from an environment variable
//    4. specified via a command line option
// Later values override earlier settings.
namespace RuntimeSetting
{
    // ****************************************************************************
    //  Function: RuntimeSetting::lookup
    //
    //  Purpose:  Looks up a value
    //
    //  Programmer: Tom Fogal
    //  Creation: June 25, 2009
    //
    // ****************************************************************************
    int MISC_API         lookupi(const char *key);
    double MISC_API      lookupf(const char *key);
    std::string MISC_API lookups(const char *key);
    bool MISC_API        lookupb(const char *key);

    // ****************************************************************************
    //  Function: RuntimeSetting::parse_command_line
    //
    //  Purpose:  Creates overrides based on command line parameters.
    //
    //  Programmer: Tom Fogal
    //  Creation: June 25, 2009
    //
    // ****************************************************************************
    void MISC_API parse_command_line(int argc, const char *argv[]);
}
#endif // VISIT_RUNTIME_SETTING_H
