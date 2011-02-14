/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <VisItControlInterface_V2.h>
#include "VisItFortran.h"

/* Data Access function prototypes. */
int VisItActivateTimestep(void *);
visit_handle VisItGetMetaData(void *);
visit_handle VisItGetMesh(int domain, const char *name, void *);
visit_handle VisItGetMaterial(int domain, const char *name, void *);
visit_handle VisItGetVariable(int domain, const char *name, void *);
visit_handle VisItGetCurve(const char *name, void *);
visit_handle VisItGetDomainList(const char *, void *);
visit_handle VisItGetDomainBoundaries(const char *, void *);
visit_handle VisItGetDomainNesting(const char *, void *);

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 *********
 *********
 ********* CONTROL INTERFACE FUNCTIONS
 *********
 *********
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 *****************************************************************************/

/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE ARE CALLBACK FUNCTIONS WE REGISTER WITH THE CONTROL INTERFACE
 **** SO IT CAN PERFORM SIMULATION-SPECIFIC ACTIONS. WE HAVE THESE HERE SO
 **** WE CAN CALL INTO FORTRAN WHERE THE FUNCTIONS WILL BE DEFINED.
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/

/* Functions to be provided by the FORTRAN simulation. */
#define F_VISITSLAVEPROCESSCALLBACK    F77_ID(visitslaveprocesscallback_,visitslaveprocesscallback,VISITSLAVEPROCESSCALLBACK)
#define F_VISITBROADCASTINTFUNCTION    F77_ID(visitbroadcastintfunction_,visitbroadcastintfunction,VISITBROADCASTINTFUNCTION)
#define F_VISITBROADCASTSTRINGFUNCTION F77_ID(visitbroadcaststringfunction_,visitbroadcaststringfunction,VISITBROADCASTSTRINGFUNCTION)
#define F_VISITCOMMANDCALLBACK         F77_ID(visitcommandcallback_,visitcommandcallback,VISITCOMMANDCALLBACK)
extern void F_VISITSLAVEPROCESSCALLBACK(void);
extern int  F_VISITBROADCASTINTFUNCTION(int *, int *);
extern int  F_VISITBROADCASTSTRINGFUNCTION(char *, int *, int *);
extern void F_VISITCOMMANDCALLBACK(const char*, int*, const char*, int *);

void
f_visit_internal_slaveprocesscallback(void)
{
    /* Call the fortran function. */
    F_VISITSLAVEPROCESSCALLBACK();
}

int
f_visit_internal_broadcastintfunction(int *value, int sender)
{
    /* Call the fortran function. */
    return F_VISITBROADCASTINTFUNCTION(value, &sender);
}

int
f_visit_internal_broadcaststringfunction(char *str, int lstr, int sender)
{
    /* Call the fortran function. */
    return F_VISITBROADCASTSTRINGFUNCTION(str, &lstr, &sender);
}

void
f_visit_internal_commandcallback(const char *cmd, const char *stringdata, void *cbdata)
{
    /* Call the fortran function. */
    char *realcmd = NULL;
    int lcmd;
    int lstringdata = strlen(stringdata);

    /* See if the format of the message from the viewer contains a bunch of
     * Qt signal junk that has been added for new features. If that junk is 
     * present and is for a push button then strip it out and send the 
     * command string to the simulation for processing. Otherwise, pass along
     * the unmodified message.
     */
    if(strncmp(cmd, "clicked();", 10) == 0)
    {
        char *end = NULL;
        if((end = strstr(cmd, ";QPushButton;Simulations;NONE")) != NULL)
        {
            lcmd = end - cmd - 10;
            realcmd = ALLOC(char, lcmd + 1);
            memset(realcmd, 0, lcmd + 1);
            strncpy(realcmd, cmd + 10, lcmd);
        }
    }

    if(realcmd == NULL)
    {
        lcmd = strlen(cmd);
        realcmd = ALLOC(char, lcmd + 1);
        strcpy(realcmd, cmd);
    }

    F_VISITCOMMANDCALLBACK(realcmd, &lcmd, stringdata, &lstringdata);

    FREE(realcmd);
}

