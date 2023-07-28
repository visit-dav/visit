// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtConduitBlueprintInfoWarningHandler.C
// ************************************************************************* //

#include "avtConduitBlueprintInfoWarningHandler.h"
#include "avtConduitBlueprintLogging.h"

//-----------------------------------------------------------------------------
// These methods are used to re-wire conduit's default error handling
//-----------------------------------------------------------------------------
void
avtConduitBlueprintInfoWarningHandler::avt_conduit_blueprint_print_msg(
	const std::string &msg,
	const std::string &file,
	int line)
{
    // Uncomment for very verbose traces:
    //
    // debug5 << "File:"    << file << std::endl
    //        << "Line:"    << line << std::endl
    //        << "Message:" << msg  << std::endl;
    debug5 << msg  << std::endl;
}

//-----------------------------------------------------------------------------
void
avtConduitBlueprintInfoWarningHandler::avt_conduit_blueprint_info_handler(
	const std::string &msg,
	const std::string &file,
	int line)
{
    avt_conduit_blueprint_print_msg(msg,file,line);
}


//-----------------------------------------------------------------------------
void
avtConduitBlueprintInfoWarningHandler::avt_conduit_blueprint_warning_handler(
	const std::string &msg,
	const std::string &file,
	int line)
{
    avt_conduit_blueprint_print_msg(msg,file,line);
}

//-----------------------------------------------------------------------------
void
avtConduitBlueprintInfoWarningHandler::avt_conduit_blueprint_error_handler(
	const std::string &msg,
	const std::string &file,
	int line)
{
    std::ostringstream bp_err_oss;
    bp_err_oss << "[ERROR]"
               << "File:"    << file << std::endl
               << "Line:"    << line << std::endl
               << "Message:" << msg  << std::endl;

    debug1 << bp_err_oss.str();

    AVT_CONDUIT_BP_EXCEPTION1(InvalidVariableException, bp_err_oss.str());

}

