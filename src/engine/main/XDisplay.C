/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <iterator>
#include <vector>

#include <X11/Xlib.h>

#include <XDisplay.h>

#include <DebugStream.h>
#include <InitVTKRendering.h>
#include <snprintf.h>
#include <StringHelpers.h>
#include <visit-config.h>

#ifdef DEBUG_X
#   define DEBUG_ONLY(block) block
#else
#   define DEBUG_ONLY(block) /* nothing */
#endif

static inline bool initialized(pid_t x) { return ((int)x != -1); }
static inline void set_uninitialized(pid_t *x) { *x = (pid_t) -1; }
static char **vec_convert(std::vector<std::string> svec, size_t *len);
void vec_convert_free(char **vec, size_t len);
static pid_t xinit(const std::string& display,
                   const std::vector<std::string>& user_args);
static void fix_signals();

// ****************************************************************************
//  Method: XDisplay constructor
//
//  Purpose:
//    Sets the default state of uninitialized X server for display 0.
//
//  Programmer:  Tom Fogal
//  Creation:    August 29, 2008
//
// ****************************************************************************

XDisplay::XDisplay(): xserver((pid_t)-1), launch(true) { }

// ****************************************************************************
//  Method: XDisplay destructor
//
//  Purpose:
//    Ensures the X server has been brought down.
//
//  Programmer:  Tom Fogal
//  Creation:    August 29, 2008
//
// ****************************************************************************

XDisplay::~XDisplay()
{
    if(initialized(this->xserver))
    {
        this->Teardown();
    }
}

// ****************************************************************************
//  Method: XDisplay::Initialize
//
//  Purpose:
//    Fork-exec an X server
//
//  Returns: Success / failure.
//
//  Arguments:
//    display    Display number to create.
//    user_args  User-given arguments to xinit.
//
//  Programmer:  Tom Fogal
//  Creation:    July 27, 2008
//
//  Modifications:
//
//    Tom Fogal, Mon May 24 18:58:57 MDT 2010
//    Abstract most of this method out to `xinit'.
//
//    Tom Fogal, Tue May 25 16:11:38 MDT 2010
//    Lookup hostname; we'll use it in error messages.
//
//    Tom Fogal, Wed May 26 09:20:18 MDT 2010
//    Do not launch X servers if the client does not want us to.
//
// ****************************************************************************

bool
XDisplay::Initialize(std::string display,
                     const std::vector<std::string>& user_args)
{
    this->display = display;

    if(gethostname(this->hostname, 512) != 0)
    {
        debug1 << "Error " << errno << " while getting hostname.\n";
        this->hostname[0] = 0;
    }
    if(this->launch && (this->xserver = xinit(this->display, user_args)) == -1)
    {
      return false;
    }
    debug2 << this->hostname << " saved X server PID " << this->xserver << "\n";
    return true;
}

// ****************************************************************************
//  Method: XDisplay::Connect
//
//  Purpose:
//    Associates this process with an X server.
//
//  Programmer:  Tom Fogal
//  Creation:    August 29, 2008 (from a July 27th method)
//
//  Modifications:
//
//    Tom Fogal, Tue Aug  5 16:36:20 EDT 2008
//    Dropped the array size down a notch; why was it so huge before?
//
//    Tom Fogal, Fri Aug 29 19:19:25 EDT 2008
//    Removed a variable that was only useful in debugging.
//
//    Tom Fogal, Tue May 25 16:07:27 MDT 2010
//    Made it return a bool so we can detect errors.
//
//    Tom Fogal, Wed May 26 09:05:00 MDT 2010
//    Detect errors.
//
//    Tom Fogal, Wed May 26 10:15:05 MDT 2010
//    Not initializing the X server shouldn't mean the connect fails.
//
//    Tom Fogal, Thu May  5 11:36:39 MDT 2011
//    Eventually the spinning should end.
//
// ****************************************************************************