/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE ARE FORTRAN WRAPPERS FOR THE SIMULATION CONTROL INTERFACE
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/

#define F_VISITSETDIRECTORY         F77_ID(visitsetdirectory_,visitsetdirectory,VISITSETDIRECTORY)
#define F_VISITSETOPTIONS           F77_ID(visitsetoptions_,visitsetoptions,VISITSETOPTIONS)
#define F_VISITOPENTRACEFILE        F77_ID(visitopentracefile_,visitopentracefile,VISITOPENTRACEFILE)
#define F_VISITCLOSETRACEFILE       F77_ID(visitclosetracefile_,visitclosetracefile,VISITCLOSETRACEFILE)
#define F_VISITSETUPENV             F77_ID(visitsetupenv_,visitsetupenv,VISITSETUPENV)
#define F_VISITINITIALIZESIM        F77_ID(visitinitializesim_,visitinitializesim,VISITINITIALIZESIM)
#define F_VISITSETPARALLEL          F77_ID(visitsetparallel_,visitsetparallel,VISITSETPARALLEL)
#define F_VISITSETPARALLELRANK      F77_ID(visitsetparallelrank_,visitsetparallelrank,VISITSETPARALLELRANK)
#define F_VISITDETECTINPUT          F77_ID(visitdetectinput_,visitdetectinput,VISITDETECTINPUT)
#define F_VISITATTEMPTCONNECTION    F77_ID(visitattemptconnection_,visitattemptconnection,VISITATTEMPTCONNECTION)
#define F_VISITPROCESSENGINECOMMAND F77_ID(visitprocessenginecommand_,visitprocessenginecommand,VISITPROCESSENGINECOMMAND)
#define F_VISITTIMESTEPCHANGED      F77_ID(visittimestepchanged_,visittimestepchanged,VISITTIMESTEPCHANGED)
#define F_VISITUPDATEPLOTS          F77_ID(visitupdateplots_,visitupdateplots,VISITUPDATEPLOTS)
#define F_VISITEXECUTECOMMAND       F77_ID(visitexecutecommand_,visitexecutecommand,VISITEXECUTECOMMAND)
#define F_VISITDISCONNECT           F77_ID(visitdisconnect_,visitdisconnect,VISITDISCONNECT)
#define F_VISITGETLASTERROR         F77_ID(visitgetlasterror_,visitgetlasterror,VISITGETLASTERROR)
#define F_VISITISCONNECTED          F77_ID(visitisconnected_,visitisconnected,VISITISCONNECTED)
#define F_VISITSYNCHRONIZE          F77_ID(visitsynchronize_,visitsynchronize,VISITSYNCHRONIZE)
#define F_VISITENABLESYNCHRONIZE    F77_ID(visitenablesynchronize_,visitenablesynchronize,VISITENABLESYNCHRONIZE)
#define F_VISITDEBUG1               F77_ID(visitdebug1_,visitdebug1,VISITDEBUG1)
#define F_VISITDEBUG2               F77_ID(visitdebug2_,visitdebug2,VISITDEBUG2)
#define F_VISITDEBUG3               F77_ID(visitdebug3_,visitdebug3,VISITDEBUG3)
#define F_VISITDEBUG4               F77_ID(visitdebug4_,visitdebug4,VISITDEBUG4)
#define F_VISITDEBUG5               F77_ID(visitdebug5_,visitdebug5,VISITDEBUG5)
#define F_VISITSAVEWINDOW           F77_ID(visitsavewindow_,visitsavewindow,VISITSAVEWINDOW)

