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
//                                    Init.C                                 //
// ************************************************************************* //

#include <VisItInit.h>

#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <process.h>
#include <winsock2.h>
#else
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#endif
#include <new>
#include <cstring>

#include <DebugStreamFull.h>
#include <DebugStream.h>
#include <InstallationFunctions.h>
#include <TimingsManager.h>
#include <visit-config.h>

static void RemovePrependedDirs(const char *, char *); 
static char executableName[256];
static char componentName[256];
static const char *AVTPREP = "avtprep";
static const char *CLI = "cli";
static const char *ENGINE = "engine";
static const char *GUI = "gui";
static const char *LAUNCHER = "launcher";
static const char *MDSERVER = "mdserver";
static const char *VIEWER = "viewer";
static const char *const allComponentNames[] = {
    AVTPREP,
    CLI,
    ENGINE,
    GUI,
    LAUNCHER,
    MDSERVER,
    VIEWER
};
static ErrorFunction errorFunction = NULL;
static void *errorFunctionArgs = NULL;
static bool initializeCalled = false;
static bool finalizeCalled = false;

// ****************************************************************************
//  Function: striparg
//
//  Purpose:
//      Given an argc and an argv, strip off the arguments from i to i+n.
//
//  Arguments:
//      i           The first argument to strip      (i)
//      n           The number of arguments to strip (i)
//      argc        The number of total arguments    (io)
//      argv        The array of arguments           (io)
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Feb  1 16:05:05 PST 2002
//    Fixed a bug where it was copying beyond the end of the array.
//
// ****************************************************************************
static void
striparg(int i, int n, int &argc, char *argv[])
{
    for ( ; i+n<argc; i++)
        argv[i] = argv[i+n];

    argc -= n;
}

// ****************************************************************************
//  Function: NewHandler
//
//  Purpose:
//      Called when the component runs out of memory.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 20 13:07:51 PST 2001
//    Added an abort.
//
// ****************************************************************************
static void
NewHandler(void)
{
    debug1 << "This component has run out of memory." << endl;
    abort();
}

// ****************************************************************************
//  Function: VisItInit::Initialize
//
//  Purpose:
//      Parse any common commandline arguments (e.g. "-debug N") and do
//      common initialization.
//
//  Arguments:
//      argc        The number of total arguments                       (io)
//      argv        The array of arguments                              (io)
//      r           The rank of this process                            (i)
//      n           The total number of processes                       (i)
//      strip       Whether or not to strip the argument from the list. (i)
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Nov 27 16:25:07 PST 2000
//    I added a new flag that indicates whether or not to strip the argument
//    from the list after processing it.
//
//    Hank Childs, Sat Mar 10 15:35:36 PST 2001
//    Start timings manager too.
//
//    Hank Childs, Mon Aug 13 13:43:21 PDT 2001
//    Remove the directory before the program name to allow debug stream to
//    go to current directory.
//
//    Hank Childs, Tue Aug 21 13:05:28 PDT 2001
//    Registered a new handler.
//
//    Jeremy Meredith, Fri Feb  1 16:05:36 PST 2002
//    Added an 'else' to the if test when checking arguments, and added a
//    '--i' when stripping the arg.
//
//    Brad Whitlock, Thu Mar 14 12:43:34 PDT 2002
//    Made it work on Windows. Added code to initialize sockets.
//
//    Jeremy Meredith, Wed Aug  7 13:17:01 PDT 2002
//    Made it clamp the debug level to 0 through 5.
//
//    Brad Whitlock, Wed May 21 17:03:56 PST 2003
//    I made it write the command line arguments to the debug logs.
//
//    Brad Whitlock, Wed Jun 18 11:15:22 PDT 2003
//    I made it understand the -timing argument.
//
//    Hank Childs, Tue Jun  1 11:47:36 PDT 2004
//    Removed atexit call, since that is buggy with gcc.
//
//    Jeremy Meredith, Tue May 17 11:20:51 PDT 2005
//    Allow disabling of signal handlers.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added explicit call to SetFilename for TimingsManager. This is to
//    handle cases where TimingsManager may have already been instanced
//    before Initialize is called as in the engine.
//
//    Kathleen Bonnell, Wed Aug 22 18:00:57 PDT 2007 
//    On Windows, write timings and log files to User's directory. 
//
//    Cyrus Harrison, Tue Sep  4 10:54:59 PDT 2007
//    If -vtk-debug is specified make sure the debug level is at least 1
//    for the engine
//
//    Hank Childs, Tue Dec 18 15:53:10 PST 2007
//    Add support for -svn_revision.
//
//    Cyrus Harrison, Wed Jan 23 09:23:19 PST 2008
//    Changed set_new_handler to std::set_new_handler b/c of change from 
//    deprecated <new.h> to <new>
//
//    Brad Whitlock, Wed Jun 25 10:42:58 PDT 2008
//    Added check to return early in case this function gets called multiple
//    times as when you include multiple VisIt components in a single
//    executable.
//
//    Dave Pugmire, Thu Jan 15 16:35:08 EST 2009
//    Add support for -debug_engine_rank, limiting debug output to the specified
//    rank.
//
//    Brad Whitlock, Fri Apr 10 16:00:55 PDT 2009
//    I added support for reading -dv.
//
//    Mark C. Miller, Tue Apr 21 14:24:18 PDT 2009
//    Added logic to manage buffering of debug logs; an extra 'b' after level.
//
// ****************************************************************************

