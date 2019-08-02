/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <visit-config.h>
#include <windows.h>
#include <Winbase.h>
#include <errno.h>
#include <process.h>
#include <sys/stat.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <snprintf.h>

#include <vector>
#include <string>
#include <iostream>

using std::vector;
using std::string;
using std::cerr;
using std::endl;
typedef vector<int> intVector;
typedef vector<intVector> intIntVector;
typedef vector<string> stringVector;

/* Uncomment these definitions if you need to debug the functions. */
/*
#define DEBUG_GetPathToOlderConfigFiles
#define DEBUG_MigrateConfigFiles
#define DEBUG_VisItPath
*/


/*
 * Macros
 */
#define ARG(A) (strcmp((A), argv[i]) == 0)

#define ENDSWITH(A) EndsWith(argv[i], A)

#define BEGINSWITHQUOTE(A) (A[0] == '\'' || A[0] == '\"')

#define ENDSWITHQUOTE(A) (A[strlen(A)-1] == '\'' || A[strlen(A)-1] == '\"')

#define HASSPACE(A) (strstr(A, " ") != NULL)
#define HASANGLEBRACKET(A) (strstr(A, "<") != NULL || strstr(A, ">") != NULL)
#define HASSPECIAL(A) (HASSPACE(A) || HASANGLEBRACKET(A))
#define DQUOTEARG(A) (tmpArg = string("\"") + string(A) + string("\""))


/*
 * Constants
 */
static const char usage[] = 
"USAGE:  visit [arguments]\n"
"\n"
"    Program arguments:\n"
"        -gui                 Run with the Graphical User Interface (default)\n"
"        -cli                 Run with the Command Line Interface\n"
"        -silex               Run silex\n"
"        -xmledit             Run xmledit\n"
"        -mpeg2encode         Run mpeg2encode program for movie-making\n"
"        -composite           Run composite program for movie-making\n"
"        -transition          Run transition program for movie-making\n"
"\n"
"    Window arguments:\n"
"        -small               Use a smaller desktop area/window size\n"
"        -geometry   <spec>   What portion of the screen to use.  This is a\n"
"                                 standard X Windows geometry specification\n"
"        -style      <style>  One of: windows,cde,motif,sgi\n"
"        -background <color>  Background color for GUI\n"
"        -foreground <color>  Foreground color for GUI\n"
"        -nowin               Run without viewer windows\n"
"        -newconsole          Run the component in a new console window\n"
"\n"
"    Scripting arguments:\n"
"        -s    <scriptname>   Runs the specified VisIt script. Note that\n"
"                             this argument only takes effect with -cli.\n"
"        -verbose             Prints status information during pipeline\n"
"                             execution.\n"
"\n"
"    Debugging arguments:\n"
"        -debug <level>       Run with <level> levels of output logging.\n"
"                             <level> must be between 1 and 5.\n"
"\n"
"    Developer arguments:\n"
"        -xml2cmake           Run the xml2cmake tool.\n"
"        -xml2atts            Run the xml2atts tool.\n"
"        -public              xml2cmake: force install plugins publicly\n"
"        -private             xml2cmake: force install plugins privately\n"
"        -clobber             Permit xml2... tools to overwrite old files\n"
"        -noprint             Silence debugging output from xml2... tools\n"
"        -env                 Print environment strings set up by the launcher\n"
"\n";

/*
 * Prototypes
 */
string GetVisItEnvironment(stringVector &, bool, bool &);

void   SetVisItEnvironment(const stringVector &);
string AddPath(char *, const char *, const char*);
bool   ReadKey(const char *key, char **keyval);
void   PrintEnvironment(void);
string WinGetEnv(const char * name);

static bool EndsWith(const char *s, const char *suffix)
{
    bool retval = false;
    size_t lens = strlen(s);
    size_t lensuffix = strlen(suffix);
    if(lens >= lensuffix)
    {
        const char *start = s + lens - lensuffix;
        retval = strcmp(start, suffix) == 0;
    }
    return retval;
}

