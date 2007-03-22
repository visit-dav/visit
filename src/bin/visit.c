/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <visit-config.h>
#include <windows.h>

/*
 * Macros
 */
#define ARG(A) (strcmp((A), argv[i]) == 0)

#define PUSHARG(A) componentArgs[nComponentArgs] = (char*)malloc(strlen(A)+1); \
                   strcpy(componentArgs[nComponentArgs], A); \
                   ++nComponentArgs;

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
"                             <level> must be between 1 and 5.\n";

/*
 * Prototypes
 */
char *AddEnvironment(int);
void AddPath(char *, const char *);
int ReadKey(const char *key, char **keyval);

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
 *****************************************************************************/

int
main(int argc, char *argv[])
{
    int   nComponentArgs = 0;
    char *componentArgs[100], *command = 0, *printCommand = 0, *visitpath = 0,
         *visitargs = 0, *cptr = 0, *cptr2 = 0;
    int i, size = 0, retval = 0, skipping = 0;
    int addMovieArguments = 0, addVISITARGS = 1, useShortFileName = 0;
    int newConsole = 0;

    /*
     * Default values.
     */
    char component[100];
    strcpy(component, "gui");

    /*
     * Parse the command line arguments.
     */
    for(i = 1; i < argc; ++i)
    {
        if(ARG("-help"))
        {
            printf("%s", usage);
            return 0;
        }
        else if(ARG("-version"))
        {
            printf("%s\n", VERSION);
            return 0;
        }
        else if(ARG("-gui"))
        {
            strcpy(component, "gui");
            addVISITARGS = 1;
        }
        else if(ARG("-cli"))
        {
            strcpy(component, "cli");
            addVISITARGS = 1;
        }
        else if(ARG("-viewer"))
        {
            strcpy(component, "viewer");
            addVISITARGS = 0;
        }
        else if(ARG("-mdserver"))
        {
            strcpy(component, "mdserver");
            addVISITARGS = 0;
        }
        else if(ARG("-engine"))
        {
            strcpy(component, "engine");
            addVISITARGS = 0;
        }
        else if(ARG("-vcl"))
        {
            strcpy(component, "vcl");
            addVISITARGS = 0;
        }
        else if(ARG("-movie"))
        {
            strcpy(component, "cli");
            addMovieArguments = 1;
            useShortFileName = 1;
        }
        else if(ARG("-mpeg2encode"))
        {
            strcpy(component, "mpeg2enc.exe");
            addVISITARGS = 1;
        }
        else if(ARG("-transition"))
        {
            strcpy(component, "visit_transition");
            addVISITARGS = 1;
        }
        else if(ARG("-composite"))
        {
            strcpy(component, "visit_composite");
            addVISITARGS = 1;
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
            strcpy(component, "xmledit");
            addVISITARGS = 0;
        }
        else if(ARG("-silex"))
        {
            strcpy(component, "silex");
            addVISITARGS = 0;
        }
        else if(ARG("-v"))
        {
            /* Skip the next argument too. */
            ++i;
        }
        else if(ARG("-newconsole"))
        {
            newConsole = 1;
        }
        else
        {
            char *newArg = argv[i];
            if (strstr(argv[i], " ") != NULL)
            {
                newArg = (char*)malloc(strlen(argv[i]+1+2)); /* just leak it */
                sprintf(newArg, "\"%%s\"", argv[i]);
            }

            PUSHARG(argv[i]);
        }
    }

    /* If we want a new console, allocate it now. */
    if(newConsole)
    {
        FreeConsole();
        AllocConsole();
    }

    /*
     * Add some stuff to the environment.
     */
    visitpath = AddEnvironment(useShortFileName);

    /*
     * Get additional VisIt arguments.
     */
    if(addVISITARGS)
    {
        if(ReadKey("VISITARGS", &visitargs) == 0)
        {
            addVISITARGS = 0;
            visitargs = 0;
        }
    }

    /*
     * Figure out the length of the command string.
     */
    size = strlen(visitpath) + strlen(component) + 4;
    for(i = 0; i < nComponentArgs; ++i)
        size += (strlen(componentArgs[i]) + 1);
    if(addMovieArguments)
    {
        size += strlen("-s") + 1;
        size += 1 + strlen(visitpath) + 1 + strlen("makemoviemain.py") + 2;
        size += strlen("-nowin") + 1;
    }
    if(addVISITARGS)
        size += (strlen(visitargs) + 1);

    /*
     * Create the command to execute and the string that we print.
     */
    command = (char *)malloc(size);
    memset(command, 0, size);
    printCommand = (char *)malloc(size);
    memset(printCommand, 0, size);
    if(useShortFileName)
    {
        sprintf(command, "%s\\%s", visitpath, component);
        sprintf(printCommand, "%s\\%s", visitpath, component);
    }
    else
    {
        sprintf(command, "\"%s\\%s\"", visitpath, component);
        sprintf(printCommand, "\"%s\\%s\"", visitpath, component);
    }
    cptr = command + strlen(command);
    cptr2 = printCommand + strlen(printCommand);
    for(i = 0; i < nComponentArgs; ++i)
    {
        sprintf(cptr, " %s", componentArgs[i]);
        cptr += (strlen(componentArgs[i]) + 1);

        if(strcmp(componentArgs[i], "-key") == 0)
            skipping = 1;

        if(skipping == 0) 
        {
            sprintf(cptr2, " %s", componentArgs[i]);
            cptr2 += (strlen(componentArgs[i]) + 1);
        }
    }
    if(addVISITARGS)
    {
        sprintf(cptr, " %s", visitargs);
        cptr += (strlen(visitargs) + 1);
        sprintf(cptr2, " %s", visitargs);
        cptr2 += (strlen(visitargs) + 1);
    }
    if(addMovieArguments)
    {
        sprintf(cptr, " -s \"%s\\makemoviemain.py\" -nowin", visitpath);
        sprintf(cptr2, " -s \"%s\\makemoviemain.py\" -nowin", visitpath);
    }
    command[size-1] = '\0';
    printCommand[size-1] = '\0';

    /*
     * Print the run information.
     */
    fprintf(stderr, "Running: %s\n", printCommand);
    fflush(stderr);

    /*
     * Launch the appropriate VisIt component.
     */
    retval = system(command);

    /*
     * Free memory
     */
    free(command);
    for(i = 0; i < nComponentArgs; ++i)
        free(componentArgs[i]);
    free(visitpath);
    if(visitargs != 0)
        free(visitargs);

    return retval;
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
 *
 *****************************************************************************/

int
ReadKeyFromRoot(HKEY which_root, const char *key, char **keyval)
{
    int  readSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", VERSION);
    *keyval = (char *)malloc(500);
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD keyType, strSize = 500;
        if(RegQueryValueEx(hkey, key, NULL, &keyType, *keyval, &strSize) == ERROR_SUCCESS)
        {
            readSuccess = 1;
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
 *****************************************************************************/

int
ReadKey(const char *key, char **keyval)
{
    int retval = 0;

    if((retval = ReadKeyFromRoot(HKEY_CLASSES_ROOT, key, keyval)) == 0)
        retval = ReadKeyFromRoot(HKEY_CURRENT_USER, key, keyval);
    
    return retval;     
}

/******************************************************************************
 *
 * Purpose: Adds information needed to run VisIt to the environment.
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
 *****************************************************************************/

char *
AddEnvironment(int useShortFileName)
{
    char *tmp, *visitpath = 0, *ssh = 0, *sshargs = 0, *visitsystemconfig = 0;
    int haveVISITHOME = 0, haveSSH = 0, haveSSHARGS = 0,
        haveVISITSYSTEMCONFIG = 0;

    /* Try and read values from the registry. */
    haveVISITHOME         = ReadKey("VISITHOME", &visitpath);
    haveSSH               = ReadKey("SSH", &ssh);
    haveSSHARGS           = ReadKey("SSHARGS", &sshargs);
    haveVISITSYSTEMCONFIG = ReadKey("VISITSYSTEMCONFIG", &visitsystemconfig);

    /* We could not get the value associated with the key. It may mean
     * that VisIt was not installed properly. Use a default value.
     */
    if(!haveVISITHOME)
    {
        int haveVISITDEVDIR = 0;
        char *visitdevdir = 0;
        haveVISITDEVDIR = ReadKey("VISITDEVDIR", &visitdevdir);

        if(haveVISITDEVDIR)
        {
#ifdef USING_MSVC6
#if defined(_DEBUG)
            static const char *configDir = "\\bin\\Debug";
#else
            static const char *configDir = "\\bin\\Release";
#endif
#else
            /* The location of the binaries are different for MSVC7.Net */
#if defined(_DEBUG)
            static const char *configDir = "\\bin\\MSVC7.Net\\Debug";
#else
            static const char *configDir = "\\bin\\MSVC7.Net\\Release";
#endif
#endif
            visitpath = (char *)malloc(strlen(visitdevdir) + strlen(configDir) + 1);
            sprintf(visitpath, "%s%s", visitdevdir, configDir);
        }
        else
        {
            char tmpdir[512];
#ifdef USING_MSVC6
#if defined(_DEBUG)
            sprintf(tmpdir, "C:\\VisItDev%s\\bin\\Debug", VERSION);
#else
            sprintf(tmpdir, "C:\\VisItDev%s\\bin\\Release", VERSION);
#endif
#else
            /* The location of the binaries are different for MSVC7.Net */
#if defined(_DEBUG)
            sprintf(tmpdir, "C:\\VisItDev%s\\bin\\MSVC7.Net\\Debug", VERSION);
#else
            sprintf(tmpdir, "C:\\VisItDev%s\\bin\\MSVC7.Net\\Release", VERSION);
#endif
#endif
            visitpath = (char *)malloc(strlen(tmpdir) + 1);
            strcpy(visitpath, tmpdir);
        }
    }

    /* Turn the long VisIt path into the shortened system path. */
    if(useShortFileName)
    {
        char *vp2 = (char *)malloc(512);
        GetShortPathName(visitpath, vp2, 512);
        free(visitpath);
        visitpath = vp2;
    }

    tmp = (char *)malloc(10000);

    /* Add VisIt's home directory to the path */
    AddPath(tmp, visitpath);

    /* Set the VisIt home dir. */
    sprintf(tmp, "VISITHOME=%s", visitpath);
    putenv(tmp);

    /* Set the plugin dir. */
    sprintf(tmp, "VISITPLUGINDIR=%s", visitpath);
    putenv(tmp);

    /* Set the help dir. */
    sprintf(tmp, "VISITHELPHOME=%s\\help", visitpath);
    putenv(tmp);

    /* Set the SSH program. */
    if(haveSSH)
    {
        sprintf(tmp, "VISITSSH=%s", ssh);
        putenv(tmp);
    }
    else
    {
        sprintf(tmp, "VISITSSH=%s\\qtssh.exe", visitpath);
        putenv(tmp);
    }

    /* Set the SSH arguments. */
    if(haveSSHARGS)
    {
        sprintf(tmp, "VISITSSHARGS=%s", sshargs);
        putenv(tmp);
    }

    /* Set the system config variable. */
    if(haveVISITSYSTEMCONFIG)
    {
        sprintf(tmp, "VISITSYSTEMCONFIG=%s", visitsystemconfig);
        putenv(tmp);
    }

    free(tmp);
    free(ssh);
    free(sshargs);
    free(visitsystemconfig);

    return visitpath;
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
 *
 *****************************************************************************/

void
AddPath(char *tmp, const char *visitpath)
{
    char *env = 0, *path, *start = tmp;

    strcpy(tmp, "PATH=");
    start = path = tmp + 5;

    if((env = getenv("PATH")) != NULL)
    {
       char *token, *env2;

       env2 = (char *)malloc(strlen(env) + 1);
       strcpy(env2, env);

       token = strtok( env2, ";" );
       while(token != NULL)
       {
           /* If the token does not contain "VisIt " then add it to the path. */
           if(strstr(token, "VisIt ") == NULL)
           {
               int len = strlen(token);
               if(path == start)
               {
                   sprintf(path, "%s", token);
                   path += len;
               }
               else
               {
                   sprintf(path, ";%s", token);
                   path += (len + 1);
               }
           }

           /* Get next token: */
           token = strtok( NULL, ";" );
       }

       free(env2);
    }

    if(path == start)
        sprintf(path, "%s", visitpath);
    else
        sprintf(path, ";%s", visitpath);

    putenv(tmp);
}
