/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#define SQUOTEARG(A) (tmpArg = string("\'") + string(A) + string("\'"))


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
"        -public              xml2cmake: force install plugins publicly\n"
"        -private             xml2cmake: force install plugins privately\n"
"        -clobber             Permit xml2... tools to overwrite old files\n"
"        -noprint             Silence debugging output from xml2... tools\n"
"        -env                 Print environment strings set up by the launcher\n"
"\n";

/*
 * Prototypes
 */
string GetVisItEnvironment(stringVector &, bool, bool, bool &);

void   SetVisItEnvironment(const stringVector &);
string AddPath(char *, const char *, const char*);
bool   ReadKey(const char *key, char **keyval);
void   TestForConfigFiles(const string &component);
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
 *   Skip over arguments that end with 'visit', 'visit.exe', 'visit"', 'visit.exe"'
 *   since we're starting the argv iteration at 0, which means we'll pick up
 *   the visit.exe program.
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
 *****************************************************************************/

int
VisItLauncherMain(int argc, char *argv[])
{
    bool addMovieArguments = false; 
    bool addVISITARGS = true; 
    bool useShortFileName = false;
    bool addPluginVars = false;
    bool newConsole = false;
    bool noloopback = false;
    bool parallel = false;
    bool hostset = 0;
    bool envOnly = false;
    bool debugLaunch = false;

    string nps("2");
    stringVector componentArgs;
    stringVector engineArgs;
    string component("gui");
    string tmpArg;
    

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
            useShortFileName = true;
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
        else if(ARG("-env"))
        {
            envOnly = true;
        }
        else if(ARG("-debuglaunch"))
        {
            debugLaunch = true;
            componentArgs.push_back("-debuglaunch");
        }
        else
        {
            if (!BEGINSWITHQUOTE(argv[i]) && HASSPACE(argv[i]))
            {
                SQUOTEARG(argv[i]); 
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
    string visitpath = GetVisItEnvironment(visitEnv, useShortFileName, addPluginVars, usingDev);
    if (usingDev)
        componentArgs.push_back("-dv");
    SetVisItEnvironment(visitEnv);
#ifdef VISIT_WINDOWS_APPLICATION
    // Show the path and the environment we've created.
    string envStr;
    for(size_t i = 0; i < visitEnv.size(); ++i)
        envStr = envStr + visitEnv[i] + "\n";
    string msgStr((visitpath + "\n\n") + envStr);
    if(debugLaunch)
        MessageBox(NULL, msgStr.c_str(), component.c_str(), MB_OK);
#endif

    if (envOnly)
    {
        PrintEnvironment();
        componentArgs.clear();
        return 1;
    }

    //
    // Migrate config files 
    // 
    if (!usingDev && (component == "gui"  || component == "cli"))
    {
        TestForConfigFiles(component);
    }
    
    stringVector command;
    command.reserve(20);
    string quote("\"");
    if (component == "engine_par")
    {
        // first look for an EnvVar set by the installer
        string mpipath = WinGetEnv("VISIT_MPIEXEC");
        if (mpipath.empty())
        {
            // Check for EnvVar set by MSMPI R2 redist installer
            mpipath = WinGetEnv("MSMPI_INC");
            if (!mpipath.empty())
            {
                // found the include path, point to Bin
                mpipath += "\\..\\Bin\\mpiexec.exe";
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
            }
        }
        if (mpipath.empty())
        {
            cerr << "Could not find path to \"mpiexec\"" << endl;
            cerr << "If it is installed on your system, please set "
                 << "an environment variable 'VISIT_MPIEXEC' that points"
                 << "to its location." << endl;
            return -1;
        }

        char *shortmpipath = (char*)malloc(512);
        GetShortPathName(mpipath.c_str(), shortmpipath, 512);

        // mpi exec, with shortpath 
        command.push_back(shortmpipath);
        free(shortmpipath);
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

        if(program.find(" ") != std::string::npos)
        {
            char *shortname = (char*)malloc(512);
            GetShortPathName(program.c_str(), shortname, 512);
            program = shortname;
            free(shortname);
        }

        command.push_back(program);
    }

    for(size_t i = 0; i < componentArgs.size(); ++i)
    {
        if((componentArgs[i] == "-host") && !noloopback)
        {
            // Replace the host arg with the loopback
            componentArgs[i+1] = "127.0.0.1"; 
        }

        if (!BEGINSWITHQUOTE(componentArgs[i].c_str()) && 
            HASSPACE(componentArgs[i].c_str()))
        {
            SQUOTEARG(componentArgs[i].c_str());
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
            eArgs.append(";");
            eArgs.append(engineArgs[i]);
        }
        command.push_back(eArgs);
    }
    if(addMovieArguments)
    {
        command.push_back("-s");
        command.push_back(quote + visitpath + string("\\makemoviemain.py") + quote);
        command.push_back("-nowin");
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
    string cmdLine(command[0]);
    for(size_t i = 1; i < command.size(); ++i)
        cmdLine.append(string(" ") + command[i]);
    cerr << "Running: " << cmdLine << endl;

#ifdef VISIT_WINDOWS_APPLICATION
    if(debugLaunch)
        MessageBox(NULL, cmdLine.c_str(), component.c_str(), MB_OK);

    // We can't use system() since that opens a cmd shell.
    const char *exeName = command[0].c_str();
    const char **exeArgs = new const char *[command.size()+1];
    for(size_t i = 0; i < command.size(); ++i)
        exeArgs[i] = command[i].c_str();
    exeArgs[command.size()] = NULL;
    _spawnv( _P_WAIT, exeName, exeArgs);
    delete [] exeArgs;
#else
    char *cl = const_cast<char*>(cmdLine.c_str());
    system(cl);
#endif

    componentArgs.clear();
    return 0;
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
 *****************************************************************************/

std::string 
GetVisItEnvironment(stringVector &env, bool useShortFileName, bool addPluginVars, bool &usingdev)
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
            int pos = 0;
            int len = strlen(tmpdir);
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
    {
        char visituserpath[MAX_PATH], expvisituserpath[MAX_PATH];
        bool haveVISITUSERHOME=0;
        TCHAR szPath[MAX_PATH];
        struct _stat fs;
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 
                                 SHGFP_TYPE_CURRENT, szPath))) 
        {
            SNPRINTF(visituserpath, 512, "%s\\VisIt %s", szPath, VISIT_VERSION);
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
        sprintf(tmp, "VISITUSERHOME=%s", expvisituserpath);
        env.push_back(tmp);
    }

    /* 
     * Turn the long VisIt path into the shortened system path.
     */
    if(useShortFileName)
    {
        char *vp2 = (char *)malloc(512);
        GetShortPathName(visitpath, vp2, 512);
        if (freeVisItPath)
            free(visitpath);
        visitpath = vp2;
        freeVisItPath = true;
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
        char appData[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL,
                                          SHGFP_TYPE_CURRENT, appData)))
        {
            PathAppend(appData, "LLNL");
            PathAppend(appData, "VisIt");
            sprintf(tmp, "VISITPLUGINDIR=%s;%s", appData, visitpath);
            env.push_back(tmp);
            if (addPluginVars)
            {
                sprintf(tmp, "VISITPLUGININSTPRI=%s", appData);
                env.push_back(tmp);
            }
        }
        else
        {
            sprintf(tmp, "VISITPLUGINDIR=%s", visitpath);
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
     * Set the help dir.
     */
    if (!usingdev)
    {
        sprintf(tmp, "VISITHELPHOME=%s\\help", visitpath);
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
        sprintf(tmp, "PYTHONPATH=%s\\lib;%s\\lib\\Python\\lib", 
                visitpath, visitpath);
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
        int haveSSH = 0, haveSSHARGS = 0, freeSSH = 0, freeSSHARGS = 0;
        
        if((ssh = getenv("VISITSSH")) == NULL)
        {
            haveSSH = ReadKey("SSH", &ssh);
            if(haveSSH)
            {
                sprintf(tmp, "VISITSSH=%s", ssh);
                env.push_back(tmp);
            }
            else
            {
                sprintf(tmp, "VISITSSH=%s\\qtssh.exe", visitpath);
                env.push_back(tmp);
            }
            free(ssh);
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

    /*
     * Set the system config variable.
     */
    {
        char *visitsystemconfig = NULL;
        int haveVISITSYSTEMCONFIG = 0;
        haveVISITSYSTEMCONFIG = ReadKey("VISITSYSTEMCONFIG",
                                        &visitsystemconfig);
        if(haveVISITSYSTEMCONFIG)
        {
            sprintf(tmp, "VISITSYSTEMCONFIG=%s", visitsystemconfig);
            env.push_back(tmp);
        }
        free(visitsystemconfig);
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
               int len = strlen(token);
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



/******************************************************************************
 *  The following were extracted from VIKit, because it was determined that
 *  that migrating config-files should be done on a per-user basis upon first
 *  run of a new VisIt version, rather than at install-time.
******************************************************************************/

/******************************************************************************
 *
 * Purpose: Reads a directory and calls a callback function for each .ini
 *          or session file in the directory.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jul 8 14:35:57 PST 2005
 *
 * Input Arguments:
 *   path   : The path to the directory where we should read files.
 *   cb     : The callback function to call.
 *   cbData : The callback function data.
 *
 * Modifications:
 *
 *****************************************************************************/

static void
HandleConfigFiles(const char *path, void (*cb)(const char *, void *), 
                  void *cbData)
{
    int pathlen = 0;
    WIN32_FIND_DATA fd;
    HANDLE dirHandle = INVALID_HANDLE_VALUE;
    char path2[1024], search[1024];    
    FILE *log = NULL;
  

    strcpy(path2, path);
    pathlen = strlen(path2);
    if(path2[pathlen-1] != '\\')
    {
        path2[pathlen] = '\\';
        path2[++pathlen] = '\0';
    }
    strcpy(search, path2);
    search[pathlen] = '*';
    search[pathlen+1] = '\0';

    log = (FILE *)(((void **)cbData)[0]);
    if(log != NULL)
       fprintf(log, "search = \"%s\"\n", search);

    /* List the files in the old path and for each one of them that starts
       with "config ", copy the file to the new directory.
     */
    dirHandle = FindFirstFile(search, &fd);
    if(dirHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            int len = strlen(fd.cFileName);
            char *ini = fd.cFileName + len-4;
            char *vses = fd.cFileName + len-5;
            char *vsesgui = fd.cFileName + len-5-4;
            char *session = fd.cFileName + len-8;
            char *sessiongui = fd.cFileName + len-8-4;

            if(log != NULL)
            {
                fprintf(log, "\t%s\n", fd.cFileName);
                if(ini >= fd.cFileName)     fprintf(log, "\t\t%s\n", ini);
                if(vses >= fd.cFileName)    fprintf(log, "\t\t%s\n", vses);
                if(vsesgui >= fd.cFileName) fprintf(log, "\t\t%s\n", vsesgui);
                if(session >= fd.cFileName)    fprintf(log, "\t\t%s\n", session);
                if(sessiongui >= fd.cFileName) fprintf(log, "\t\t%s\n", sessiongui);
            }

            if((ini >= fd.cFileName && _stricmp(ini, ".ini") == 0)   ||
              (vses >= fd.cFileName && _stricmp(vses, ".vses") == 0) ||
             (vsesgui >= fd.cFileName && _stricmp(vsesgui, ".vses.gui") == 0) ||
             (session >= fd.cFileName && _stricmp(session, ".session") == 0) ||
             (sessiongui >= fd.cFileName && _stricmp(sessiongui, ".session.gui") == 0)
              )
            {
                if(_strnicmp(fd.cFileName, "visit-config", 12) == 0)
                {
                    /* We want to skip visit-config files because those get
                     * updated each version and copying over them would be
                     * bad.
                     */
                    if(log != NULL)
                        fprintf(log, "Skipping %s\n", fd.cFileName);
                }
                else
                    (*cb)(fd.cFileName, cbData);
            }
        } while(FindNextFile(dirHandle, &fd));
        FindClose(dirHandle);
    }
    else
    {
       if(log != NULL)
           fprintf(log, "INVALID_HANDLE!\n");
    }
}

/******************************************************************************
 *
 * Purpose: Reads a directory and returns 1 if there are config files in it
 *          or 0 if there are no config files.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jul 8 14:35:57 PST 2005
 *
 * Modifications:
 *
 *****************************************************************************/

static void
has_config_files(const char *filename, void *cbData)
{
    void **cbd = (void**)cbData;
    FILE *log = (FILE *)(cbd[0]);
    int *ival = (int *)(cbd[1]);
    
    if(log != NULL)
        fprintf(log, "%s\n", filename);

    *ival = 1;
}

static int
HasConfigFiles(const char *path, FILE *log)
{
    int  retval = 0;
    void *cbData[2];
    cbData[0] = (void *)log;
    cbData[1] = (void *)&retval;

    HandleConfigFiles(path, has_config_files, (void *)cbData);

    return retval;
}

/******************************************************************************
 *
 * Purpose: Copies config files from 1 directory to another directory.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jul 8 14:35:57 PST 2005
 *
 * Modifications:
 *   Kathleen Bonnell, Tue Oct 7 16:19:07 PDT 2008
 *   Stat newPath and create it if necessary.
 *
 *****************************************************************************/

static void
copy_config_files(const char *filename, void *cbData)
{
    char oldFile[1024], toFile[1024];
    const char *oldpath = 0;
    const char *newpath = 0;
    int len = 0;
    struct _stat fs;

    FILE *log = (FILE *)(((void **)(cbData))[0]);
    oldpath = (const char *)(((void **)(cbData))[1]);
    newpath = (const char *)(((void **)(cbData))[2]);

    if (_stat(newpath, &fs) == -1)
    {
        _mkdir(newpath);
    }

    len = strlen(oldpath);
    if(oldpath[len-1] == '\\')
        _snprintf(oldFile, 1024, "%s%s",  oldpath, filename);
    else
        _snprintf(oldFile, 1024, "%s\\%s",  oldpath, filename);

    len = strlen(newpath);
    if(newpath[len-1] == '\\')
        _snprintf(toFile, 1024, "%s%s",  newpath, filename);
    else
        _snprintf(toFile, 1024, "%s\\%s",  newpath, filename);

    if(log != NULL)
        fprintf(log, "copy file %s to %s\n", oldFile, toFile);

    CopyFile(oldFile, toFile, FALSE);
}

static void
CopyConfigFiles(const char *oldpath, const char *newpath, FILE *log)
{
    void *cbData[3];
    cbData[0] = (void *)log;
    cbData[1] = (void *)oldpath;
    cbData[2] = (void *)newpath;

    HandleConfigFiles(oldpath, copy_config_files, (void *)cbData);
}

/******************************************************************************
 *
 * Purpose: compares two VisIt versions of the form major.minor.patch, 
 *          stored as ints in a std vector.
 *
 * Returns:  -1 if v1 < v2
 *            0 if v1 == v2
 *            1 if v1 > v2
 *
 * Programmer: Kathleen Bonnell 
 * Date:       October 8, 2008
 *
 * Modifications:
 *
 *****************************************************************************/

int 
compareVersions(const intVector &v1, const intVector &v2)
{
    if (v1[0] == v2[0]) /* major */
    {
        if (v1[1] == v2[1]) /*minor */
        {
            if (v1[2] == v2[2]) /*patch */
            {
                return 0;
            }
            else if (v1[2] < v2[2])
            {
                return -1;
            }
            else 
            {
                return 1;
            }
        }
        else if (v1[1] < v2[1])
        {
            return -1;
        }
        else 
        {
            return 1;
        }
    }
    else if (v1[0] < v2[0])
    {
        return -1;
    }
    else 
    {
        return 1;
    }
}


/******************************************************************************
 *
 * Purpose: sorts (descending) a list (vector) of VisIt versions of the form 
 *          major.minor.patch, stored as ints in a std vector.
 *
 * Programmer: Kathleen Bonnell 
 * Date:       October 8, 2008
 *
 * Modifications:
 *
 *****************************************************************************/


void
sortVersions(intIntVector &versions)
{
    intVector temp;
    for (size_t i = 0; i < versions.size() -1; ++i)
        for (size_t j = i+1; j < versions.size(); ++j)
    {
        if (compareVersions(versions[i], versions[j]) < 0)
        {
            temp = versions[i];
            versions[i] = versions[j];
            versions[j] = temp;
        } 
    }
}


/******************************************************************************
 *
 * Purpose: Searches the passed directory for folders representing versions of
 *          VisIt ("VisIt 1.9.1", "VisIt 1.10.0" etc).
 *
 * Returns: A list (vector) of VisIt version numbers.
 *
 * Programmer: Kathleen Bonnell 
 * Date:       October 8, 2008
 *
 * Modifications:
 *
 *****************************************************************************/

intIntVector
FindOldVisItVersions(const char *basePath, FILE *log)
{
    char search[1024];
    WIN32_FIND_DATA fd;
    HANDLE dirHandle = INVALID_HANDLE_VALUE;

    sprintf(search, "%s\\*", basePath);
    dirHandle = FindFirstFile(search, &fd);
    intIntVector versions;

    if(dirHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            char *visit = strstr(fd.cFileName, "VisIt ");
            if (visit != NULL)
            {
                intVector v(3);
                if(log != NULL)
                {
                    fprintf(log, " \tfound possibility at: %s\n", visit);
                }
                if (sscanf(visit+6, "%d.%d.%d", &v[0], &v[1], &v[2]) == 3)
                    versions.push_back(v);
            }
        } while(FindNextFile(dirHandle, &fd));
        FindClose(dirHandle);
    }
    else
    {
       if(log != NULL)
           fprintf(log, "INVALID_HANDLE!\n");
    }
    if (versions.size() > 0)
        sortVersions(versions);
    return versions;
}

/******************************************************************************
 *
 * Purpose: This function is a custom extension for the NSIS installer that
 *          allows it to determine the path to the most recent older config
 *          files from previously installed versions of VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Jun 6 16:55:54 PST 2005
 *
 * Precondition:
 *   The string containing the current version of VisIt and a string 
 *   containing the path to the user's 'My Documents' folder is to be passed 
 *   on the stack.
 *
 * Postcondition:
 *   2 values are returned on the stack. The top value is the last version of
 *   VisIt to have config files that we want to copy. 
 *   The value at top-1 in the stack is a message box prompt that is
 *   populated if we found config files.
 *
 *   If no config files were found for any version of VisIt, 2 blank strings
 *   are pushed onto the stack.
 *
 * Modifications:
 *   Kathleen Bonnell, Tue Oct 7 16:28:43 PDT 2008
 *   Modified pre and post conditions to reflect that 'My Documents' is now
 *   the place to store user config files (not install dir).  Create a list of
 *   VisIt versions present in My Documents, rather than storing versions in a 
 *   table which needs to be updated frequently as new versions are created.
 *
 *   Kathleen Bonnell, Fri Nov 7 15:50:12 PST 2008
 *   Retrieve users' home dir for location of log file.  Precondition is now
 *   only currentVersion.  basePath retrieved as user's home dir. Post
 *   condition is <top> msg, ver, basePath if old configs found, <top> "" 
 *   if no config files found.
 *
 *****************************************************************************/

int 
GetPathToOlderConfigFiles(const char *basePath, 
    const char *currentVersion, char *oldVersion, char *msg)
{
    int  found = 0;
    char oldPath[2048];
    FILE *log = NULL;

#ifdef DEBUG_GetPathToOlderConfigFiles
    string logFile = string(basePath) + string("\\VIkit_GetPathToOlderConfigFiles.txt"); 
    log = fopen(logFile.c_str(), "wt");
    if(log != NULL)
    {
        fprintf(log, "currentVersion = \"%s\"\n", currentVersion);
        fprintf(log, "basePath = \"%s\"\n", basePath);
    }
#endif

    intVector cV(3);
    sscanf(currentVersion, "%d.%d.%d", &cV[0], &cV[1], &cV[2]);

    intIntVector versions = FindOldVisItVersions(basePath, log);
    
    /* Look through the versions until we find one that is installed 
       that is not the current version. If we find one then push its
       installation directory onto the stack.
     */

    char ver[100];

    for(size_t i = 0; i < versions.size() && !found; ++i)
    {
        sprintf(ver, "%d.%d.%d", versions[i][0], versions[i][1], versions[i][2]);
        if(log != NULL)
        {
            fprintf(log, "ver = \"%s\"\n", ver);
        }
        if(compareVersions(versions[i], cV) < 0)
        {
            sprintf(oldPath, "%s\\VisIt %s\\", basePath, ver); 
            found = HasConfigFiles(oldPath, log);
            
            if(found)
            {
                _snprintf(msg, 1024, "There are config files from VisIt %s.\n"
                    "Would you like to copy them to your new VisIt "
                    "installation? (y/n)", ver);
            
                _snprintf(oldVersion, 512, ver);

                if(log != NULL)
                    fprintf(log, "msg=%s\noldPath=%s\n", msg, oldPath);
                break;
            }
        }
    }

#ifdef DEBUG_GetPathToOlderConfigFiles
    if(!found)
    {
        if(log != NULL)
            fprintf(log, "No config files from older versions!\n");
    }

    if(log != NULL)
        fclose(log);
#endif
    return found;
}


/******************************************************************************
 *
 * Purpose: This function is a custom extension for the NSIS installer that
 *          allows it to determine the path to the most recent older config
 *          files from previously installed versions of VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Jun 6 16:55:54 PST 2005
 *
 * Precondition:
 *   The string containing the path to the 'My Documents' folder must be on
 *   the top of the stack.
 *   The string containing the old version of VisIt from which to copy config
 *   files from must be on the top of the stack -1. 
 *   The string containing the current version of VisIt must be on the top 
 *   of the stack -2. 
 *
 * Postcondition:
 *   The values are popped from the stack and config files are copied.
 *
 * Modifications:
 *   Kathleen Bonnell, Tue Oct 7 16:28:43 PDT 2008
 *   Modified pre and post conditions to reflect that 'My Documents' is now
 *   the place to store user config files (not install dir).
 *
 *   Kathleen Bonnell, Fri Nov 7 15:54:21 PST 2008 
 *   Preconditions are now: <top> currentVersion, oldVersion, basePath 
 * 
 *****************************************************************************/

void 
MigrateConfigFiles(const char *basePath, char *currentVersion, char* oldVersion)
{
    char newPath[1024];
    char oldPath[1024];
    FILE *log = NULL;


    sprintf(oldPath, "%s\\VisIt %s\\", basePath, oldVersion);
    sprintf(newPath, "%s\\VisIt %s\\", basePath, currentVersion);
#ifdef DEBUG_MigrateConfigFiles
    string logFile = basePath + "\\VIkit_MigrateConfigFiles.txt"; 
    log = fopen(logFile.c_str(), "wt");
    if(log != NULL)
    {
        fprintf(log, "basePath = \"%s\"\n", basePath);
        fprintf(log, "oldPath = \"%s\"\n", oldPath);
        fprintf(log, "newPath = \"%s\"\n", newPath);
    }
#endif

    /* Copy the config files. */
    if(strlen(oldPath) > 0)
        CopyConfigFiles(oldPath, newPath, log);

#ifdef DEBUG_MigrateConfigFiles
    if(log != NULL)
        fclose(log);
#endif
}

/******************************************************************************
 *
 * Purpose: Finds the users' home directory.
 *
 * Programmer: Kathleen Bonnell 
 * Date:       November 6, 2008
 *
 * Modifications:
 *   Kathleen Bonnell, Fri Nov 7 15:48:57 PST 2008
 *   GetUserProfileDirectory requires user name, so use SHGetFolderPath 
 *   instead.
 *
 *****************************************************************************/
string
GetUserHomeDir()
{
    char *profPath = new char[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 
                                  SHGFP_TYPE_CURRENT, profPath)))
        return string(profPath);
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 
                                  SHGFP_TYPE_CURRENT, profPath)))
        return string(profPath);
    return string("C:\\");
}