/******************************************************************************
 *
 * Purpose: This is the main function for a simple C program that launches
 *          VisIt's components on MS Windows.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Apr 10 12:23:32 PDT 2002
 *
 * Modifications:
 *   Brad Whitlock, Mon Jul 15 10:57:32 PDT 2002
 *   I changed the code so it uses the entire path to the executable. This 
 *   prevents an error where the viewer cannot be found because of another 
 *   Windows program called "viewer".
 *
 *   Brad Whitlock, Thu Jul 18 11:32:52 PDT 2002
 *   I added quotes around the path and name of the component being run
 *   since it almost always gets installed in a path that has spaces in
 *   the name.
 *
 *   Brad Whitlock, Mon Feb 10 11:59:38 PDT 2003
 *   I added code to prevent the -key argument from being printed.
 *
 *   Brad Whitlock, Thu Apr 24 07:26:43 PDT 2003
 *   I added code to strip off the -v version arguments because they are
 *   not important on Windows.
 *
 *   Brad Whitlock, Wed May 7 09:49:59 PDT 2003
 *   I added the vcl component.
 *
 *   Brad Whitlock, Tue Aug 12 09:41:07 PDT 2003
 *   I added some support for movie making.
 *
 *   Brad Whitlock, Tue Nov 11 17:20:05 PST 2003
 *   I added code to launch Silex and XMLedit.
 *
 *   Brad Whitlock, Tue Mar 8 10:28:21 PDT 2005
 *   I added support for adding any additional arguments in the registry's
 *   VISITARGS key to the component's command line.
 *
 *   Brad Whitlock, Fri Jun 3 14:46:18 PST 2005
 *   Added code to flush stderr so the Java client can start up the viewer.
 *
 *   Brad Whitlock, Wed Jun 22 17:24:08 PST 2005
 *   Added support for -newconsole.
 *
 *   Brad Whitlock, Thu Feb 2 14:48:18 PST 2006
 *   I changed makemovie.py to makemoviemain.py so right-click movies and
 *   the "visit -movie" commands work again.
 *
 *   Brad Whitlock, Tue Sep 19 17:09:57 PST 2006
 *   Added support for mpeg2enc.exe so we can create MPEG movies on Windows.
 * 
 *   Brad Whitlock, Thu Dec 21 14:52:11 PST 2006
 *   Added support for transition and composite programs.
 *
 *   Kathleen Bonnell, Thu Mar 22 09:29:45 PDT 2007
 *   Enclose argv[i] in quotes before calling PUSHARG, if there are spaces. 
 *
 *   Kathleen Bonnell, Mon Jul  2 10:43:29 PDT 2007 
 *   Remove last fix. 
 *
 *   Kathleen Bonnell, Tue Jul 24 15:19:23 PDT 2007 
 *   Added tests for spaces in args, so they can be surrounded in quotes, and
 *   for args beginning with quotes so that the entire arg can be concatenated
 *   into 1 argument surrounded by quotes. 
 *
 *   Kathleen Bonnell, Tue Jan  8 18:09:38 PST 2008 
 *   When parsing args, moved around the order of testing for spaces and
 *   surrounding quotes, to fix problem with path-with-spaces used with -o. 
 * 
 *   Kathleen Bonnell, Fri Feb 29 16:43:46 PST 2008 
 *   Added call to free printCommand. 
 *
 *   Kathleen Bonnell, Thu Jul 17 4:16:22 PDT 2008 
 *   Added call to free visitargs if not found, because ReadKey does the
 *   allocation regardless.
 *
 *   Kathleen Bonnell, Wed Sep 10 16:19:53 PDT 2008 
 *   Added loopback support: it replaces the remote host name with 127.0.0.1
 *   unless the "-noloopback" flag is given by visit or by the user.
 *
 *   Brad Whitlock, Mon Nov 17 12:11:35 PST 2008
 *   I fixed a bug with host renaming that caused invalid security keys on
 *   machines with short hostnames.
 *
 *   Kathleen Bonnell, Tue Mar 17 16:55:32 MST 2009 
 *   Added 'TestForConfigFiles' and attendant methods.
 *
 *   Kathleen Bonnell, Frid Sep 11 10:43:57 MST 2009
 *   Changed mpeg2encode executable to the correct name.
 *
 *   Kathleen Bonnell, Fri Jan 29 9:01:15 MST 2009
 *   Changed engine executable name to engine_ser.
 *
 *   Kathleen Bonnell, Wed Dec 1 08:45:12 MST 2010
 *   Add support for xml2cmake.
 *
 *   Kathleen Bonnell, Tue May 3 14:31:50 MST 2011
 *   Add support for -env. 
 *
 *   Kathleen Bonnell, Mon Sep 26 07:13:08 MST 2011
 *   Add support for parallel engine.  Use string for arg storage.
 *
 *   Kathleen Bonnell, Thu Sep 29 16:41:28 MST 2011
 *   Pass over 'visit' and '-visit' args.
 * 
 *   Brad Whitlock, Thu Dec 8 14:51:PST 2011
 *   Skip over arguments that end with 'visit', 'visit.exe', 'visit"', 
 *   'visit.exe"' since we're starting the argv iteration at 0, which means 
 *   we'll pick up the visit.exe program.
 *
 *   Brad Whitlock, Tue Dec 13 10:49:34 PDT 2011
 *   I added all command line arguments to a string vector instead of building
 *   up a string. I also added support for a different style of launching if
 *   we're building the launcher as a windows app. I also added message box
 *   debugging for the launcher so we can see the command line and environment
 *   we're attempting to use.
 *
 *   Kathleen Biagas, Fri May 4 14:05:27 PDT 2012
 *   If working from a dev build, pass "-dv" to components.
 *
 *   Kathleen Biagas, Mon Jul 16 13:43:12 MST 2012
 *   Prevent arguments ending in '.visit' from being skipped.
 *
 *   Kathleen Biagas, Wed Nov 7 09:46:15 PDT 2012
 *   Removed 'TestForConfigFIles' and attendent methods. Remove version # 
 *   from VISITUSERHOME path.
 *
 *   Kathleen Biagas, Wed Dec 19 17:35:21 MST 2012
 *   Return '0' instead of '1' when only printing environment.
 *
 *   Kathleen Biagas, Tue Sep 29 15:51:12 MST 2015
 *   Add movie args before any others when running -movie.
 *
 *   Kathleen Biagas, Tue Dec 15 13:30:47 MST 2015
 *   Add '<' and '>' to the list of special characters that indicate an
 *   argument needs to be re-surrounded by quotes when passing along to
 *   visit's components.  Use double-quotes when re-surrounding instead of
 *   single.
 *
 *   Kathleen Biagas, Mon Jul 17 15:11:09 MST 2017
 *   Ensure '-dv' isn't added multiple times to the command line.
 *   Change placement of ';' when creating the engineArgs string for
 *   the command line.
 *
 *   Kathleen Biagas, Fri Sep 7 09:34:27 MST 2017
 *   Support MSMPI version 8 which creates an MSMPI_BIN env var, and include/
 *   libs are in a completely different location, so don't assume mpiexec
 *   can be found from MSMPI_INC.
 *
 *   Kathleen Biagas, Tue Nov 21 13:43:42 MST 2017
 *   Display message box if attempting to use parallel engine but mpiexec
 *   not found.
 *
 *   Kathleen Biagas, Thu Sep 27 11:43:37 PDT 2018
 *   Change private plugin directory to userHome.
 *
 *   Kathleen Biagas, Thu Aug 1 13:41:12 MST 2019
 *   Removed usage of shortname (8dot3name). This utility may be disabled on
 *   Windows systems, so it shouldn't be relied upon any more.  Added spawnv
 *   error message if debuglaunch specified.
 *
 *****************************************************************************/

