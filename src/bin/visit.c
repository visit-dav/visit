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
"\n"
"    Window arguments:\n"
"        -small               Use a smaller desktop area/window size\n"
"        -geometry   <spec>   What portion of the screen to use.  This is a\n"
"                                 standard X Windows geometry specification\n"
"        -style      <style>  One of: windows,cde,motif,sgi\n"
"        -background <color>  Background color for GUI\n"
"        -foreground <color>  Foreground color for GUI\n"
"        -nowin               Run without viewer windows\n"
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
 *****************************************************************************/

int
main(int argc, char *argv[])
{
    int   nComponentArgs = 0;
    char *componentArgs[100], *command, *printCommand, *visitpath, *cptr, *cptr2;
    int i, size = 0, retval = 0, printRunInfo = 1, skipping = 0;
    int addMovieArguments = 0, useShortFileName = 0;

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
        }
        else if(ARG("-cli"))
        {
            strcpy(component, "cli");
        }
        else if(ARG("-viewer"))
        {
            strcpy(component, "viewer");
        }
        else if(ARG("-mdserver"))
        {
            strcpy(component, "mdserver");
        }
        else if(ARG("-engine"))
        {
            strcpy(component, "engine");
        }
        else if(ARG("-vcl"))
        {
            strcpy(component, "vcl");
        }
        else if(ARG("-movie"))
        {
            strcpy(component, "cli");
            addMovieArguments = 1;
            useShortFileName = 1;
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
        }
        else if(ARG("-silex"))
        {
            strcpy(component, "silex");
        }
        else if(ARG("-v"))
        {
            /* Skip the next argument too. */
            ++i;
        }
        else
        {
            PUSHARG(argv[i]);
        }
    }

    /*
     * Add some stuff to the environment.
     */
    visitpath = AddEnvironment(useShortFileName);

    /*
     * Figure out the length of the command string.
     */
    size = strlen(visitpath) + strlen(component) + 4;
    for(i = 0; i < nComponentArgs; ++i)
        size += (strlen(componentArgs[i]) + 1);
    if(addMovieArguments)
    {
        size += strlen("-s") + 1;
        size += 1 + strlen(visitpath) + 1 + strlen("makemovie.py") + 2;
        size += strlen("-nowin") + 1;
    }

    /*
     * Create the command to execute and the string that we print.
     */
    command = (char *)malloc(size);
    printCommand = (char *)malloc(size);
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
    if(addMovieArguments)
    {
        sprintf(cptr, " -s \"%s\\makemovie.py\" -nowin", visitpath);
        sprintf(cptr2, " -s \"%s\\makemovie.py\" -nowin", visitpath);
    }
    command[size-1] = '\0';
    printCommand[size-1] = '\0';

    /*
     * Print the run information.
     */
    if(printRunInfo)
        fprintf(stderr, "Running: %s\n", printCommand);

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
 *   key : The key that we're looking for.
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
ReadKey(const char *key, char **keyval)
{
    int  readSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", VERSION);
    *keyval = (char *)malloc(500);
    if(RegOpenKeyEx(HKEY_CLASSES_ROOT, regkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
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
        static const char *defaultVisItPath = "C:\\VisItWindows\\bin";
        visitpath = (char *)malloc(strlen(defaultVisItPath) + 1);
        strcpy(visitpath, defaultVisItPath);
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
