/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*  
 *  $Header: /n/picasso/project/mm/mpeg/mpeg_dist/mpeg_encode/RCS/file.c,v 1.2 1993/06/30 20:06:09 keving Exp $
 *  $Log: file.c,v $
 * Revision 1.2  1993/06/30  20:06:09  keving
 * nothing
 *
 * Revision 1.1  1993/06/03  21:08:08  keving
 * nothing
 *
 */

#include "tk.h"

#include "all.h"

#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>

#define MAX_FILES   1000
#define MAX_NAME_LEN    256
#define MAX_STRING_LEN    MAX_NAME_LEN

typedef int boolean;
#define TRUE 1
#define FALSE 0

extern char currentPath[MAXPATHLEN];

char    globString[1024];

static DIR *dfd;

void    ResetPath(void);
int ListDirectory(ClientData nulldata, Tcl_Interp *interp, int argc,
          char **argv);
int ChangeDirectory(ClientData nulldata, Tcl_Interp *interp, int argc,
          char **argv);
void    SortFiles(int numStrings, char strings[MAX_FILES][MAX_NAME_LEN],
          boolean *dirList, int permute[]);

static void    UpdatePath(Tcl_Interp *interp, char *directory);
static boolean    MatchesGlob(char *string, char *glob);



void    ResetPath()
{
    if ( getwd(currentPath) == 0 )
    {
    fprintf(stderr, "Error getting pathname!!!\n");
    exit(1);
    }

    strcpy(&currentPath[strlen(currentPath)], "/");

    dfd = opendir(currentPath);
    if ( dfd == NULL )
    {
    fprintf(stderr, "can't open '%s'\n", currentPath);
    exit(1);
    }
}


static void    UpdatePath(Tcl_Interp *interp, char *directory)
{
    int length;
    char *charPtr;

    length = strlen(currentPath);

    if ( strcmp(directory, "./") == 0 )
    return /* nothing */ ;
    else if ( strcmp(directory, "../") == 0 )
    {
    /* delete backwards up to '/' */

    if ( length < 2 )
    {
        fprintf(stderr, "Error:  backing up from root directory!!!\n");
        exit(1);
    }

    charPtr = &currentPath[length-2];
    while ( (charPtr != currentPath) && (*charPtr != '/') )
        charPtr--;
    charPtr++;    /* leave the '/' */
    *charPtr = '\0';
    }
    else
    {
    strcpy(&currentPath[length], directory);
    }
}


int ChangeDirectory(ClientData nulldata, Tcl_Interp *interp, int argc,
          char **argv)
{
    char *directory = argv[1];

    UpdatePath(interp, directory);

    fprintf(stdout, "Opening directory: '%s'\n", currentPath);

    dfd = opendir(currentPath);
    if ( dfd == NULL )
    {
    fprintf(stderr, "can't open '%s'\n", currentPath);
    return TCL_OK;    /* shouldn't, really */
    }

    return TCL_OK;
}


int ListDirectory(ClientData nulldata, Tcl_Interp *interp, int argc,
          char **argv)
{
    struct dirent *dp;
    struct stat stbuf;
    char command[256];
    char fileName[MAX_FILES][MAX_NAME_LEN];
    boolean dirList[MAX_FILES];
    int permute[MAX_FILES];
    int    fileCount = 0;
    register int index;
    char fullName[MAXPATHLEN];
    char    *restPtr;

    sprintf(command, "ShowCurrentDirectory %s", currentPath);
    Tcl_Eval(interp, command, 0, (char **) NULL);

    if ( dfd == NULL )
    {
    fprintf(stderr, "TRIED TO LIST NULL DIRECTORY\n");

    return TCL_OK;
    }

/* check if root directory */
    if ( strlen(currentPath) != 1 )
    {
    sprintf(fileName[fileCount], "../");
    dirList[fileCount] = TRUE;
    fileCount++;
    }

    strcpy(fullName, currentPath);
    restPtr = &fullName[strlen(fullName)];

    while ( (dp = readdir(dfd)) != NULL )
    {
    strcpy(restPtr, dp->d_name);
    stat(fullName, &stbuf);

    if ( dp->d_name[0] != '.' )
    {
        if ( S_ISDIR(stbuf.st_mode) )
        {
        sprintf(fileName[fileCount], "%s/", dp->d_name);
        dirList[fileCount] = TRUE;
        fileCount++;
        }
        else
        {
        if ( MatchesGlob(dp->d_name, globString) )
        {
            strcpy(fileName[fileCount], dp->d_name);
            dirList[fileCount] = FALSE;
            fileCount++;
        }
        }
    }
    }

    SortFiles(fileCount, fileName, dirList, permute);

    for ( index = 0; index < fileCount; index++ )
    {
    sprintf(command, "AddBrowseFile %s", fileName[permute[index]]);
    Tcl_Eval(interp, command, 0, (char **) NULL);
    }

    closedir(dfd);

    return TCL_OK;
}


void    SortFiles(int numStrings, char strings[MAX_FILES][MAX_NAME_LEN],
          boolean *dirList, int permute[])
{
    register int i, j;
    int temp;
    int    numDirs;
    int    ptr;

    for ( i = 0; i < numStrings; i++ )
    permute[i] = i;

    /* put all directories at front */
    numDirs = 0;
    ptr = numStrings-1;
    while ( numDirs != ptr )
    {
    /* go past dirs */
    while ( (numDirs < ptr) && (dirList[permute[numDirs]]) )
        numDirs++;

    /* go past non-dirs */
    while ( (numDirs < ptr) && (! dirList[permute[ptr]]) )
        ptr--;

    if ( numDirs != ptr )
    {
        temp = permute[numDirs];
        permute[numDirs] = ptr;
        permute[ptr] = temp;
    }
    }

    if ( dirList[permute[numDirs]] )
    numDirs++;

    for ( i = 0; i < numDirs; i++ )
    for ( j = i+1; j < numDirs; j++ )
    {
        if ( strcmp(&strings[permute[j]][0], &strings[permute[i]][0]) < 0 )
        {
        temp = permute[j];
        permute[j] = permute[i];
        permute[i] = temp;
        }
    }

    for ( i = numDirs; i < numStrings; i++ )
    for ( j = i+1; j < numStrings; j++ )
    {
        if ( strcmp(&strings[permute[j]][0], &strings[permute[i]][0]) < 0 )
        {
        temp = permute[j];
        permute[j] = permute[i];
        permute[i] = temp;
        }
    }
}


int SetBrowseGlob (ClientData nulldata, Tcl_Interp *interp,
           int argc, char **argv)
{
    if (argc == 2 )
    {
    strcpy(globString, argv[1]);

    fprintf(stdout, "GLOB:  %s\n", globString);

    return TCL_OK;
    }

    Tcl_AppendResult (interp, 
            "wrong args: should be \"", argv[0]," string\"", (char *) NULL);
    return TCL_ERROR;
}


static boolean    MatchesGlob(char *string, char *glob)
{
    char    *stringRight, *globRight;

    while ( (*glob != '\0') && (*glob != '*') )        /* match left side */
    {
    if ( (*string == '\0') || (*string != *glob) )
        return FALSE;
    string++;
    glob++;
    }

    if ( *glob == '\0' )    /* no star */
    return TRUE;

    /* now match right side */
    stringRight = &string[strlen(string)-1];
    globRight = &glob[strlen(glob)-1];

    while ( *globRight != '*' )
    {
    if ( (stringRight < string) || (*stringRight != *globRight) )
        return FALSE;
    globRight--;
    stringRight--;
    }

    return TRUE;
}
