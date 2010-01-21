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

#include <DebugStreamFull.h>
#include <DebugStream.h>
#include <visitstream.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <visit-config.h>
using std::vector;

// static DebugStreamBuf class data
vector<DebugStreamFull::DebugStreamBuf*> DebugStreamFull::DebugStreamBuf::allBuffers;
int DebugStreamFull::DebugStreamBuf::curLevel;

// global DebugStreams
// We make these static so they are NOT visible outside this file
static DebugStreamFull debug1_realobj(1);
static DebugStreamFull debug2_realobj(2);
static DebugStreamFull debug3_realobj(3);
static DebugStreamFull debug4_realobj(4);
static DebugStreamFull debug5_realobj(5);

bool DebugStream::Level1() { return debug1_realobj.isenabled(); };
bool DebugStream::Level2() { return debug2_realobj.isenabled(); };
bool DebugStream::Level3() { return debug3_realobj.isenabled(); };
bool DebugStream::Level4() { return debug4_realobj.isenabled(); };
bool DebugStream::Level5() { return debug5_realobj.isenabled(); };

ostream& DebugStream::Stream1() { return *((ostream*) &debug1_realobj); };
ostream& DebugStream::Stream2() { return *((ostream*) &debug2_realobj); };
ostream& DebugStream::Stream3() { return *((ostream*) &debug3_realobj); };
ostream& DebugStream::Stream4() { return *((ostream*) &debug4_realobj); };
ostream& DebugStream::Stream5() { return *((ostream*) &debug5_realobj); };

int
DebugStream::GetLevel()
{
    int level = 0;
    level += debug1_realobj.isenabled();
    level += debug2_realobj.isenabled();
    level += debug3_realobj.isenabled();
    level += debug4_realobj.isenabled();
    level += debug5_realobj.isenabled();
    return level;
}

// ****************************************************************************
// Function: close_streams
//
// Purpose: 
//   Closes the debug streams.
//
// Note:       Taken from code in the signal handlers.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 28 10:19:51 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

static void
close_streams()
{
    if (debug1_realobj.isenabled()) debug1_realobj.close();
    if (debug2_realobj.isenabled()) debug2_realobj.close();
    if (debug3_realobj.isenabled()) debug3_realobj.close();
    if (debug4_realobj.isenabled()) debug4_realobj.close();
    if (debug5_realobj.isenabled()) debug5_realobj.close();
}

// ****************************************************************************
//  Function:  signalhandler_core
//
//  Purpose:
//    signal handler for signals which cause a core file to be created
//    flush and close the error logs
//
//  Arguments:
//    sig         the signal caught
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Eric Brugger, Thu Feb 22 12:00:06 PST 2001
//    I removed a reference to SIGEMT since it isn't portable.
//
//    Jeremy Meredith, Thu May 10 13:03:27 PDT 2001
//    Prevent infinite loops from errors on close.
//
//    Jeremy Meredith, Fri Dec 14 12:28:50 PST 2001
//    Check to make sure a debugstream is open before closing it.
//
//    Brad Whitlock, Thu Mar 14 11:54:34 PDT 2002.
//    Added conditional compilation for certain signals that are not
//    defined on Windows.
//
//    Brad Whitlock, Mon Apr 28 10:20:26 PDT 2003
//    I made it print the name of the signal to the debug log so we have
//    an idea of whether or not VisIt quit due to handling a signal.
//
// ****************************************************************************

static void
signalhandler_core(int sig)
{
    // don't catch the abort!
    signal(SIGABRT,SIG_DFL);

    // also prevent infinite loops
    signal(sig,SIG_DFL);

    switch (sig)
    {
      case SIGILL:
          debug1 << "signalhandler_core: SIG!" << endl;
          close_streams(); abort();
          break;
      case SIGABRT:
          debug1 << "signalhandler_core: SIGBRT!" << endl;
          close_streams(); abort();
          break;
      case SIGFPE:
          debug1 << "signalhandler_core: SIGFPE!" << endl;
          close_streams(); abort();
          break;
      case SIGSEGV:
          debug1 << "signalhandler_core: SIGSEGV!" << endl;
          close_streams(); abort();
          break;
#if !defined(_WIN32)
      case SIGBUS:
          debug1 << "signalhandler_core: SIGBUS!" << endl;
          close_streams(); abort();
          break;
      case SIGQUIT:
          debug1 << "signalhandler_core: SIGQUIT!" << endl;
          close_streams(); abort();
          break;
      case SIGTRAP:
          debug1 << "signalhandler_core: SIGTRAP!" << endl;
          close_streams(); abort();
          break;
      case SIGSYS:
          debug1 << "signalhandler_core: SIGSYS!" << endl;
          close_streams(); abort();
          break;
#endif
    }
}