int
VisItLauncherMain(int argc, char *argv[])
{
    bool addMovieArguments = false; 
    bool addCinemaArguments = false; 
    bool addVISITARGS = true; 
    bool addPluginVars = false;
    bool newConsole = false;
    bool noloopback = false;
    bool parallel = false;
    bool hostset = 0;
    bool envOnly = false;
    bool debugLaunch = false;
    bool apitrace = false;

    string nps("2");
    stringVector componentArgs;
    stringVector engineArgs;
    string component("gui");
    string tmpArg;
    string apitrace_component("");
    

    //
    // Parse the command line arguments.
    // 
    for(int i = 0; i < argc; ++i)
    {
        if(ARG("-visit"))
        {
           continue; 
        }
        else if (!ENDSWITH(".visit") && !ENDSWITH(".visit\"") &&
                (ENDSWITH("visit")   || ENDSWITH("visit.exe") ||
                ENDSWITH("visit\"") || ENDSWITH("visit.exe\"")))
        {
            continue;
        }
        else if(ARG("-help"))
        {
            printf("%s", usage);
            return 0;
        }
        else if(ARG("-version"))
        {
            printf("%s\n", VISIT_VERSION);
            return 0;
        }
        else if(ARG("-gui"))
        {
            component = "gui";
            addVISITARGS = true;
        }
        else if(ARG("-cli"))
        {
            component = "cli";
            addVISITARGS = true;
        }
        else if(ARG("-viewer"))
        {
            component = "viewer";
            addVISITARGS = false;
        }
        else if(ARG("-mdserver"))
        {
            component = "mdserver";
            addVISITARGS = false;
        }
        else if(ARG("-engine"))
        {
            component = "engine";
            addVISITARGS = false;
        }
        else if(ARG("-vcl"))
        {
            component = "vcl";
            addVISITARGS = false;
        }
        else if(ARG("-movie"))
        {
            component = "cli";
            addMovieArguments = true;
        }
        else if(ARG("-cinema"))
        {
            component = "cli";
            addCinemaArguments = true;
        }
        else if(ARG("-mpeg2encode"))
        {
            component = "mpeg2encode";
            addVISITARGS = true;
        }
        else if(ARG("-transition"))
        {
            component = "visit_transition";
            addVISITARGS = true;
        }
        else if(ARG("-composite"))
        {
            component = "visit_composite";
            addVISITARGS = true;
        }
        else if(ARG("-mpeg_encode"))
        {
            fprintf(stderr, "The mpeg_encode component is not supported "
                            "on Windows! You can only generate sequences "
                            "of still images.\n");
            return -1;
        }
        else if(ARG("-xmledit"))
        {
            component = "xmledit";
            addVISITARGS = false;
        }
        else if(ARG("-silex"))
        {
            component = "silex";
            addVISITARGS = false;
        }
        else if(ARG("-v"))
        {
            /* Skip the next argument too. */
            ++i;
        }
        else if(ARG("-newconsole"))
        {
            newConsole = true;
        }
        else if(ARG("-noloopback"))
        {
            noloopback = true;
            componentArgs.push_back("-noloopback");
        }
        else if(ARG("-par"))
        {
            if(component == "engine")
            {
                parallel = true;
            }
            else if(component != "mdserver")
            {
                engineArgs.push_back("-par");
            }
        }
        else if(ARG("-np"))
        {       
            if (component == "engine")
            {
                parallel = true;
                nps = string(argv[i+1]);
            }
            else if(component != "mdserver")
            {
                engineArgs.push_back("-np");
                engineArgs.push_back(argv[i+1]);
            }
            // skip next argument.
            ++i;
        }
        else if(ARG("-host"))
        {
            hostset = true;
            componentArgs.push_back("-host");
        }
        else if(ARG("-xml2cmake"))
        {
            component = "xml2cmake";
            addVISITARGS = false;
            addPluginVars = true;
        }
        else if(ARG("-xml2atts"))
        {
            component = "xml2atts";
            addVISITARGS = false;
            addPluginVars = true;
        }
        else if(ARG("-xml2java"))
        {
            component = "xml2java";
            addVISITARGS = false;
            addPluginVars = true;
        }
        else if(ARG("-xml2python"))
        {
            component = "xml2python";
            addVISITARGS = false;
            addPluginVars = true;
        }
        else if(ARG("-xml2info"))
        {
            component = "xml2info";
            addVISITARGS = false;
            addPluginVars = true;
        }
        else if(ARG("-xml2plugin"))
        {
            component = "xml2plugin";
            addVISITARGS = false;
            addPluginVars = true;
        }
        else if(ARG("-env"))
        {
            envOnly = true;
        }
        else if(ARG("-debuglaunch"))
        {
            debugLaunch = true;
            componentArgs.push_back("-debuglaunch");
        }
        else if(ARG("-dv"))
        {
            continue;
        }
        else if(ARG("-apitrace"))
        {
            apitrace_component = string(argv[i+1]);
            if (component == apitrace_component)
            {
                apitrace = true;
            }
            else
            {
                componentArgs.push_back("-apitrace");
                componentArgs.push_back(apitrace_component);
            }
            ++i;
        }
        else
        {
            if (!BEGINSWITHQUOTE(argv[i]) && HASSPECIAL(argv[i]))
            {
                DQUOTEARG(argv[i]);
                componentArgs.push_back(tmpArg);
            }
            else if (BEGINSWITHQUOTE(argv[i]) && !ENDSWITHQUOTE(argv[i]))
            {
                tmpArg = argv[i];
                int nArgsSkip = 1;
                for (int j = i+1; j < argc; j++)
                {
                    nArgsSkip++;
                    tmpArg += " ";
                    tmpArg += argv[j];
                    if (ENDSWITHQUOTE(argv[j]))
                        break;
                }
                componentArgs.push_back(tmpArg);
                i += (nArgsSkip -1);
            }
            else 
            {
                componentArgs.push_back(argv[i]);
            }
        }
    }

    if(component == "engine")
        component = parallel ? "engine_par" : "engine_ser";

    string mpipath;
    if (component == "engine_par")
    {
        mpipath = WinGetEnv("VISIT_MPIEXEC");
        // first look for an EnvVar set by the installer
        if (mpipath.empty())
        {
            // Check for EnvVar set by MSMPI 8.1 redist installer
            mpipath = WinGetEnv("MSMPI_BIN");
            if (!mpipath.empty())
            {
                // add the executable
                mpipath += "\\mpiexec.exe";
            }
        }
        if (mpipath.empty())
        {
            // Check for EnvVar set by MSMPI R2 redist installer
            mpipath = WinGetEnv("MSMPI_INC");
            if (!mpipath.empty())
            {
                mpipath += "\\..\\Bin\\mpiexec.exe";
                // found the include path, point to Bin
                // however, this env var is also set by MSMPI 8.1 SDK,
                // so actually should check that executable exists
                if (!PathFileExists(mpipath.c_str()))
                {
                    mpipath.clear();
                }
            }
        }
        if (mpipath.empty())
        {
            // Check for EnvVar set by HPC SDK installer
            mpipath = WinGetEnv("CCP_SDK");
            if (!mpipath.empty())
            {
                // found the base path, point to Bin
                mpipath += "\\Bin\\mpiexec.exe";
                // check that mpiexec exists
                if (!PathFileExists(mpipath.c_str()))
                {
                    mpipath.clear();
                }
            }
        }
        if (mpipath.empty() || !PathFileExists(mpipath.c_str()))
        {
            string msg("Could not find path to \"mpiexec\".\nIf it is installed on your system, please set an environment variable 'VISIT_MPIEXEC' that points to its location. VisIt will launch a serial engine for this session.");
#ifdef VISIT_WINDOWS_APPLICATION
            MessageBox(NULL,
                       (LPCSTR)msg.c_str(),
                       (LPCSTR)"Missing mpiexec",
                       MB_ICONEXCLAMATION | MB_OK);
#else
            cerr << msg << endl;
#endif
            parallel = false;
            component = "engine_ser";
        }
    }

    if (parallel && !noloopback)
    {
        noloopback = true;
        componentArgs.push_back("-noloopback");
    }

    //
    // If we want a new console, allocate it now.
    //
    if(newConsole || component == "engine_par")
    {
        FreeConsole();
        AllocConsole();
#ifdef VISIT_WINDOWS_APPLICATION
        // If we're running a parallel engine then let's hide the console window.
        if(component == "engine_par")
            ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    }

    //
    // Add some stuff to the environment.
    //
    stringVector visitEnv;
    bool usingDev;
    string visitpath = GetVisItEnvironment(visitEnv, addPluginVars, usingDev);
    if (usingDev)
        componentArgs.push_back("-dv");
    SetVisItEnvironment(visitEnv);
#ifdef VISIT_WINDOWS_APPLICATION
    if(debugLaunch)
    {
        // Show the path and the environment we've created.
        string envStr;
        for(size_t i = 0; i < visitEnv.size(); ++i)
            envStr = envStr + visitEnv[i] + "\n";
        string msgStr((visitpath + "\n\n") + envStr);
        MessageBox(NULL, msgStr.c_str(), component.c_str(), MB_OK);
    }
#endif

    if (envOnly)
    {
        PrintEnvironment();
        componentArgs.clear();
        return 0;
    }

    stringVector command;
    command.reserve(20);
    string quote("\"");

    if (apitrace)
    {
        string apitrace_path = WinGetEnv("APITRACE_EXE");
        command.push_back(apitrace_path);
        command.push_back("trace");
        command.push_back("--api");
        command.push_back("gl");
    }

    if (component == "engine_par")
    {
        // we've already determined the path to mpiexec, if it wasn't
        // found, we've switched to a serial engine.
        // mpi exec
        command.push_back(mpipath);
        // mpi exec args, 
        command.push_back("-n");
        command.push_back(nps);
        // engine_par, Surrounded by quotes
        command.push_back(quote + string(visitpath) + string("\\") +
                          string(component) + string(".exe") + quote);
    }
    else
    {
        string program(visitpath + string("\\") + component + string(".exe"));
        command.push_back(program);
    }

    if(addMovieArguments)
    {
        command.push_back("-s");
        command.push_back(quote + visitpath + string("\\makemoviemain.py") + quote);
        command.push_back("-nowin");
    }
    else if(addCinemaArguments)
    {
        command.push_back("-s");
        command.push_back(quote + visitpath + string("\\visitcinemamain.py") + quote);
        command.push_back("-nowin");
    }

    for(size_t i = 0; i < componentArgs.size(); ++i)
    {
        if((componentArgs[i] == "-host") && !noloopback)
        {
            // Replace the host arg with the loopback
            componentArgs[i+1] = "127.0.0.1"; 
        }

        if (!BEGINSWITHQUOTE(componentArgs[i].c_str()) && 
            HASSPECIAL(componentArgs[i].c_str()))
        {
            DQUOTEARG(componentArgs[i].c_str());
            command.push_back(tmpArg);
        }
        else
            command.push_back(componentArgs[i]);
        
    }

    if (!engineArgs.empty())
    {
        command.push_back("-launchengine");
        command.push_back("localhost");
        command.push_back("-engineargs");
        string eArgs;
        for (size_t i = 0; i < engineArgs.size(); ++i)
        {
            eArgs.append(engineArgs[i]);
            if (i < engineArgs.size()-1)
                eArgs.append(";");
        }
        command.push_back(eArgs);
    }

    //
    // Create the command to execute and the string that we print.
    //
    if(addVISITARGS)
    {
        char *visitargs = 0;
        if(ReadKey("VISITARGS", &visitargs))
        {
            command.push_back(visitargs);
        }
        free(visitargs);
        visitargs = 0;
    }

    // 
    // Print the run information.
    // 
    // cmdLine is used with console-app version as argument to system command.
    // so the first 'arg' must be quoted.
    string cmdLine(quote + command[0] + quote);
    for(size_t i = 1; i < command.size(); ++i)
        cmdLine.append(string(" ") + command[i]);
    cerr << "Running: " << cmdLine << endl;

    int retVal = 0;
#ifdef VISIT_WINDOWS_APPLICATION
    if(debugLaunch)
        MessageBox(NULL, cmdLine.c_str(), component.c_str(), MB_OK);

    // We can't use system() since that opens a cmd shell.
    // the exeName is the second arg to _spawnv, and doesn't need quotes,
    // but when used as the first arg in the exeArgs list, it does. 
    const char *exeName = command[0].c_str();
    const char **exeArgs = new const char *[command.size()+1];
    string quotedCommand(quote + command[0] + quote);
    exeArgs[0] = quotedCommand.c_str();
    for(size_t i = 1; i < command.size(); ++i)
        exeArgs[i] = command[i].c_str();
    exeArgs[command.size()] = NULL;
    retVal =_spawnv( _P_WAIT, exeName, exeArgs);
    if (debugLaunch && retVal == -1)
    {
        errno_t err;
        _get_errno(&err);
        char errmsg[30];
        switch(err)
        {
            case E2BIG:
                _snprintf(errmsg, 30, "_spawn error: %d: E2BIG\n", err);
                break;
            case EINVAL:
                _snprintf(errmsg, 30, "_spawn error: %d: EINVAL\n", err);
                break;
            case ENOENT:
                _snprintf(errmsg, 30, "_spawn error: %d: ENOENT\n", err);
                break;
            case ENOEXEC:
                _snprintf(errmsg, 30, "_spawn error: %d: ENOEXEC\n", err);
                break;
            case ENOMEM:
                _snprintf(errmsg, 30,  "_spawn error: %d: ENOMEM\n", err);
                break;
            default: 
                _snprintf(errmsg, 30, "_spawn error: %d: UNKNOWN\n", err);
                break;
        }
        MessageBox(NULL, errmsg, component.c_str(), MB_OK);
    }
    delete [] exeArgs;
#else
    // without shortname, need to quote the entire string so the quotes around
    // the first arg (the executable) aren't lost.
    cmdLine = quote + cmdLine + quote;
    retVal = system(const_cast<char*>(cmdLine.c_str()));
#endif

    componentArgs.clear();
    return retVal;
}