void
VisItInit::Initialize(int &argc, char *argv[], int r, int n, bool strip, bool sigs)
{
    // Return if Initialize has already been called.
    if(initializeCalled)
         return;
    initializeCalled = true;

    int i, debuglevel = 0, engineDebugRank=-1;
#if defined(_WIN32)
    bool usePid = true;
#else
    bool usePid = false;
#endif
    bool bufferDebug = false;
    bool clobberVlogs = false;
    bool vtk_debug = false;
    bool enableTimings = false;
    for (i=1; i<argc; i++)
    {
        if (strcmp("-debug_engine_rank", argv[i]) == 0)
        {
            engineDebugRank = -1;
            if (i+1 < argc && isdigit(*(argv[i+1])))
                engineDebugRank = atoi(argv[i+1]);
            else
                cerr<<"Warning: debug engine rank not specified. Igorning.\n";
            if (engineDebugRank >= n)
            {
                cerr<<"Warning: clamping debug engine rank to "<<n-1<<endl;
                engineDebugRank = n-1;
            }
            else if (engineDebugRank < 0)
            {
                cerr<<"Warning: clamping debug engine rank to 0\n";
                engineDebugRank = 0;
            }
            if(strip)
            {
                striparg(i,2, argc,argv);
                --i;
            }
            else
                ++i;
        }

        if (strncmp("-debug",argv[i],6) == 0)
        {
	    if ((strlen(argv[i]) > 7 && IsComponent(&argv[i][7])) ||
	         strlen(argv[i]) == 6)
	    {
                debuglevel = 1;
                if (i+1 < argc && isdigit(*(argv[i+1])))
                   debuglevel = atoi(argv[i+1]);
                else
                   cerr << "Warning: debug level not specified, assuming 1\n";

                if (debuglevel > 5)
                {
                    cerr << "Warning: clamping debug level to 5\n";
                    debuglevel = 5;
                }
                if (debuglevel < 0)
                {
                    cerr << "Warning: clamping debug level to 0\n";
                    debuglevel = 0;
                }
                if (i+1 < argc && *(argv[i+1]+1) == 'b')
                    bufferDebug = true;

                if(strip)
                {
                    striparg(i,2, argc,argv);
                    --i;
                }
                else
                {
                    ++i;
                }
	    }
        }
        else if (strcmp("-clobber_vlogs", argv[i]) == 0)
        {
            clobberVlogs = true;
        }
        else if (strcmp("-pid", argv[i]) == 0)
        {
            usePid = true;
        }
        else if (strcmp("-vtk-debug", argv[i]) == 0)
        {
            vtk_debug = true;
        }
        else if (strcmp("-timing",  argv[i]) == 0 ||
                 strcmp("-timings", argv[i]) == 0)
        {
            enableTimings = true;
        }
        else if (strcmp("-dv",  argv[i]) == 0)
        {
            SetIsDevelopmentVersion(true);
        }
        else if (strcmp("-svn_revision",  argv[i]) == 0)
        {
            if (strcmp(SVN_REVISION, "") == 0)
                cerr << "SVN revision is unknown!" << endl;
            else
                cerr << "Built from revision " << SVN_REVISION << endl;
            exit(0);
        }
    }

    char progname_wo_dir[256];
    RemovePrependedDirs(argv[0], progname_wo_dir);
    strcpy(executableName, progname_wo_dir);
    strcpy(componentName, progname_wo_dir);
#ifdef WIN32
    // On windows, we want timings and log files to go in user's directory,
    // not install directory, because users may not have write permissions.
    const char *home = getenv("VISITUSERHOME");
    std::string homedir;
    if(home != 0)
    {
        homedir = std::string(home);
        if(homedir[homedir.size() - 1] != SLASH_CHAR)
            homedir += SLASH_STRING;
        homedir += executableName;
        strcpy(progname_wo_dir, homedir.c_str());
    }
#endif
    char progname[256];
    if (n > 1)
    {
        sprintf(progname, "%s.%03d", progname_wo_dir, r);
    }
    else
    {
        strcpy(progname, progname_wo_dir);
    }
    
    if (usePid)
    {
#if defined(_WIN32)
        int pid = _getpid();
#else
        int pid = (int) getpid();
#endif
        char progname2[256];
        sprintf(progname2, "%s.%d", progname, pid);
        strcpy(progname, progname2);
    }

    // if this is the engine and -vtk-debug is enabled, make sure the
    // debuglevel is at least 1
    if(vtk_debug && strstr(progname,"engine") != NULL  && debuglevel < 1)
        debuglevel = 1;

    // If debug rank specified, and we are not rank 'r', then turn off debuging.
    if ((strncmp(progname, ENGINE, strlen(ENGINE)) == 0) &&
        engineDebugRank != -1 && engineDebugRank != r)
    {
        debuglevel = 0;
    }
    
    // Initialize the debug streams and also add the command line arguments
    // to the debug logs.
    DebugStreamFull::Initialize(progname, debuglevel, sigs, clobberVlogs, bufferDebug);
    for(i = 0; i < argc; ++i)
        debug1 << argv[i] << " ";
    debug1 << endl;

    TimingsManager::Initialize(progname);
    // In case TimingsManager was already initialized...
    visitTimer->SetFilename(progname);
    if(enableTimings)
        visitTimer->Enable();

#if !defined(_WIN32)
    std::set_new_handler(NewHandler);
#endif

#if defined(_WIN32)
    // Windows specific code
    WORD wVersionRequested;
    WSADATA wsaData;

    // Initiate the use of a Winsock DLL (WS2_32.DLL), necessary for sockets.
    wVersionRequested = MAKEWORD(2,2);
    WSAStartup(wVersionRequested, &wsaData);
#endif
}

