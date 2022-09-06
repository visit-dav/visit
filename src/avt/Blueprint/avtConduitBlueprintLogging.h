// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  avtConduitBlueprintLogging.h
// ************************************************************************* //

#ifndef AVT_CONDUIT_BLUEPRINT_LOGGING_H
#define AVT_CONDUIT_BLUEPRINT_LOGGING_H

#include "conduit.hpp"
#include <string>
#include "InvalidVariableException.h"

//-----------------------------------------------------------------------------
// visit includes
//-----------------------------------------------------------------------------
#include "DebugStream.h"

//-----------------------------------------------------------------------------
/// Macros for info messages, warnings and and errors
//-----------------------------------------------------------------------------

#define AVT_CONDUIT_BP_INFO(  msg  )                                \
{                                                                   \
    debug5 << msg;                                                  \
}                                                                   \

#define AVT_CONDUIT_BP_WARNING(  msg  )                             \
{                                                                   \
    debug5 << "[blueprint warning] " << msg;                        \
}                                                                   \

#define AVT_CONDUIT_BP_EXCEPTION1(  etype , msg )                   \
{                                                                   \
    std::ostringstream err_oss;                                     \
    err_oss << msg << std::endl;                                    \
    debug1 << "[avtConduitBlueprint error] " << err_oss.str();      \
    EXCEPTION1( etype  , err_oss.str() );                           \
}                                                                   \

#endif