/******************************************************************************
 *
 * Purpose: Reads a string value from the Windows registry entry for the
 *          current version of VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 26 13:08:21 PST 2002
 *
 * Input Arguments:
 *   which_root : The root key to open.
 *   key        : The key that we're looking for.
 *
 * Output Arguments:
 *   keyval : A string containing the value for the key. This memory is
 *            allocated regardless of whether or not the key is found and it
 *            should be freed by the caller.
 *
 * Modifications:
 *   Kathleen Bonnell, Fri Feb 29 16:43:46 PST 2008 
 *   Only malloc keyval if it is null. 
 *
 *   Kathleen Bonnell, Thu Jun 17 20:23:51 MST 2010
 *   Location of VisIt's registry keys has changed to Software\Classes.
 *
 *****************************************************************************/

bool
ReadKeyFromRoot(HKEY which_root, const char *key, char **keyval)
{
    bool readSuccess = false;
    char regkey[100];
    HKEY hkey;

    /* 
     * Try and read the key from the system registry. 
     */
    sprintf(regkey, "Software\\Classes\\VISIT%s", VISIT_VERSION);
    if (*keyval == 0)
        *keyval = (char *)malloc(500);
    
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_QUERY_VALUE, &hkey) == 
       ERROR_SUCCESS)
    {
        DWORD keyType, strSize = 500;
        if(RegQueryValueEx(hkey, key, NULL, &keyType, (LPBYTE)*keyval, 
                           &strSize) == ERROR_SUCCESS)
        {
            readSuccess = true;
        }

        RegCloseKey(hkey);
    }

    return readSuccess;
}

