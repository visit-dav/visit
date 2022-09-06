// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  avtBlueprintLogging.h
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_LOGGING_H
#define AVT_BLUEPRINT_LOGGING_H

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
    debug5 << msg;                                                  \
}                                                                   \

#define BP_PLUGIN_WARNING(  msg  )                                  \
{                                                                   \
    debug5 << "[blueprint warning] " << msg;                        \
}                                                                   \

#define BP_PLUGIN_EXCEPTION1(  etype , msg )                        \
{                                                                   \
    std::ostringstream bp_err_oss;                                  \
    bp_err_oss << msg << std::endl;                                 \
    debug1 << "[blueprint plugin error] " << bp_err_oss.str();      \
    EXCEPTION1( etype  , bp_err_oss.str() );                        \
}                                                                   \


//-----------------------------------------------------------------------------
/// The CHECK_HDF5_ERROR macro is used to check error codes from HDF5.
//-----------------------------------------------------------------------------
#define CHECK_HDF5_ERROR( hdf5_err, msg    )                        \
{                                                                   \
    if( hdf5_err < 0 )                                              \
    {                                                               \
        std::ostringstream hdf5_err_oss;                            \
        hdf5_err_oss << "HDF5 Error code"                           \
            <<  hdf5_err                                            \
            << " " << msg;                                          \
        BP_PLUGIN_EXCEPTION1( InvalidVariableException,             \
                              hdf5_err_oss.str());                  \
    }                                                               \
}                                                                   \

#endif