/******************************************************************************
 * Function: F_VISITSETDIRECTORY
 *
 * Purpose:   Allows FORTRAN to set the directory.
 *
 * Arguments:
 *   dir  : Fortran string containing the directory.
 *   ldir : Length of the directory string.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSETDIRECTORY(VISIT_F77STRING dir, int *ldir)
{
    char *f_dir = NULL;
    COPY_FORTRAN_STRING(f_dir, dir, ldir);

    VisItSetDirectory(f_dir);

    FREE(f_dir);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITSETOPTIONS
 *
 * Purpose:   Allows FORTRAN to set the options.
 *
 * Arguments:
 *   dir  : Fortran string containing the options.
 *   ldir : Length of the options string.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSETOPTIONS(VISIT_F77STRING opt, int *lopt)
{
    char *f_opt = NULL;
    COPY_FORTRAN_STRING(f_opt, opt, lopt);

    VisItSetOptions(f_opt);

    FREE(f_opt);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITOPENTRACEFILE
 *
 * Purpose:   Allows FORTRAN to open a trace file for debugging libsim.
 *
 * Arguments:
 *   dir  : Fortran string containing the options.
 *   ldir : Length of the options string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb 19 15:47:02 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITOPENTRACEFILE(VISIT_F77STRING name, int *lname)
{
    char *f_name = NULL;
    COPY_FORTRAN_STRING(f_name, name, lname);

    VisItOpenTraceFile(f_name);

    FREE(f_name);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITCLOSETRACEFILE
 *
 * Purpose:   Allows FORTRAN to close the trace file.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb 19 15:47:02 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITCLOSETRACEFILE(void)
{
    VisItCloseTraceFile();
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITSETUPENV
 *
 * Purpose:   Allows FORTRAN to setup the VisIt environment variables.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSETUPENV(void)
{
    VisItSetupEnvironment();
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITINITIALIZESIM
 *
 * Purpose:   Allows FORTRAN to dump a sim file containing the information
 *            that VisIt needs to connect to the sim.
 *
 * Arguments:
 *   name      : Name of the simulation
 *   comment   : Info about the simulation
 *   path      : Path to the simulation's data.
 *   inputfile : The name of the simulation's input deck.
 *   uifile    : The name of the simulation's UI file.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITINITIALIZESIM( 
    VISIT_F77STRING name, int *lname,
    VISIT_F77STRING comment, int *lcomment,
    VISIT_F77STRING path, int *lpath,
    VISIT_F77STRING inputfile, int *linputfile,
    VISIT_F77STRING uifile, int *luifile,
    VISIT_F77STRING absoluteFilename, int *labsoluteFilename)
{
    int retval = -1;
    char *f_name = NULL;
    char *f_comment = NULL;
    char *f_path = NULL;
    char *f_inputfile = NULL;
    char *f_uifile = NULL;
    char *f_absoluteFilename = NULL;
    COPY_FORTRAN_STRING(f_name, name, lname);
    COPY_FORTRAN_STRING(f_comment, comment, lcomment);
    COPY_FORTRAN_STRING(f_path, path, lpath);
    COPY_FORTRAN_STRING(f_inputfile, inputfile, linputfile);
    COPY_FORTRAN_STRING(f_uifile, uifile, luifile);
    COPY_FORTRAN_STRING(f_absoluteFilename, absoluteFilename, labsoluteFilename);
    

    retval = VisItInitializeSocketAndDumpSimFile(f_name, f_comment,
                 f_path, f_inputfile, f_uifile,f_absoluteFilename);

    FREE(f_name);
    FREE(f_comment);
    FREE(f_path);
    FREE(f_inputfile);
    FREE(f_uifile);
    FREE(f_absoluteFilename);

    return retval;
}

/******************************************************************************
 * Function: F_VISITSETPARALLEL
 *
 * Purpose:   Allows FORTRAN to set whether it is a parallel sim.
 *
 * Arguments:
 *   val : Whether the simulation is parallel.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSETPARALLEL(int *val)
{
    VisItSetParallel(*val);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITSETPARALLELRANK
 *
 * Purpose:   Allows FORTRAN to set the current processor's parallel rank.
 *
 * Arguments:
 *   rank : The rank of the current processor.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSETPARALLELRANK(int *rank)
{
    VisItSetParallelRank(*rank);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDETECTINPUT
 *
 * Purpose:   Allows FORTRAN to detect VisIt socket input and console input.
 *
 * Arguments:
 *   blocking    : Whether or not to block.
 *   consoledesc : The console file descriptor.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDETECTINPUT(int *blocking, int *consoledesc)
{
    return VisItDetectInput(*blocking, *consoledesc);
}

/******************************************************************************
 * Function: F_VISITATTEMPTCONNECTION
 *
 * Purpose:   Allows FORTRAN to try and connect back to the VisIt viewer.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITATTEMPTCONNECTION(void)
{
    int ret;

    /* Set up some callbacks that will reference external FORTRAN functions. */
    VisItSetBroadcastIntFunction(f_visit_internal_broadcastintfunction);
    VisItSetBroadcastStringFunction(f_visit_internal_broadcaststringfunction);

    ret = VisItAttemptToCompleteConnection();

    /* Hook up the VisIt callback functions if VisIt connected. */
    if(ret == 1)
    {
        VisItSetActivateTimestep(VisItActivateTimestep, NULL);
        VisItSetGetMetaData(VisItGetMetaData, NULL);
        VisItSetGetMesh(VisItGetMesh, NULL);
        VisItSetGetMaterial(VisItGetMaterial, NULL);
        VisItSetGetVariable(VisItGetVariable, NULL);
        VisItSetGetCurve(VisItGetCurve, NULL);
        VisItSetGetDomainList(VisItGetDomainList, NULL);
        VisItSetGetDomainBoundaries(VisItGetDomainBoundaries, NULL);
        VisItSetGetDomainNesting(VisItGetDomainNesting, NULL);
    
        /* These functions need to be set up but they can't be set up until
         * after the VisItAttemptToCompleteConnection function completes.
         */
        VisItSetSlaveProcessCallback(f_visit_internal_slaveprocesscallback);
        VisItSetCommandCallback(f_visit_internal_commandcallback, NULL);
    }

    return ret;
}