bool
XDisplay::Connect()
{
    static char env_display[128];

    debug3 << "Connecting to display " << this->display << std::endl;
    SNPRINTF(env_display, 128, "DISPLAY=%s", this->display.c_str());

    if(putenv(env_display) != 0)
    {
        perror("putenv");
        debug1 << this->hostname << ": putenv(\"" << env_display
               << "\") failed.\n";
    }
    InitVTKRendering::UnforceMesa();

    std::string xh = std::string("xhost +") + this->hostname;
    system(xh.c_str());

    // Test our connection.
    Display* dpy=NULL;
    size_t spin_count = 0;
    do
    {
        int status=0;
        if(initialized(this->xserver))
        {
            // Our X server might have died.  Don't bother spinning
            // until we can connect if it's never going to start!
            switch(waitpid(this->xserver, &status, WNOHANG) == -1)
            {
            case -1:
                debug1 << this->hostname << ": waitpid(" << this->xserver
                       << ") failed.\n";
                return false;
                break;
            case 0:
                debug5 << "Good, X is still running.\n";
                break;
            default:
                if(WIFEXITED(status) || WIFSIGNALED(status))
                {
                    debug1 << this->hostname << ": X server exited before we "
                           << "could connect!  This normally means the X "
                              "server configuration is incorrect.\n";
                    return false;
                }
                break;
            }
        }
        dpy = XOpenDisplay(NULL);
        if(dpy == NULL)
        {
            debug1 << this->hostname << ": could not connect to display "
                   << XDisplayName(NULL) << "; spinning...\n";
            ++spin_count;
            if(++spin_count > 10) {
              return false;
            }
            sleep(1);
        }
    } while(dpy == NULL);
    XCloseDisplay(dpy);

    return true;
}

// ****************************************************************************
//  Method: XDisplay::Teardown
//
//  Purpose:
//    Disconnect from the display we used for HW rendering.  Stop the given
//    process.
//
//  Programmer:  Tom Fogal
//  Creation:    July 29, 2008
//
//  Modifications:
//
//    Eric Brugger, Tue Oct 21 16:58:25 PDT 2008
//    I made use of unsetenv dependent on HAVE_SETENV (only gcc 3.2 on
//    Solaris).  I Replaced strerror_r with strerror.
//
// ****************************************************************************

void
XDisplay::Teardown()
{
#ifdef HAVE_SETENV
    unsetenv("DISPLAY");
#endif

    debug3 << "Tearing down display " << this->xserver << std::endl;

    // First, lets try to be nice about it.
    if(kill(this->xserver, SIGINT) < 0)
    {
        perror("Killing X via SIGINT");
        sleep(2);
        // Hrumph.
        if(kill(this->xserver, SIGKILL) < 0)
        {
            debug1 << "Could not stop X server: "
                   << strerror(errno) << std::endl;
            std::cerr << "Could not stop the X server: "
                      << strerror(errno) << std::endl
                      << "You might have stale X server or engine_par "
                      << "processes around now." << std::endl;
        }
    }
    int status;
    waitpid(this->xserver, &status, WUNTRACED);
    if(WIFEXITED(status))
    {
        debug4 << "X server exited on it's own." << std::endl;
    }
    else if(WIFSIGNALED(status))
    {
        debug4 << "X server killed successfully." << std::endl;
    }
    else
    {
        debug4 << "*Unexpected* X server death status: " << status << std::endl;
    }

    set_uninitialized(&this->xserver);
}

// ****************************************************************************
//  Method: XDisplay::Launch
//
//  Purpose:
//    Lets the user tell us whether the X server should be launched by VisIt.
//
//  Programmer:  Tom Fogal
//  Creation:    May 26, 2010
//
//  Modifications:
//
// ****************************************************************************
void XDisplay::Launch(bool l) { this->launch = l; }