// ****************************************************************************
//  Function:  signalhandler_exit
//
//  Purpose:
//    signal handler for signals which cause the program to exit
//    flush and close the error logs
//
//  Arguments:
//    sig         the signal caught
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 14 12:28:50 PST 2001
//    Check to make sure a debugstream is open before closing it.
//
//    Brad Whitlock, Thu Mar 14 11:58:34 PDT 2002
//    Added conditional compilation for Windows.
//
//    Brad Whitlock, Mon Apr 28 10:19:09 PDT 2003
//    I added code to print the name of the signal that's causing VisIt
//    to exit.
//
// ****************************************************************************

static void
signalhandler_exit(int sig)
{
    switch (sig)
    {
      case SIGTERM:
          debug1 << "signalhandler_exit: SIGTERM!" << endl;
          close_streams(); exit(-1);
          break;
      case SIGINT:
          debug1 << "signalhandler_exit: SIGINT!" << endl;
          close_streams(); exit(-1);
          break;
    }
}


// ****************************************************************************
// ****************************************************************************
//                         class DebugStreamBuf
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
//  Constructor:  DebugStreamBuf::DebugStreamBuf
//
//  Purpose:
//    constructor for DebugStreamBuf
//
//  Arguments:
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
// ****************************************************************************
DebugStreamFull::DebugStreamBuf::DebugStreamBuf() : streambuf()
{
    allBuffers.push_back(this);
    level = 0;
    out = NULL;
}


// ****************************************************************************
//  Destructor:  DebugStreamBuf::~DebugStreamBuf
//
//  Purpose:
//    destructor for DebugStreamBuf
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
// ****************************************************************************
DebugStreamFull::DebugStreamBuf::~DebugStreamBuf()
{
    close();
}


// ****************************************************************************
//  Method:  DebugStreamBuf::SetLevel
//
//  Purpose:
//    set the debug level of this streambuf
//
//  Arguments:
//    level_     the new level
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
// ****************************************************************************
void
DebugStreamFull::DebugStreamBuf::SetLevel(int level_)
{
    level=level_;
}


// ****************************************************************************
//  Method:  DebugStreamBuf::close
//
//  Purpose:
//    safely close the file and delete it if necessary
//
//  Arguments:
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
// ****************************************************************************
void
DebugStreamFull::DebugStreamBuf::close()
{
    if (out)
    {
        out->close();
        delete out;
        out = NULL;
    }
}


// ****************************************************************************
//  Method:  DebugStreamBuf::open
//
//  Purpose:
//    open a file and set its buffering
//
//  Arguments:
//    filename_    the filename of the file to create
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Jun  1 17:25:27 PDT 2001
//    Made *all* DebugStreams unit buffered.
//
//    Jeremy Meredith, Fri Dec 14 12:27:18 PST 2001
//    Unit-buffering them was not working on SunOS.  I added a call
//    to force unit-buffering on the streambuf, maybe even unbuffered.
//
//    Jeremy Meredith, Mon Apr 15 12:19:37 PDT 2002
//    Apparently setbuf is not public on some platforms.  I added an #ifdef
//    to disable it based on a configure'd define.
//
//    Mark C. Miller, Tue Apr 14 16:01:49 PDT 2009
//    Added option to buffer the debug logs.
// ****************************************************************************
void
DebugStreamFull::DebugStreamBuf::open(const char *filename_, bool buffer_debug)
{
    close();
    strcpy(filename, filename_);
    out = new ofstream;
    out->open(filename, ios::out);
    if (! *out)
    {
        delete out;
        out = NULL;
    }
    else
    {
        // flush the buffer after every operation
        if (!buffer_debug)
            out->setf(ios::unitbuf);
#ifndef NO_SETBUF
        // the previous flag does nothing on SunOS;
        // I hate to do this, but I'm doing it to force automatic flushing:
        if (!buffer_debug)
            out->rdbuf()->setbuf((char*)0,0);
#endif
    }
}


// ****************************************************************************
//  Method:  DebugStreamBuf::put
//
//  Purpose:
//    write a character to the file if the current level is at least at the
//    priority of this streambuf's level
//
//  Arguments:
//    c         the character
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Mar 14 12:08:34 PST 2002
//    Added a cast.
//
// ****************************************************************************
int
DebugStreamFull::DebugStreamBuf::put(int c)
{
    if (out &&
        curLevel <= level)
    {
        if (c!=EOF)
        {
            out->put((char)c);
        }
    }
    return c;
}


