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
#include <cerrno>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include <XDisplay.h>
#include <DebugStream.h>
#include <InitVTK.h>
#include <snprintf.h>
#include <StringHelpers.h>

#ifdef DEBUG_X
#   define DEBUG_ONLY(block) block
#else
#   define DEBUG_ONLY(block) /* nothing */
#endif

static inline bool initialized(pid_t x) { return ((int)x != -1); }
static inline void set_uninitialized(pid_t *x) { *x = (pid_t) -1; }
static char **vec_convert(std::vector<std::string> svec, size_t *len);
void vec_convert_free(char **vec, size_t len);

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

XDisplay::XDisplay(): xserver((pid_t)-1), display(0) { }

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
//    Tom Fogal, Tue Aug  5 16:33:49 EDT 2008
//    Add argument string-vector, and code to convert that into an argv-style
//    array.  Finally, use execvp to start xinit.  All of this allows the user
//    to specify their own custom arguments to the X launch.
//
//    Tom Fogal, Mon Aug 11 19:02:11 EDT 2008
//    Move print out of the child; this is confusing for the output stream (two
//    processes can write to it, concurrently!).  Convert to an argv[] array in
//    both processes, so that we can still output the X server options.
//
// ****************************************************************************

bool
XDisplay::Initialize(size_t display, const std::vector<std::string> &user_args)
{
    char **argv;
    size_t v_elems;

    this->display = display;

    std::vector<std::string> args;
    args.push_back("xinit");
    args.push_back("--");
    args.push_back(format(":%l", /* unused */0, display));
    args.push_back("-sharevts");
    args.push_back("-once");
    args.push_back("-terminate");
    StringHelpers::append(args, user_args);

    argv = vec_convert(args, &v_elems);
    if((this->xserver = fork()) == (pid_t) -1)
    {
        perror("fork");
        vec_convert_free(argv, v_elems);
        return false;
    }

    if(this->xserver == 0)
    {
        execvp("xinit", argv);
        perror("execvp of xinit");
        vec_convert_free(argv, v_elems);
        return false;
    }
    DEBUG_ONLY(
        debug5 << "X server command line arguments:" << std::endl;
        for(size_t i=0; i < args.size(); ++i)
        {
            debug5 << "\t" << argv[i] << std::endl;
        }
    )

    vec_convert_free(argv, v_elems);
    debug4 << "Giving a sec for the X server to start ...";
    sleep(display);
    debug4 << " done!" << std::endl;

    debug3 << "Saved X server PID " << (int)this->xserver << std::endl;
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
// ****************************************************************************

void
XDisplay::Connect()
{
    static char env_display[128];

    debug3 << "Connecting to display " << this->display << std::endl;
    SNPRINTF(env_display, 128, "DISPLAY=:%zu", this->display);

    if(putenv(env_display) != 0)
    {
        perror("putenv");
        debug1 << "putenv(\"" << env_display << "\") failed." << std::endl;
    }
    InitVTK::UnforceMesa();

    system("xhost +");
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
// ****************************************************************************

void
XDisplay::Teardown()
{
    unsetenv("DISPLAY");

    debug3 << "Tearing down display " << this->xserver << std::endl;

    // First, lets try to be nice about it.
    if(kill(this->xserver, SIGINT) < 0)
    {
        perror("Killing X via SIGINT");
        sleep(2);
        // Hrumph.
        if(kill(this->xserver, SIGKILL) < 0)
        {
            char err[1024];
            strerror_r(errno, err, 1024);
            debug1 << "Could not stop X server: " << err << std::endl;
            std::cerr << "Could not stop the X server: " << err << std::endl
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
