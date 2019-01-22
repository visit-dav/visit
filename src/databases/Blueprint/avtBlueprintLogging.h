/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    CONDUIT_INFO( msg );                                            \
}                                                                   \

#define BP_PLUGIN_WARNING(  msg  )                                  \
{                                                                   \
    CONDUIT_INFO( "[blueprint warning] " << msg );                  \
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