/******************************************************************************
 * Function: F_VISITPROCESSENGINECOMMAND
 *
 * Purpose:   Allows FORTRAN to process VisIt engine commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITPROCESSENGINECOMMAND(void)
{
    return VisItProcessEngineCommand();
}

/******************************************************************************
 * Function: F_VISITTIMESTEPCHANGED
 *
 * Purpose:   Allows FORTRAN to notify VisIt that the time step changed.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITTIMESTEPCHANGED(void)
{
    VisItTimeStepChanged();
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITUPDATEPLOTS
 *
 * Purpose:   Allows FORTRAN to notify VisIt that that plots should be recreated.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITUPDATEPLOTS(void)
{
    VisItUpdatePlots();
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITEXECUTECOMMAND
 *
 * Purpose:   Allows FORTRAN to tell VisIt a command that should be interpreted.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITEXECUTECOMMAND(VISIT_F77STRING command, int *lcommand)
{
    char *f_command = NULL;
    COPY_FORTRAN_STRING(f_command, command, lcommand);
    VisItExecuteCommand(f_command);
    FREE(f_command);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDISCONNECT
 *
 * Purpose:   Allows FORTRAN to disconnect VisIt from the simulation.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDISCONNECT(void)
{
    VisItDisconnect();
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITGETLASTERROR
 *
 * Purpose:   Allows FORTRAN to retrieve the last error.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITGETLASTERROR(char *dest, int *bufsize)
{
    int M = *bufsize-1;
    int S = strlen(VisItGetLastError())-1;
    if(S < M)
        M = S;
    strncpy(dest, VisItGetLastError(), M);
    dest[M] = '\0';
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITGETLASTERROR
 *
 * Purpose:   Allows FORTRAN to retrieve whether VisIt is connected
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITISCONNECTED()
{
    return VisItIsConnected();
}

/******************************************************************************
 * Function: F_VISITGETLASTERROR
 *
 * Purpose:   Allows FORTRAN to cause a synchronize
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

int
F_VISITSYNCHRONIZE(void)
{
    return VisItSynchronize();
}

/******************************************************************************
 * Function: F_VISITGETLASTERROR
 *
 * Purpose:   Allows FORTRAN to set the synchronization mode.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITENABLESYNCHRONIZE(int *val)
{
    VisItEnableSynchronize(*val);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDEBUG1
 *
 * Purpose:   Allows FORTRAN to write to debug1 logs.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDEBUG1(const char *str, int *lstr)
{
    char *f_str = NULL;
    COPY_FORTRAN_STRING(f_str, str, lstr);
    VisItDebug1("%s", f_str);
    FREE(f_str);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDEBUG2
 *
 * Purpose:   Allows FORTRAN to write to debug2 logs.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDEBUG2(const char *str, int *lstr)
{
    char *f_str = NULL;
    COPY_FORTRAN_STRING(f_str, str, lstr);
    VisItDebug2("%s", f_str);
    FREE(f_str);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDEBUG3
 *
 * Purpose:   Allows FORTRAN to write to debug3 logs.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDEBUG3(const char *str, int *lstr)
{
    char *f_str = NULL;
    COPY_FORTRAN_STRING(f_str, str, lstr);
    VisItDebug3("%s", f_str);
    FREE(f_str);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDEBUG4
 *
 * Purpose:   Allows FORTRAN to write to debug4 logs.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDEBUG4(const char *str, int *lstr)
{
    char *f_str = NULL;
    COPY_FORTRAN_STRING(f_str, str, lstr);
    VisItDebug4("%s", f_str);
    FREE(f_str);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITDEBUG5
 *
 * Purpose:   Allows FORTRAN to write to debug5 logs.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITDEBUG5(const char *str, int *lstr)
{
    char *f_str = NULL;
    COPY_FORTRAN_STRING(f_str, str, lstr);
    VisItDebug5("%s", f_str);
    FREE(f_str);
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: F_VISITSAVEWINDOW
 *
 * Purpose:   Allows FORTRAN to save a window.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSAVEWINDOW(const char *filename, int *lfilename, int *width, int *height, int *format)
{
    int ret = VISIT_ERROR;
    char *f_filename = NULL;
    COPY_FORTRAN_STRING(f_filename, filename, lfilename);
    ret = VisItSaveWindow(f_filename, *width, *height, *format);
    FREE(f_filename);
    return ret;
}

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 *********
 *********
 ********* DATA INTERFACE FUNCTIONS
 *********
 *********
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 *****************************************************************************/

