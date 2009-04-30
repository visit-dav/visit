/*


                          Copyright 2000 - 2004.
               The Regents of the University of California.
                           All rights reserved.

This work was  produced at the  University of California,  Lawrence  Livermore
National Laboratory  (UC LLNL) under contract  no. W-7405-ENG-48 (Contract 48)
between the U.S. Department of Energy (DOE)  and The Regents of the University
of California (University) for the operation of UC LLNL. Copyright is reserved
to the University for purposes of  controlled dissemination, commercialization
through formal licensing, or other disposition under terms of Contract 48; DOE
policies, regulations and orders; and U.S. statutes. The rights of the Federal
Government are reserved  under Contract 48 subject to  the restrictions agreed
upon by the DOE and University as allowed under DOE Acquisition Letter 97-1.


                                DISCLAIMER

This  work was  prepared as an  account of work sponsored  by an agency of the
United  States  Government.  Neither  the  United  States  Government  nor the
University  of California  nor any of  their employees,  makes  any  warranty,
express  or implied,  or  assumes  any liability  or  responsibility  for  the
accuracy,  completeness, or usefulness of any information, apparatus, product,
or process disclosed, or represents that its use would not infringe privately-
owned rights.  Reference herein  to any specific commercial products, process,
or  service by  trade  name, trademark,  manufacturer  or  otherwise  does not
necessarily constitute or  imply its endorsement, recommendation,  or favoring
by the United States Government or the University of California. The views and
opinions of authors expressed herein do not necessarily state or reflect those
of the United States Government or the University of California, and shall not
be used for advertising or product endorsement purposes.


                          LICENSING REQUIREMENTS

Permission is hereby  granted to use and  copy this software and documentation
for internal noncommercial purposes only, provided that 1) the above copyright
notice and disclaimer appear in all  copies of the software and documentation,
and 2) all UC LLNL identification in the user interface remains unchanged. Any
use,  reproduction,   modification,  or  distribution   of  this  software  or
documentation for commercial  purposes requires a license from the University.
Contact:  Lawrence Livermore National Laboratory,  Industrial Partnerships and
Commercialization Office, P.O. Box 808, L-795, Livermore, CA 94551.
*/

#include "RCDebugStream.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include <visit-config.h>
using std::vector;
using namespace std;

namespace rclib {
// static DebugStreamBuf class data
vector<DebugStream::DebugStreamBuf*> DebugStream::DebugStreamBuf::allBuffers;
int DebugStream::DebugStreamBuf::curLevel;

static int gDebugLevel=0; 
int get_debug_level(void) {
  return gDebugLevel; 
}

// global DebugStreams
DebugStream debug1_real(1);
DebugStream debug2_real(2);
DebugStream debug3_real(3);
DebugStream debug4_real(4);
DebugStream debug5_real(5);

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
    if (debug1_real) debug1_real.close();
    if (debug2_real) debug2_real.close();
    if (debug3_real) debug3_real.close();
    if (debug4_real) debug4_real.close();
    if (debug5_real) debug5_real.close();
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
          rcdebug1 << "signalhandler_core: SIG!" << endl;
          close_streams(); abort();
          break;
      case SIGABRT:
          rcdebug1 << "signalhandler_core: SIGBRT!" << endl;
          close_streams(); abort();
          break;
      case SIGFPE:
          rcdebug1 << "signalhandler_core: SIGFPE!" << endl;
          close_streams(); abort();
          break;
      case SIGSEGV:
          rcdebug1 << "signalhandler_core: SIGSEGV!" << endl;
          close_streams(); abort();
          break;
#if !defined(_WIN32)
      case SIGBUS:
          rcdebug1 << "signalhandler_core: SIGBUS!" << endl;
          close_streams(); abort();
          break;
      case SIGQUIT:
          rcdebug1 << "signalhandler_core: SIGQUIT!" << endl;
          close_streams(); abort();
          break;
      case SIGTRAP:
          rcdebug1 << "signalhandler_core: SIGTRAP!" << endl;
          close_streams(); abort();
          break;
      case SIGSYS:
          rcdebug1 << "signalhandler_core: SIGSYS!" << endl;
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
          rcdebug1 << "signalhandler_exit: SIGTERM!" << endl;
          close_streams(); exit(-1);
          break;
      case SIGINT:
          rcdebug1 << "signalhandler_exit: SIGINT!" << endl;
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
DebugStream::DebugStreamBuf::DebugStreamBuf() : streambuf()
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
DebugStream::DebugStreamBuf::~DebugStreamBuf()
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
DebugStream::DebugStreamBuf::SetLevel(int level_)
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
DebugStream::DebugStreamBuf::close()
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
// ****************************************************************************
void
DebugStream::DebugStreamBuf::open(const char *filename_)
{
    close();
    strcpy(filename, filename_);
    out = new ofstream;
#ifdef DEBUG_TO_STDERR 
    out->open("/dev/null"); 
#else
    out->open(filename, ios::out);
#endif
    if (! *out)
    {
        delete out;
        out = NULL;
    }
    else
    {
        // flush the buffer after every operation
        out->setf(ios::unitbuf);
    /*
#ifndef NO_SETBUF
        // the previous flag does nothing on SunOS;
        // I hate to do this, but I'm doing it to force automatic flushing:
        out->rdbuf()->setbuf((char*)0,0);
#endif
    */
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
DebugStream::DebugStreamBuf::put(int c)
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
DebugStream::DebugStreamBuf::overflow(int c)
{
    curLevel = level;
    for (uint i=0; i<allBuffers.size(); i++)
        allBuffers[i]->put(c);
    return c;
}



// ****************************************************************************
// ****************************************************************************
//                           class DebugStream
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
//  Constructor:  DebugStream::DebugStream
//
//  Purpose:
//    constructor for the DebugStream
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
// ****************************************************************************
DebugStream::DebugStream(int level_) : ostream(&buf)
{
    level = level_;
    buf.SetLevel(level);
    enabled = false;
}


// ****************************************************************************
//  Destructor:  DebugStream::~DebugStream
//
//  Purpose:
//    destructor for the DebugStream
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 17, 2000
//
// ****************************************************************************
DebugStream::~DebugStream()
{
    buf.close();
}


// ****************************************************************************
//  Method:  DebugStream::open
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
// ****************************************************************************
void
DebugStream::open(const char *progname)
{
    char filename[256];
    sprintf(filename, "%s.%d.log", progname, level);
    buf.open(filename);
    enabled = true;
}


// ****************************************************************************
//  Method:  DebugStream::close
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
// ****************************************************************************
void
DebugStream::close()
{
    buf.close();
    enabled = false;
}

// ****************************************************************************
//  Method:  DebugStream::Initialize
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
// ****************************************************************************

void
DebugStream::Initialize(const char *progname, int debuglevel)
{
    switch (debuglevel)
    {
      case 5:  debug5_real.open(progname);
        /* FALLTHRU */
      case 4:  debug4_real.open(progname);
        /* FALLTHRU */
      case 3:  debug3_real.open(progname);
        /* FALLTHRU */
      case 2:  debug2_real.open(progname);
        /* FALLTHRU */
      case 1:  debug1_real.open(progname);
        break;
      default:
        break;
    }
    gDebugLevel = debuglevel; 

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

} // end namespace rclib
