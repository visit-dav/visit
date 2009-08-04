/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
};
#endif // VISIT_RUNTIME_SETTING_H