// ****************************************************************************
//  Method:  DebugStreamBuf::overflow
//
//  Purpose:
//    this is the routine called by the owning ostream for unbuffered output...
//    we never set up buffered output since we can buffer (if we want to)
//    through the ofstream we are really using to write
//
//  Arguments:
//    c         the character to write
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
// ****************************************************************************
int
DebugStreamFull::DebugStreamBuf::overflow(int c)
{
    curLevel = level;
    for (size_t i=0; i<allBuffers.size(); i++)
        allBuffers[i]->put(c);
    return c;
}



// ****************************************************************************
// ****************************************************************************
//                           class DebugStreamFull
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
//  Constructor:  DebugStreamFull::DebugStreamFull
//
//  Purpose:
//    constructor for the DebugStreamFull
//
//  Arguments:
//    level_    the debug level of this ostream
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Jun  1 17:18:46 PDT 2001
//    Added initialization of 'enabled'.
//
//    Eric Brugger, Tue Aug  3 11:03:38 PDT 2004
//    Change the DebugStreamBuf member to be a pointered value instead of a
//    referenced value so that it works with the MIPSpro compiler.
//
// ****************************************************************************
DebugStreamFull::DebugStreamFull(int level_) : ostream(new DebugStreamBuf)
{
    level = level_;
    buf = (DebugStreamBuf*)(rdbuf());
    buf->SetLevel(level);
    enabled = false;
}


// ****************************************************************************
//  Destructor:  DebugStreamFull::~DebugStreamFull
//
//  Purpose:
//    destructor for the DebugStreamFull
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Eric Brugger, Tue Aug  3 11:03:38 PDT 2004
//    Change the DebugStreamBuf member to be a pointered value instead of a
//    referenced value so that it works with the MIPSpro compiler.
//
// ****************************************************************************
DebugStreamFull::~DebugStreamFull()
{
    buf->close();
}


// ****************************************************************************
//  Method:  DebugStreamFull::open
//
//  Purpose:
//    create a file for logging given the program name
//
//  Arguments:
//    progname   the name of the program executing
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Jun  1 17:25:01 PDT 2001
//    Added setting of 'enabled' to true.
//
//    Eric Brugger, Tue Aug  3 11:03:38 PDT 2004
//    Change the DebugStreamBuf member to be a pointered value instead of a
//    referenced value so that it works with the MIPSpro compiler.
//
//    Mark C. Miller, Thu Apr  3 13:39:55 PDT 2008
//    Change the extension from '.log' to '.vlog'. Made it loop to find
//    a unique filename.
//
//    Mark C. Miller, Thu Apr 10 08:07:52 PDT 2008
//    Changed order of debug level and run number in filename.
//    Changed strspn to scan progname, not filename (which would already
//    have a 'level' sprintf'd into it.
//
//    Mark C. Miller, Thu Mar 12 18:20:34 PDT 2009
//    Changed the case of existing, older vlogs are handled. Before, it would
//    create a unique name by incrimenting a counter. Now, it will maintain
//    the 5 most recent logs named A.progname.level.vlog, B.progname.level.vlog
//    etc. with A always being the most recent and E the least recent. 
//    This has some undesirable behavior if you happening to be editing a vlog
//    and re-run VisIt as it may cause the editor to save a swap file because
//    the file's name is getting changed out from underneath the editor.
//
//    Mark C. Miller, Tue Apr 14 16:01:49 PDT 2009
//    Added option to buffer the debug logs.
//
//    Kathleen Bonnell, Thu Jan 21 07:44:15 MST 2009 
//    Changed naming on windows to NOT prepend a letter, due to fact that
//    progname generally contains fully-qualified-filename, and the fact that
//    we always use pids on windows, so renaming not necessary.
//
// ****************************************************************************


