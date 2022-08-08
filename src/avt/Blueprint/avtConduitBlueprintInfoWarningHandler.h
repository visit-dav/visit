// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtConduitBlueprintInfoWarningHandler.h
// ************************************************************************* //

#include <string>

class avtConduitBlueprintInfoWarningHandler
{
public:
//-----------------------------------------------------------------------------
// These methods are used to re-wire conduit's default error handling
//-----------------------------------------------------------------------------
    static void avt_conduit_blueprint_print_msg(const std::string &msg,
                                                const std::string &file,
                                                int line);
    static void avt_conduit_blueprint_info_handler(const std::string &msg,
                                                   const std::string &file,
                                                   int line);
    static void avt_conduit_blueprint_warning_handler(const std::string &msg,
                                                      const std::string &file,
                                                      int line);
    static void avt_conduit_blueprint_error_handler(const std::string &msg,
                                                    const std::string &file,
                                                    int line);  
};