/* Functions to be provided in the Fortran application */
#define F_VISITACTIVATETIMESTEP F77_ID(visitactivatetimestep_,visitactivatetimestep,VISITACTIVATETIMESTEP)
#define F_VISITGETMETADATA      F77_ID(visitgetmetadata_,visitgetmetadata,VISITGETMETADATA)
#define F_VISITGETMESH          F77_ID(visitgetmesh_,visitgetmesh,VISITGETMESH)
#define F_VISITGETVARIABLE      F77_ID(visitgetvariable_,visitgetvariable,VISITGETVARIABLE)
#define F_VISITGETMATERIAL      F77_ID(visitgetmaterial_,visitgetmaterial,VISITGETMATERIAL)
#define F_VISITGETCURVE         F77_ID(visitgetcurve_,visitgetcurve,VISITGETCURVE)
#define F_VISITGETDOMAINLIST    F77_ID(visitgetdomainlist_,visitgetdomainlist,VISITGETDOMAINLIST)
#define F_VISITGETDOMAINBOUNDS  F77_ID(visitgetdomainbounds_,visitgetdomainbounds,VISITGETDOMAINBOUNDS)
#define F_VISITGETDOMAINNESTING F77_ID(visitgetdomainnesting_,visitgetdomainnesting,VISITGETDOMAINNESTING)

extern int F_VISITACTIVATETIMESTEP(void);
extern int F_VISITGETMETADATA(void);
extern int F_VISITGETMESH(int *, const char *, int *);
extern int F_VISITGETVARIABLE(int *, const char *, int *);
extern int F_VISITGETMATERIAL(int *, const char *, int *);
extern int F_VISITGETCURVE(const char *, int *);
extern int F_VISITGETDOMAINLIST(const char *, int *);
extern int F_VISITGETDOMAINBOUNDS(const char *, int *);
extern int F_VISITGETDOMAINNESTING(const char *, int *);

#define F_VISITSTRCMP           F77_ID(visitstrcmp_,visitstrcmp,VISITSTRCMP)

