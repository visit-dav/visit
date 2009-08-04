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
//                              VisitDebugOut.C                              //
// ************************************************************************* //
#include <cstdarg>
#include "VisItDebugOut.h"
#include <DebugStream.h>

static const size_t DBG_BUFFER_SIZE = 8192;

// ****************************************************************************
// Method: VisItDebugOut::printf
//
// Purpose:
//   It's a bit unclear where a call to printf should go; the other
//   DebugOut implementations use it unify the output of the actual
//   message.  However we need to do the output in each of the
//   individual methods, because the method itself tells us which
//   VisIt stream to output to.
//
// Arguments:
//   format : format string using printf syntax.
//   ...    : replacements for the format string.
//
// Programmer: Tom Fogal
// Creation:   Thu Mar  5 13:58:00 MST 2009
//
// Modifications:
//
//   Tom Fogal, Thu Mar  5 16:44:49 MST 2009
//   Give messages a clear label to disambiguate their source.
//
// ****************************************************************************
void
VisItDebugOut::printf(const char *format, ...) const
{
    // We'll just output to 5 and hope that makes sense..
    char buffer[DBG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
#ifdef WIN32
        _vsnprintf_s(buffer, DBG_BUFFER_SIZE, sizeof(buffer), format, args);
#else
        vsnprintf(buffer, sizeof(buffer), format, args);
#endif
    va_end(args);
    debug5 << "[TUVOK] " << buffer << std::endl;
}

// ****************************************************************************
// Method: VisItDebugOut::Message
//
// Purpose: Maps Tuvok's "Message" outputs to VisIt's debug5 stream.
//
// Arguments:
//   src    : source file location of this message (method name)
//   format : format string using printf syntax.
//   ...    : replacements for the format string.
//
// Programmer: Tom Fogal
// Creation:   Thu Mar  5 13:59:12 MST 2009
//
// Modifications:
//
//   Tom Fogal, Thu Mar  5 16:44:49 MST 2009
//   Give messages a clear label to disambiguate their source.
//
//   Tom Fogal, Sat Mar  7 22:02:18 MST 2009
//   Print the function name where this came from.
//
// ****************************************************************************
void
VisItDebugOut::Message(const char *src, const char *format, ...)
{
    char buffer[DBG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
#ifdef WIN32
        _vsnprintf_s(buffer, DBG_BUFFER_SIZE, sizeof(buffer), format, args);
#else
        vsnprintf(buffer, sizeof(buffer), format, args);
#endif
    va_end(args);
    debug5 << "[TUVOK] (" << src << ") " << buffer << std::endl;
}

// ****************************************************************************
// Method: VisItDebugOut::Warning
//
// Purpose: Maps Tuvok's "Warning" outputs to VisIt's debug2 stream.
//
// Arguments:
//   src    : source file location of this message (method name)
//   format : format string using printf syntax.
//   ...    : replacements for the format string.
//
// Programmer: Tom Fogal
// Creation:   Thu Mar  5 13:59:50 MST 2009
//
// Modifications:
//
//   Tom Fogal, Thu Mar  5 16:44:49 MST 2009
//   Give messages a clear label to disambiguate their source.
//
//   Tom Fogal, Sat Mar  7 22:02:18 MST 2009
//   Print the function name where this came from.
//
// ****************************************************************************
void
VisItDebugOut::Warning(const char* src, const char* format, ...)
{
    char buffer[DBG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
#ifdef WIN32
        _vsnprintf_s(buffer, DBG_BUFFER_SIZE, sizeof(buffer), format, args);
#else
        vsnprintf(buffer, sizeof(buffer), format, args);
#endif
    va_end(args);
    debug2 << "[TUVOK] (" << src << ") " << buffer << std::endl;
}

// ****************************************************************************
// Method: VisItDebugOut::Error
//
// Purpose: Maps Tuvok's "Error" outputs to VisIt's debug1 stream.
//
// Arguments:
//   src    : source file location of this message (method name)
//   format : format string using printf syntax.
//   ...    : replacements for the format string.
//
// Programmer: Tom Fogal
// Creation:   Thu Mar  5 14:00:20 MST 2009
//
// Modifications:
//
//   Tom Fogal, Thu Mar  5 16:44:49 MST 2009
//   Give messages a clear label to disambiguate their source.
//
//   Tom Fogal, Sat Mar  7 22:02:18 MST 2009
//   Print the function name where this came from.
//
// ****************************************************************************
void
VisItDebugOut::Error(const char* src, const char* format, ...)
{
    char buffer[DBG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
#ifdef WIN32
        _vsnprintf_s(buffer, DBG_BUFFER_SIZE, sizeof(buffer), format, args);
#else
        vsnprintf(buffer, sizeof(buffer), format, args);
#endif
    va_end(args);
    debug1 << "[TUVOK] (" << src << ") " << buffer << std::endl;
}
