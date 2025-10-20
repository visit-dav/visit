// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  avtMFEMLogging.h
// ************************************************************************* //

#ifndef AVT_MFEM_LOGGING_H
#define AVT_MFEM_LOGGING_H

#include <string>
#include "InvalidVariableException.h"

//-----------------------------------------------------------------------------
// visit includes
//-----------------------------------------------------------------------------
#include "DebugStream.h"

//-----------------------------------------------------------------------------
/// Macros for info messages, warnings and and errors
//-----------------------------------------------------------------------------

#define AVT_MFEM_INFO(  msg  )                                      \
{                                                                   \
    debug5 << msg << std::endl;                                     \
}                                                                   \

#define AVT_MFEM_EXCEPTION1(  etype , msg )                         \
{                                                                   \
    std::ostringstream err_oss;                                     \
    err_oss << msg << std::endl;                                    \
    debug1 << "[avtMFEM error] " << err_oss.str();                  \
    EXCEPTION1( etype  , err_oss.str() );                           \
}                                                                   \

#endif