/******************************************************************************
 *  End extraction from VIKit. 
******************************************************************************/

/******************************************************************************
 * TestForConfigFiles
 *
 * Purpose: If this is the first run of VisIt, determine if config files for
 *          older versions of VisIt exist, and prompt user if they want
 *          to copy them to current version.
 *
 * Programmer: Kathleen Bonnell 
 * Date:       March 17, 2009 
 *
 * Modifications:
 *
 *****************************************************************************/
#include <io.h>

void
TestForConfigFiles(const string &component)
{
    FILE *f = NULL;
    char *visItUserHome = getenv("VISITUSERHOME");
    char rcNamegui[512];
    char rcNamecli[512];
    // Has this version of visit already been run and this question asked? 
    sprintf(rcNamegui, "%s\\state%s.txt", visItUserHome, VISIT_VERSION);
    sprintf(rcNamecli, "%s\\clistate%s.txt", visItUserHome, VISIT_VERSION);

    if ((_access(rcNamegui, 0) == 0) ||  (_access(rcNamecli, 0) == 0))
    {
        return;
    }

    // do we already have config files for the current version?
    if (HasConfigFiles(visItUserHome, NULL))
    {
        return;
    }

    char oldVersion[512];
    char msg[512];

    string userHome = GetUserHomeDir();
    if (GetPathToOlderConfigFiles(userHome.c_str(), VISIT_VERSION, oldVersion, msg))
    {
        if (component == "gui")
        {
            int msgboxID = MessageBox(NULL, msg, "Migrate Config Files",
                                       MB_ICONEXCLAMATION | MB_YESNO);

            if (msgboxID == IDYES)
            {
                MigrateConfigFiles(userHome.c_str(), VISIT_VERSION, oldVersion);
            }
        }
        else
        {
            std::cout << msg;
            char copyConfigs;
            std::cin >> copyConfigs;
            if (copyConfigs == 'y' || copyConfigs == 'Y')
            {
                MigrateConfigFiles(userHome.c_str(), VISIT_VERSION, oldVersion);
            }
            // Create the clistate file.  Cannot use state.txt, as that is
            // needed by the gui to determine whether or not release notes
            // should be displayed.
            f = fopen(rcNamecli, "w"); 
            fprintf(f, "1\n");
            fclose(f);
        }
    }
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
    if((tmp = getenv("VISITHELPHOME")) != NULL)
        fprintf(stdout, "VISITHELPHOME=%s\n", tmp);
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