void
DebugStreamFull::open(const char *progname, bool clobber, bool buffer_debug)
{
    char filename[256];

#ifdef WIN32
    // On windows, we always use pids, so won't need to rename, and thus
    // don't need to prepend a letter.
    sprintf(filename, "%s.%d.vlog", progname, level);

#else
    sprintf(filename, "A.%s.%d.vlog", progname, level);

    // only rename old vlogs if we don't have pids
    bool renameOld = !clobber && (strspn(progname, ".0123456789") == 0);

    // Move all older filenames by one letter
    if (renameOld)
    {
        char filenametmp1[256];
        char filenametmp2[256];
        sprintf(filenametmp1, "E.%s.%d.vlog", progname, level);
        unlink(filenametmp1);               // E->deleted 
        sprintf(filenametmp2, "D.%s.%d.vlog", progname, level);
        rename(filenametmp2, filenametmp1); // D->E
        sprintf(filenametmp1, "C.%s.%d.vlog", progname, level);
        rename(filenametmp1, filenametmp2); // C->D
        sprintf(filenametmp2, "B.%s.%d.vlog", progname, level);
        rename(filenametmp2, filenametmp1); // B->C
        sprintf(filenametmp1, "A.%s.%d.vlog", progname, level);
        rename(filenametmp1, filenametmp2); // A->B
    }
#endif

    // ok, open the stream
    buf->open(filename, buffer_debug);
    enabled = true;
}


// ****************************************************************************
//  Method:  DebugStreamFull::close
//
//  Purpose:
//    close the file, but don't delete it
//
//  Arguments:
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Dec 14 12:26:46 PST 2001
//    Added setting enabled to false so we won't try to close it
//    more than once.
//
//    Eric Brugger, Tue Aug  3 11:03:38 PDT 2004
//    Change the DebugStreamBuf member to be a pointered value instead of a
//    referenced value so that it works with the MIPSpro compiler.
//
// ****************************************************************************
void
DebugStreamFull::close()
{
    buf->close();
    enabled = false;
}

// ****************************************************************************
//  Method:  DebugStreamFull::Initialize
//
//  Purpose:
//    This should be called once to initalize all the debug streams.
//    Also registers signal handlers to flush the output of all the log files
//    on abnormal program termination.
//
//  Arguments:
//    progname     the name of the running program
//    debuglevel   1-5: the lowest priority of messages you want to log
//                 0: same as 1, but hide the log and delete it on normal exit
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
//  Modifications:
//    Eric Brugger, Thu Feb 22 12:00:06 PST 2001
//    I removed a reference to SIGEMT since it isn't portable.
//
//    Hank Childs, Fri Mar  2 14:53:17 PST 2001
//    Fixed bug with prefixing log name with a '.'.
//
//    Brad Whitlock, Thu Mar 14 11:59:34 PDT 2002
//    Added conditional compilation for Windows.
//
//    Jeremy Meredith, Wed Aug  7 13:17:22 PDT 2002
//    Made it not puke on interesting debug levels.
//
//    Hank Childs, Wed Nov 13 08:28:41 PST 2002
//    Ignore FPE, since they come up on gps (and MeshTV ignores them too).
//
//    Brad Whitlock, Mon Apr 28 10:18:01 PDT 2003
//    Ignore SIGPIPE because it can defeat our mechanisms for detecting when
//    we lose a connection to a component.
//
//    Jeremy Meredith, Tue May 17 11:20:51 PDT 2005
//    Allow disabling of signal handlers.
//
//    Mark C. Miller, Tue Apr 14 16:01:49 PDT 2009
//    Added option to buffer the debug logs.
// ****************************************************************************

void
DebugStreamFull::Initialize(const char *progname, int debuglevel, bool sigs,
    bool clobber, bool buffer_debug)
{
    switch (debuglevel)
    {
      case 5:  debug5_realobj.open(progname, clobber, buffer_debug);
        /* FALLTHRU */
      case 4:  debug4_realobj.open(progname, clobber, buffer_debug);
        /* FALLTHRU */
      case 3:  debug3_realobj.open(progname, clobber, buffer_debug);
        /* FALLTHRU */
      case 2:  debug2_realobj.open(progname, clobber, buffer_debug);
        /* FALLTHRU */
      case 1:  debug1_realobj.open(progname, clobber, buffer_debug);
        break;
      default:
        break;
    }

    if (sigs)
    {
#if !defined(_WIN32)
        signal(SIGQUIT,  signalhandler_core);
        signal(SIGTRAP,  signalhandler_core);
        signal(SIGSYS,   signalhandler_core);
        signal(SIGBUS,   signalhandler_core);
        signal(SIGPIPE,  SIG_IGN);
#endif

        signal(SIGILL,   signalhandler_core);
        signal(SIGABRT,  signalhandler_core);
        signal(SIGFPE,   SIG_IGN);
        signal(SIGSEGV,  signalhandler_core);
        signal(SIGTERM,  signalhandler_exit);
        signal(SIGINT,   signalhandler_exit);
    }
}
