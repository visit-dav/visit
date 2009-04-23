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
//                              DebugStreamFull.h                            //
// ************************************************************************* //

#ifndef DEBUG_STREAM_FULL_H
#define DEBUG_STREAM_FULL_H
#include <misc_exports.h>
#include <visitstream.h>

#include <vector>
#include <signal.h>
#include <stdlib.h>

// ****************************************************************************
//  Class:  DebugStreamFull
//
//  Purpose:
//    An implementation of ostream designed for error and message logging.
//    A DebugStream has a "level" assigned to it as a priority (lower number
//    is higher priority).  Multiple DebugStreams can be created.  When
//    output is written to one DebugStream, it is simultaneously written to
//    all DebugStreams with a higher level.
//
//    In this specific implementation, five debug levels are used.  Only
//    the highest priority messages go to debug1, but every message will go
//    to debug5.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 16, 2000
//
//  Modified:
//    Jeremy Meredith, Fri Jun  1 17:18:03 PDT 2001
//    Added the ability to check if a DebugStream was enabled.
//
//    Brad Whitlock, Thu Mar 14 13:37:34 PST 2002
//    Added exports.
//
//    Jeremy Meredith, Thu Jan 16 11:40:29 PST 2003
//    Made debug streams macros so that we could skip the logic to 
//    do the formatting if the debug output was disabled.  Removed the
//    ability to delete a debug stream automatically on close.
//
//    Eric Brugger, Tue Aug  3 11:00:56 PDT 2004
//    Change the DebugStreamBuf member to be a pointered value instead of
//    a referenced value so that it works with the MIPSpro compiler.
//
//    Jeremy Meredith, Tue May 17 11:20:51 PDT 2005
//    Allow disabling of signal handlers.
//
//    Mark C. Miller, Wed Apr 22 13:32:22 PDT 2009
//    Changed name to DebugStreamFull
// ****************************************************************************

class MISC_API DebugStreamFull : public ostream
{
  public:
                   DebugStreamFull(int level_);
                  ~DebugStreamFull();

    bool           isenabled() const { return enabled; };
    void           open(const char *progname, bool, bool);
    void           close();
    static void    Initialize(const char *, int, bool=true, bool=false, bool=false);
  private:
    class DebugStreamBuf : public streambuf
    {
      public:
                       DebugStreamBuf();
                      ~DebugStreamBuf();

        void           SetLevel(int level_);

        void           close();
        void           open(const char *filename_, bool=false);
        int            put(int c);
        virtual int    overflow(int c);
        virtual int    underflow() { return 0; };
      private:
        // the priority of the current messages being logged
        static int                            curLevel;
        // the list of all buffers
        static std::vector<DebugStreamBuf*>   allBuffers;

        // the level of this DebugStreamBuf
        int                                   level;
        // the filename being to which to write the messages
        char                                  filename[256];
        // the ostream used to write the log file
        ofstream                             *out;
    };


    // the streambuf used for this ostream
    DebugStreamBuf  *buf;
    // the level of this DebugStream
    int              level;
    // true if enabled
    bool             enabled;
};

#endif