/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE ARE CALLBACK FUNCTIONS THAT CALL THEIR FORTRAN EQUIVALENTS,
 **** ALLOWING THE SimV2 DATABASE PLUGIN TO OBTAIN DATA FROM THE SIMULATION.
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: VisItGetMetaData
 *
 * Purpose:   Calls FORTRAN "visitactivatetimestep" to activate the time step.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
VisItActivateTimestep(void *cbdata)
{
    return F_VISITACTIVATETIMESTEP();
}

/******************************************************************************
 * Function: VisItGetMetaData
 *
 * Purpose:   Calls FORTRAN "visitgetmetadata" to populate a metadata object.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetMetaData(void *cbdata)
{
    return F_VISITGETMETADATA();
}

/******************************************************************************
 * Function: VisItGetMesh
 *
 * Purpose:   Calls FORTRAN "visitgetmesh" to populate a mesh object.
 *
 * Arguments:
 *   domain : The domain for which we want the scalar.
 *   name   : The name of the mesh to return.
 *   mesh   : The mesh object we're filling in.
 *   cbdata : Programmer callback data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetMesh(int domain, const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETMESH(&domain, name, &lname);
}

/******************************************************************************
 * Function: VisItGetMaterial
 *
 * Purpose:   Calls FORTRAN "visitgetmaterial" to populate a material object.
 *
 * Arguments:
 *   domain : The domain for which we want the material.
 *   name   : The name of the material to return.
 *   mat    : The material object we're filling in.
 *   cbdata : Programmer callback data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetMaterial(int domain, const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETMATERIAL(&domain, name, &lname);
}

/******************************************************************************
 * Function: VisItGetVariable
 *
 * Purpose:   Calls FORTRAN "visitgetscalar" to populate a scalar object.
 *
 * Arguments:
 *   domain : The domain for which we want the scalar.
 *   name   : The name of the scalar to return.
 *   var    : The variable object we're filling in.
 *   cbdata : Programmer callback data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetVariable(int domain, const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETVARIABLE(&domain, name, &lname);
}

/******************************************************************************
 * Function: VisItGetCurve
 *
 * Purpose:   Calls FORTRAN "visitgetcurve" to populate a curve object.
 *
 * Arguments:
 *   name   : The name of the curve to return.
 *   curve  : The curve object we're filling in.
 *   cbdata : Programmer callback data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetCurve(const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETCURVE(name, &lname);
}

/******************************************************************************
 * Function: VisItGetDomainList
 *
 * Purpose:   Calls FORTRAN "visitgetdomainlist" to populate a domain list object.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetDomainList(const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETDOMAINLIST(name, &lname);
}

/******************************************************************************
 * Function: VisItGetDomainBoundaries
 *
 * Purpose:   Calls FORTRAN "visitgetdomainbounds" to populate a domain 
 *            boundaries object.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetDomainBoundaries(const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETDOMAINBOUNDS(name, &lname);
}

/******************************************************************************
 * Function: VisItGetDomainNesting
 *
 * Purpose:   Calls FORTRAN "visitgetdomainnesting" to populate a domain 
 *            nesting object.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
VisItGetDomainNesting(const char *name, void *cbdata)
{
    int lname = strlen(name);
    return F_VISITGETDOMAINNESTING(name, &lname);
}

/******************************************************************************
 * Function: F_VISITSTRCMP
 *
 * Purpose:   Allows FORTRAN to do string comparison a la C's strcmp.
 *
 * Arguments:
 *   s1  : The first Fortran string to compare.
 *   lsl : Length of the first string
 *   s2  : The second Fortran string to compare.
 *   ls2 : Length of the second string
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 11 11:17:51 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSTRCMP(VISIT_F77STRING s1, int *ls1, VISIT_F77STRING s2, int *ls2)
{
    int retval = -1;
    char *f_s1 = NULL;
    char *f_s2 = NULL;
    COPY_FORTRAN_STRING(f_s1, s1, ls1);
    COPY_FORTRAN_STRING(f_s2, s2, ls2);

    if(f_s1 != NULL && f_s2 != NULL)
        retval = strcmp(f_s1, f_s2);

    FREE(f_s1);
    FREE(f_s2);

    return retval;
}