// ****************************************************************************
//  Function: VisItInit::SetComponentName
//
//  Purpose: Sets the name of the component
//
// ****************************************************************************

void
VisItInit::SetComponentName(const char *cname)
{
   size_t len;

   if (cname != 0 && (len = strlen(cname)) > 0)
   {
      len = len < sizeof(componentName) ? len : sizeof(componentName) - 1;
      strncpy(componentName, cname, len);
      componentName[len]='\0';
   }
}

// ****************************************************************************
//  Function: VisItInit::GetComponentName
//
//  Purpose: Gets the name of the component. Defaults to name of the
//  executable of it was not set.
//
// ****************************************************************************

const char *
VisItInit::GetComponentName(void)
{
   return (const char *) componentName;
}

// ****************************************************************************
//  Function: VisItInit::ComponentNameToID
//
//  Purpose: Return integer index of component name in list of all components
//
// ****************************************************************************
const int
VisItInit::ComponentNameToID(const char *compName)
{
    int n = sizeof(allComponentNames) / sizeof(allComponentNames[0]);
    for (int i = 0; i < n; i++)
    {
        if (strcmp(compName, allComponentNames[i]) == 0)
            return i;
    }
    return -1;
}

// ****************************************************************************
//  Function: VisItInit::ComponentNameToID
//
//  Purpose: Return name of component with given index in list of all components
//
// ****************************************************************************
const char*
VisItInit::ComponentIDToName(const int id)
{
    int n = sizeof(allComponentNames) / sizeof(allComponentNames[0]);

    if (id >= 0 && id < n)
        return allComponentNames[id];
    else
        return "unknown";
}