/******************************************************************************
 *
 * Purpose: Reads a string value from the Windows registry entry for the
 *          current version of VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 26 13:08:21 PST 2002
 *
 * Input Arguments:
 *   key : The key that we're looking for.
 *
 * Output Arguments:
 *   keyval : A string containing the value for the key. This memory is
 *            allocated regardless of whether or not the key is found and it
 *            should be freed by the caller.
 *
 * Modifications:
 *   Brad Whitlock, Mon Jul 12 16:34:18 PST 2004
 *   I made it use ReadKeyFromRoot so we can check for VisIt information
 *   in a couple places. This is to avoid the situation where VisIt won't
 *   run when installed without Administrator access.
 *
 *   Kathleen Bonnell, Thu Jun 17 20:23:51 MST 2010
 *   VisIt's registry keys are stored in HKLM or HKCU.
 *
 *****************************************************************************/

bool
ReadKey(const char *key, char **keyval)
{
    bool retval = false;

    if((retval = ReadKeyFromRoot(HKEY_LOCAL_MACHINE, key, keyval)) == 0)
        retval = ReadKeyFromRoot(HKEY_CURRENT_USER, key, keyval);
    
    return retval;     
}


/******************************************************************************
 *
 * Purpose: Returns environment information needed to run VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Apr 16 14:26:30 PST 2002
 *
 * Modifications:
 *   Brad Whitlock, Thu May 9 11:50:49 PDT 2002
 *   I made it read from the Windows registry to get the installation path.
 *
 *   Brad Whitlock, Mon Jul 15 10:59:19 PDT 2002
 *   I made the function return the path to the VisIt installation.
 *
 *   Brad Whitlock, Mon Aug 26 12:54:36 PDT 2002
 *   I made it try and read SSH and SSHARGS from the registry.
 *
 *   Brad Whitlock, Thu Sep 5 15:58:07 PST 2002
 *   I added VISITHOME.
 *
 *   Brad Whitlock, Fri Feb 28 12:24:11 PDT 2003
 *   I improved how the path is added and I fixed a potential memory problem.
 *
 *   Brad Whitlock, Wed Apr 23 09:28:44 PDT 2003 
 *   I added VISITSYSTEMCONFIG to the environment.
 *
 *   Brad Whitlock, Tue Aug 12 10:47:16 PDT 2003
 *   I added an option that lets us return VisIt's path as the short system
 *   name version of the path so we can effectively do visit -movie from
 *   the command line.
 *
 *   Brad Whitlock, Wed Dec 10 16:12:21 PST 2003
 *   I changed the default directory that we use if VISITHOME is not
 *   found. In that case, it now tries to look up VISITDEVDIR. Finally,
 *   if that is not found then it resorts to using
 *
 *   Brad Whitlock, Mon Aug 16 09:22:53 PDT 2004
 *   Added binary locations for MSVC7.Net versions of VisIt.
 *
 *   Kathleen Bonnell, Thu Jul 19 07:56:22 PDT 2007
 *   Added VISITUSERHOME, which defaults to VISITHOME if the reg key could
 *   not be found.  Create the directory if it does not exist, and add 
 *   "My images" subdir for saving windows.
 *
 *   Kathleen Bonnell, Tue Jan  8 18:09:38 PST 2008 
 *   Account for the fact that VisIt may be built with MSVC8, so location of
 *   config dir and binaries differs -- use new _VISIT_MSVC define. 
 *   
 *   Kathleen Bonnell, Fri Feb 29 16:43:46 PST 2008 
 *   Added call to free visituserpath and visitdevdir.
 *
 *   Kathleen Bonnell, Thu Apr 17 10:20:25 PDT 2008
 *   Use SHGetFolderPath to retrieve "My Documents" folder path (no longer
 *   stored in registry at install) to better support roaming profiles.
 *   Use "Application Data" path for private plugins, as users have write-
 *   privileges there (and it better supports roaming profiles).
 *
 *   Kathleen Bonnell, Wed May 21 08:12:16 PDT 2008 
 *   Use ';' to separate different paths for VISITPLUGINDIR. 
 *
 *   Kathleen Bonnell, Mon Jun 2 18:08:32 PDT 2008
 *   Change how VisItDevDir is retrieved and stored.
 *
 *   Kathleen Bonnell, Thu July 17 16:20:22 PDT 2008 
 *   Free visitpath if we didn't find VISITHOME, because ReadKey mallocs
 *   regardless.  Same for tempvisitdev.  If VISITDEVDIR not defined, then
 *   find the full path to this executable and use it for visitpath and
 *   visitdevdir instead.
 *
 *   Kathleen Bonnell, Thu July 31 16:55:43 PDT 2008 
 *   Initialize visitdevdir.
 *
 *   Kathleen Bonnell, Wed Oct 8 08:49:11 PDT 2008 
 *   Re-organized code.  Modified settingup of VISITSSH and VISITSSHARGS so
 *   that if these are already set by user in environment they won't be 
 *   overwritten. 
 *
 *   Kathleen Bonnell, Wed Apr 22 17:47:34 PDT 2009
 *   Added VISITULTRAHOME env var.
 *
 *   Kathleen Bonnell, Wed Feb  3 13:59:42 PST 2010
 *   Removed use of VISITDEVDIR env var.
 *
 *   Kathleen Bonnell, Wed Mar 24 16:21:03 MST 2010
 *   Check for VISITHOME in env. Set PYTHONPATH.
 *
 *   Kathleen Bonnell, Tue Mar 30 16:46:19 MST 2010
 *   Test for dev dir and set vars accordingly.
 *
 *   Kathleen Bonnell, Wed Dec 1 08:43:44 MST 2010 
 *   Add variables necessary for plugin development if necessary.
 *
 *   Kathleen Bonnell, Tue May 3 14:33:17 MST 2011 
 *   Add root lib directory to PYTHONPATH.
 *
 *   Brad Whitlock, Tue Dec 13 10:10:23 PDT 2011
 *   I made the routine return all of the environment strings in a stringVector
 *   instead of calling _putenv on all of them.
 *
 *   Kathleen Biagas, Fri May 4 14:05:27 PDT 2012 
 *   Return usingdev as an arg.
 *
 *   Kathleen Biagas, Thu Nov  1 11:03:09 PDT 2012
 *   Set PYTHONHOME so cli still works if Python installed on system.
 *
 *   Kathleen Biagas, Fri Jan 6 18:30:12 MST 2017
 *   Allow user to specify their own HOME via VISITUSERHOME env var.
 *
 *   Kathleen Biagas, Tue Nov 21 13:43:42 MST 2017
 *   Display message box if VISITSSH set, but does not point
 *   to valid executable.
 *
 *    Kathleen Biagas, Thu Aug 1 13:41:32 MST 2019
 *    Removed useShorFileName argument.
 *
 *****************************************************************************/

