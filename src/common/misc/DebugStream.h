/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// Expose the ONLY parts of a debug stream object we really need in MOST
// of the source code.

#ifndef DEBUG_STREAM_H
#define DEBUG_STREAM_H
#include <misc_exports.h>
#include <visitstream.h>

#define debug1 if (!(*debug1_realp)) ; else (*debug1_realp)
#define debug2 if (!(*debug2_realp)) ; else (*debug2_realp)
#define debug3 if (!(*debug3_realp)) ; else (*debug3_realp)
#define debug4 if (!(*debug4_realp)) ; else (*debug4_realp)
#define debug5 if (!(*debug5_realp)) ; else (*debug5_realp)

// These are defined to maintain backward compatibility with the
// 'debugN_real' symbols before they were turned into pointers
// and which are used variously throughout VisIt.
#define debug1_real (*debug1_realp)
#define debug2_real (*debug2_realp)
#define debug3_real (*debug3_realp)
#define debug4_real (*debug4_realp)
#define debug5_real (*debug5_realp)

extern MISC_API ostream *debug1_realp;
extern MISC_API ostream *debug2_realp;
extern MISC_API ostream *debug3_realp;
extern MISC_API ostream *debug4_realp;
extern MISC_API ostream *debug5_realp;

#endif


