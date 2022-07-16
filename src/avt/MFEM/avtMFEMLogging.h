// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  avtMFEMLogging.h
// ************************************************************************* //

#ifndef AVT_MFEM_LOGGING_H
#define AVT_MFEM_LOGGING_H

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

#define BP_PLUGIN_INFO(  msg  )                                     \
{                                                                   \
    CONDUIT_INFO( msg );                                            \
}                                                                   \

#define BP_PLUGIN_EXCEPTION1(  etype , msg )                        \
{                                                                   \
    std::ostringstream err_oss;                                  \
    err_oss << msg << std::endl;                                 \
    debug1 << "[avtMFEM error] " << err_oss.str();      \
    EXCEPTION1( etype  , err_oss.str() );                        \
}                                                                   \

#endif