std::string 
GetVisItEnvironment(stringVector &env, bool addPluginVars, bool &usingdev)
{
    char *tmp, *visitpath = NULL;
    char *visitdevdir = NULL;
    char tmpdir[512];
    bool haveVISITHOME = false;
    usingdev = false;
    bool freeVisItPath = true;
    string config;

    tmp = (char *)malloc(10000);

    /*
     * Determine visit path
     */
    haveVISITHOME = ReadKey("VISITHOME", &visitpath);

    if (!haveVISITHOME)
    {
        free(visitpath);
        visitpath = NULL;
        if ((visitpath = getenv("VISITHOME")) != NULL)
        {
            haveVISITHOME = true;
            freeVisItPath = false;
        }
    }

    /*
     * We could not get the value associated with the key. It may mean
     * that VisIt was not installed properly. Use a default value.
     */
    if(!haveVISITHOME)
    {
        char tmpdir[MAX_PATH];
        if (GetModuleFileName(NULL, tmpdir, MAX_PATH) != 0)
        {
            size_t pos = 0;
            size_t len = strlen(tmpdir);
            for (pos = len; tmpdir[pos] != '\\' && pos >=0; pos--)
            {
                continue;
            }
            if (pos <= 0)
                pos = len;

            visitpath = (char*)malloc(pos +1);
            strncpy(visitpath, tmpdir, pos);
            visitpath[pos] = '\0';
         }
    }
    /*
     * Determine if this is dev version
     */
    {
        string vp(visitpath);
        string tp = vp + "\\..\\" + "ThirdParty";
        struct _stat fs;
        if (_stat(tp.c_str(), &fs) == 0)
        {
            usingdev = 1;
            size_t pos;
            size_t len = strlen(visitpath);
            for (pos = len; visitpath[pos] != '\\' && pos >=0; pos--)
            {
                continue;
            }
            if (pos <= 0)
                pos = len;

            visitdevdir = (char*)malloc(pos + 14);
            strncpy(visitdevdir, visitpath, pos);
            visitdevdir[pos] = '\0';
            strncat(visitdevdir, "\\ThirdParty", 14);
            if (len != pos)
                config = vp.substr(pos+1);
        }
    }
 
    /*
     * Determine visit user path (Path to My Documents).
     */
    string userHome;
    {
        // Test for user-specified VISITUSERHOME
        string personalUserHome = WinGetEnv("VISITUSERHOME");
        if (!personalUserHome.empty())
        {
            /* User specified path, check if writeable */
            struct _stat fs;
            if (_stat(personalUserHome.c_str(), &fs) == -1)
            {
                char tmp[1024];
                SNPRINTF(tmp, 1024, "VISITUSERHOME is set in your environment"
                            " but the specified path does not exist.\n"
                            "(%s)\n"
                            "Please specify a valid path for VISITUSERHOME"
                            " before running VisIt again.\n",
                            personalUserHome.c_str());
#ifdef VISIT_WINDOWS_APPLICATION
                MessageBox(NULL, tmp, "", MB_OK);
#else
                fprintf(stderr, tmp);
#endif
                exit(0);
            }
            if (! (fs.st_mode & _S_IFDIR))
            {
                char tmp[1024];
                SNPRINTF(tmp,1024, "VISITUSERHOME is set in your environment"
                            " but the specified value is not a folder:\n"
                            "(%s)\n"
                            "Please specify a valid folder path for "
                            "VISITUSERHOME before running VisIt again.\n",
                            personalUserHome.c_str());
#ifdef VISIT_WINDOWS_APPLICATION
                MessageBox(NULL, tmp, "", MB_OK);
#else
                fprintf(stderr, tmp);
#endif
                exit(0);
            }
            userHome = personalUserHome;
            sprintf(tmpdir, "%s\\My images", personalUserHome.c_str());
            if (_stat(tmpdir, &fs) == -1)
            {
                _mkdir(tmpdir);
            }
        }
        else
        {
            char visituserpath[MAX_PATH], expvisituserpath[MAX_PATH];
            bool haveVISITUSERHOME=0;
            TCHAR szPath[MAX_PATH];
            struct _stat fs;
            if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 
                                     SHGFP_TYPE_CURRENT, szPath))) 
            {
                SNPRINTF(visituserpath, 512, "%s\\VisIt", szPath);
                haveVISITUSERHOME = true;
            }

            if (haveVISITUSERHOME)
            {
                ExpandEnvironmentStrings(visituserpath,expvisituserpath,512);
                if (_stat(expvisituserpath, &fs) == -1)
                {
                    _mkdir(expvisituserpath);
                }
            }
            else
            {
                strcpy(expvisituserpath, visitpath);
            }
            sprintf(tmpdir, "%s\\My images", expvisituserpath);
            if (_stat(tmpdir, &fs) == -1)
            {
                _mkdir(tmpdir);
            }
            userHome = expvisituserpath;
            sprintf(tmp, "VISITUSERHOME=%s", expvisituserpath);
            env.push_back(tmp);
        }
    }

    string vp(visitpath);

    /*
     * Add VisIt's home directory to the path.
     */
    env.push_back(AddPath(tmp, visitpath, visitdevdir));

    /*
     * Set the VisIt home dir.
     */
    sprintf(tmp, "VISITHOME=%s", visitpath);
    env.push_back(tmp);

    if (visitdevdir != 0)
        free(visitdevdir);

    /*
     * Set the plugin dir.
     */
    { 
        sprintf(tmp, "VISITPLUGINDIR=%s;%s", userHome.c_str(), visitpath);
        env.push_back(tmp);
        if (addPluginVars)
        {
            sprintf(tmp, "VISITPLUGININSTPRI=%s", userHome.c_str());
            env.push_back(tmp);
        }
    }

    if (addPluginVars)
    {
        sprintf(tmp, "VISITPLUGININSTPUB=%s", visitpath);
        env.push_back(tmp);
        sprintf(tmp, "VISITARCHHOME=%s", visitpath);
        env.push_back(tmp);
    }

    /*
     * Set the ultrawrapper dir.
     */
    if (!usingdev)
    {
        sprintf(tmp, "VISITULTRAHOME=%s\\ultrawrapper", visitpath);
        env.push_back(tmp);
    }
    else
    {
        sprintf(tmp, "VISITULTRAHOME=%s\\..\\ultrawrapper", visitpath);
        env.push_back(tmp);
    }

    /*
     * Set PYTHONPATH
     */
    if (!usingdev)
    {
        sprintf(tmp, "PYTHONPATH=%s\\lib", visitpath);
        env.push_back(tmp);
        sprintf(tmp, "PYTHONHOME=%s\\lib\\python",visitpath);
        env.push_back(tmp);
    }
    else 
    {
        string vp(visitpath);
        size_t pos = vp.find_last_of("\\");
        pos = vp.find_last_of("\\", pos-1);
        string svp = vp.substr(0, pos);
        if (config.length() > 0)
        {
            sprintf(tmp, "PYTHONPATH=%s\\lib\\%s;%s\\lib\\%s\\Python\\Lib", svp.c_str(), config.c_str(), svp.c_str(), config.c_str());
        }
        else
        {
            sprintf(tmp, "PYTHONPATH=%s\\lib;%s\\lib\\Python\\Lib", svp.c_str(), svp.c_str());
        }
        env.push_back(tmp);
    }

    /*
     * Set the SSH program.
     */
    { 
        char *ssh = NULL, *sshargs = NULL;
        bool needVISITSSH = false;
        bool haveSSH = false, haveSSHARGS = false, freeSSH = false, freeSSHARGS = false;
        string errmsg;

        ssh = getenv("VISITSSH");

        if (ssh != NULL)
        {
            if (!PathFileExists(ssh))
            {
                needVISITSSH = true;
                ssh = NULL;
                errmsg = string("VISITSSH env var does not point to an executable.\n");
            }
        }
        else
        {
            needVISITSSH = true;
        }
        if (ssh == NULL)
        {
            haveSSH = ReadKey("SSH", &ssh);
            if(haveSSH)
            {
                if (PathFileExists(ssh))
                {
                    sprintf(tmp, "VISITSSH=%s", ssh);
                    env.push_back(tmp);
                    errmsg.clear(); 
                }
                else
                {
                    haveSSH = false;
                    errmsg += "SSH registry key does not point to an executable.\n";
                }
            }
            free(ssh);
        }
        if (needVISITSSH && !haveSSH)
        {
            string qpath(visitpath);
            qpath+=string("\\qtssh.exe");
            sprintf(tmp, "VISITSSH=%s", qpath.c_str());
            env.push_back(tmp);
            if (!errmsg.empty())
            {
                errmsg += "Using VisIt's qtssh.";
#ifdef VISIT_WINDOWS_APPLICATION
                MessageBox(NULL, 
                           (LPCSTR)errmsg.c_str(), 
                           "", 
                           MB_ICONEXCLAMATION | MB_OK);
#else
                cerr << errmsg << endl;
#endif
            }
        }

        /*
         * Set the SSH arguments.
         */
        if((sshargs = getenv("VISITSSHARGS")) == NULL)
        {
            haveSSHARGS = ReadKey("SSHARGS", &sshargs);
            if(haveSSHARGS)
            {
                sprintf(tmp, "VISITSSHARGS=%s", sshargs);
                env.push_back(tmp);
            }
            free(sshargs);
        }
    }

    free(tmp);
    if (freeVisItPath)
        free(visitpath);
    return vp;
}