// ****************************************************************************
//  Function: vec_convert
//
//  Purpose: Converts a vector of strings into ... a vector of strings.  The
//           latter is really a NULL-terminated C array, suitable for passing
//           to an exec(2).
//           Note that all strings (and the array itself) are dynamically
//           allocated, and must be 'free'd by the caller.
//
//  Returns: Converted string vector.
//
//  Arguments:
//    svec       original vector
//    len        filled with the number of elements in the new vector.
//
//  Programmer: Tom Fogal
//  Creation:   August 15, 2008
//
// ****************************************************************************
static char **
vec_convert(std::vector<std::string> svec, size_t *len)
{
    char **argv;

    *len = svec.size();
    argv = (char**) malloc(sizeof(char*) * (*len+1));

    size_t i;
    for(i=0; i < *len; ++i)
    {
        argv[i] = strdup(svec[i].c_str());
    }
    argv[i] = NULL;
    return argv;
}

// ****************************************************************************
//  Function: vec_convert_free
//
//  Purpose:
//    Frees a vector of strings created by vec_convert.  Ensures the client
//    doesn't have to know whether to use delete or free.
//
//  Arguments:
//    vec        the vector to free
//    len        number of elements in the vector.
//
//  Programmer: Tom Fogal
//  Creation:   September 1, 2008
//
// ****************************************************************************
void
vec_convert_free(char **vec, size_t len)
{
    for(size_t i=0; i < len; ++i)
    {
        free(vec[i]);
    }
    free(vec);
}

// ****************************************************************************
//  Function: xinit
//
//  Purpose:
//    Starts up an X server.
//
//  Arguments:
//    display    the display to start the X server on.  Should start with ":".
//    user_args  any additional server arguments to pass.
//
//  Programmer: Tom Fogal
//  Creation:   May 24, 2010
//
// ****************************************************************************
static pid_t
xinit(const std::string& display, const std::vector<std::string>& user_args)
{
    char **argv;
    size_t v_elems;

    std::vector<std::string> args;
    args.push_back("xinit");
    args.push_back("sleep"); // sleep: make sure the server dies.
    args.push_back("28800");
    args.push_back("--");
    args.push_back(display.c_str());
    args.push_back("-ac");
    args.push_back("-sharevts");
    args.push_back("-once");
    args.push_back("-terminate");
    args.push_back("-nolisten");
    args.push_back("tcp");
    args.push_back("-allowMouseOpenFail");
    StringHelpers::append(args, user_args);

    debug5 << "X server command line arguments:\n\t";
    std::copy(args.begin(), args.end(),
              std::ostream_iterator<std::string>(DebugStream::Stream5(),
                                                 "\n\t"));
    debug5 << "\n";

    argv = vec_convert(args, &v_elems);

    // make sure we'll know when the child goes away.
    signal(SIGCHLD, SIG_DFL);

    pid_t xserver;
    if((xserver = fork()) == (pid_t) -1)
    {
        perror("fork");
        vec_convert_free(argv, v_elems);
        return -1;
    }

    if(xserver == 0)
    {
        fix_signals(); // get rid of VisIt signal handlers.
        execvp(argv[0], argv);
        perror("execvp of xinit");
        vec_convert_free(argv, v_elems);
        _exit(0); // ignore atexit() handlers.
    }
    vec_convert_free(argv, v_elems);

    return xserver;
}

// ****************************************************************************
//  Function: fix_signals
//
//  Purpose:
//    Fixes our signals for launching a child process.  VisIt tries to catch
//    many signals so that we can report errors, but those handlers are
//    inappropriate for our children.
//
//  Programmer: Tom Fogal
//  Creation:   May 25, 2010
//
// ****************************************************************************
static void fix_signals()
{
  alarm(0); // reset alarm.
  signal(SIGCHLD, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTRAP, SIG_DFL);
  signal(SIGSYS,  SIG_DFL);
  signal(SIGBUS,  SIG_DFL);
  signal(SIGPIPE, SIG_DFL);
  signal(SIGILL,  SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGFPE,  SIG_DFL);
  signal(SIGSEGV, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT,  SIG_DFL);
}