// ****************************************************************************
//  Function: VisItInit::IsComponent
//
//  Purpose: Tests name of component against name passed as argument 
//
// ****************************************************************************
bool
VisItInit::IsComponent(const char *compName)
{
   if (strcmp(compName, VisItInit::GetComponentName()) == 0)
       return true;
   else
       return false;
}

// ****************************************************************************
//  Function: VisItInit::GetExecutableName
//
//  Purpose: Gets the name of the executable 
//
// ****************************************************************************

const char *
VisItInit::GetExecutableName(void)
{
   return (const char *) executableName;
}

// ****************************************************************************
//  Function: VisItInit::ComponentRegisterErrorFunction
//
//  Purpose:
//      Registers an error function.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
// ****************************************************************************

void
VisItInit::ComponentRegisterErrorFunction(ErrorFunction ef, void *args)
{
    errorFunctionArgs = args;
    errorFunction = ef;
}


// ****************************************************************************
//  Function: VisItInit::ComponentIssueError
//
//  Purpose:
//      Issues an error message on that component.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
// ****************************************************************************

void
VisItInit::ComponentIssueError(const char *msg)
{
    debug1 << "Error issued: " << msg << endl;
    if (errorFunction != NULL)
    {
        errorFunction(errorFunctionArgs, msg);
    }
    else
    {
        debug1 << "Not able to issue error because no function was registered."
               << endl;
    }
}


// ****************************************************************************
// Method: VisItInit::Finalize
//
// Purpose: 
//   Calls cleanup functions before the application exits.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 19 16:12:37 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Jun 18 11:14:50 PDT 2003
//   Added code to dump timings.
//
//   Hank Childs, Tue Jun  1 11:47:36 PDT 2004
//   Made the method be associated with the Init namespace.
//
//   Mark C. Miller, Tue Jul 25 18:26:10 PDT 2006
//   Pushed timer dump and finalization down into TimingsManager::Finalize 
//
//   Brad Whitlock, Wed Jun 25 10:44:24 PDT 2008
//   Added check to return early if Finalize has already been called, which
//   can happen when multiple VisIt components are part of the same executable.
//
// ****************************************************************************

void
VisItInit::Finalize(void)
{
    // Return if Finalize has already been called.
    if(finalizeCalled)
         return;
    finalizeCalled = true;

    TimingsManager::Finalize();

#if defined(_WIN32)
    // Terminates use of the WS2_32.DLL, the Winsock DLL.
    WSACleanup();
#endif
}

// ****************************************************************************
//  Function: RemovePrependedDirs
//
//  Purpose:
//      Removes prepended directories from a program name.
//
//  Arguments:
//      path      The input.  A program name with a path.
//      name      The output.  Only the name at the end.
//
//  Programmer:   Hank Childs
//  Creation:     August 13, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 22 18:00:57 PDT 2007
//    Use SLASH_CHAR.
//
// ****************************************************************************

static void
RemovePrependedDirs(const char *path, char *name)
{
    //
    // Find the last slash by going to the end and working backwards.
    //
    int  len = strlen(path);
    int lastSlash;
    for (lastSlash=len ; path[lastSlash]!=SLASH_CHAR && lastSlash>=0 ; lastSlash--)
    {
        continue;
    }

    //
    // Last slash can get to be less than zero if there are no slashes.
    //
    if (lastSlash < 0)
    {
        lastSlash = 0;
    }

    //
    //
    if (path[lastSlash] == SLASH_CHAR)
    {
        strcpy(name, path + lastSlash + 1);
    }
    else
    {
        strcpy(name, path + lastSlash);
    }
}