/******************************************************************************
 *
 * Purpose: Sets the VisIt environment variables from a vector of VAR=VALUE 
 *          strings.
 *
 * Programmer: Brad Whitlock
 * Date:       
 *
 * Input Arguments:
 *   env       : The vector of environment strings.
 *
 * Modifications:
 * 
 *****************************************************************************/

void
SetVisItEnvironment(const stringVector &env)
{
    for(size_t i = 0; i < env.size(); ++i)
        _putenv(env[i].c_str());
}

/******************************************************************************
 *
 * Purpose: Removes all versions of VisIt from the path and adds the current
 *          VisIt version.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb 28 12:22:31 PDT 2003
 *
 * Input Arguments:
 *   tmp       : A temporary buffer to use for string concatenation.
 *   visitpath : The path to the current version of VisIt.
 *
 * Modifications:
 *   Kathleen Bonnell, Mon Jun 2 18:11:01 PDT 2008
 *   Add 'visitdev' argument. Add it to the path if not null.
 * 
 *   Kathleen Bonnell, Sun Feb 28 16:23:45 MST 2010
 *   Add visitpath and visitdev to beginning of PATH, not end.  Ensure they
 *   don't get duplicated in the PATH string.
 * 
 *****************************************************************************/

std::string
AddPath(char *tmp, const char *visitpath, const char *visitdev)
{
    char *env = 0, *path;
    bool skiptoken;

    if (visitdev != 0)
        sprintf(tmp, "PATH=%s;%s", visitpath, visitdev);
    else
        sprintf(tmp, "PATH=%s", visitpath);

    path = tmp + strlen(tmp);

    if((env = getenv("PATH")) != NULL)
    {
       char *token, *env2;

       env2 = (char *)malloc(strlen(env) + 1);
       strcpy(env2, env);

       token = strtok( env2, ";" );
       while(token != NULL)
       {
           /* 
            * If the token does not contain "VisIt " then add it to the path.
            */
           skiptoken = false;
           if (strcmp(token, visitpath) == 0)
               skiptoken = true;
           else if (visitdev != 0 && strcmp(token, visitdev) == 0)
               skiptoken = true;
           else if(strstr(token, "VisIt ") != NULL)
               skiptoken = true;

           if (!skiptoken)
           {
               size_t len = strlen(token);
               sprintf(path, ";%s", token);
               path += (len + 1);
           }

           /* 
            * Get next token:
            */
           token = strtok( NULL, ";" );
       }

       free(env2);
    }

    return std::string(tmp);
}

