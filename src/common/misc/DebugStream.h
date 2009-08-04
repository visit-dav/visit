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
//                                DebugStream.h                              //
// ************************************************************************* //

#ifndef DEBUG_STREAM_H
#define DEBUG_STREAM_H

#include <misc_exports.h>
#include <visitstream.h>

//
// Hide as much of DebugStream interface as possible.
//
namespace DebugStream
{
    // Query if a given level is enabled
    extern MISC_API bool Level1();
    extern MISC_API bool Level2();
    extern MISC_API bool Level3();
    extern MISC_API bool Level4();
    extern MISC_API bool Level5();

    // Obtain a given level's stream object
    extern MISC_API ostream& Stream1();
    extern MISC_API ostream& Stream2();
    extern MISC_API ostream& Stream3();
    extern MISC_API ostream& Stream4();
    extern MISC_API ostream& Stream5();

    // Query what the current level is (more expensive than LevelN())
    extern MISC_API int GetLevel();
}

#define debug1 if (!DebugStream::Level1()) ; else (DebugStream::Stream1())
#define debug2 if (!DebugStream::Level2()) ; else (DebugStream::Stream2())
#define debug3 if (!DebugStream::Level3()) ; else (DebugStream::Stream3())
#define debug4 if (!DebugStream::Level4()) ; else (DebugStream::Stream4())
#define debug5 if (!DebugStream::Level5()) ; else (DebugStream::Stream5())

#endif