void
PrintEnvironment()
{
    char *tmp;

    if((tmp = getenv("VISITHOME")) != NULL)
    {
        fprintf(stdout, "LIBPATH=%s\\lib\n", tmp);
        fprintf(stdout, "VISITHOME=%s\n", tmp);
    }
    if((tmp = getenv("VISITARCHHOME")) != NULL)
        fprintf(stdout, "VISITARCHHOME=%s\n", tmp);
    if((tmp = getenv("VISITULTRAHOME")) != NULL)
        fprintf(stdout, "VISITULTRAHOME=%s\n", tmp);
    if((tmp = getenv("VISITPLUGINDIR")) != NULL)
        fprintf(stdout, "VISITPLUGINDIR=%s\n", tmp);

}


string
WinGetEnv(const char * name)
{
    const DWORD buffSize = 65535;
    static char buffer[buffSize];
    string retval;
    if (GetEnvironmentVariableA(name, buffer, buffSize))
    {
        retval = buffer;
    }
    return retval;
}

// ****************************************************************************
// Method: main/WinMain
//
// Purpose: 
//   The program entry point function.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 23 13:15:31 PST 2011
//
// Modifications:
//   
// ****************************************************************************

#if defined(VISIT_WINDOWS_APPLICATION)
int WINAPI
WinMain(HINSTANCE hInstance,     // handle to the current instance
        HINSTANCE hPrevInstance, // handle to the previous instance    
        LPSTR lpCmdLine,         // pointer to the command line
        int nCmdShow             // show state of window
)
{
    return VisItLauncherMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return VisItLauncherMain(argc, argv);
}
#endif

