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

#include <VisItControlInterface_V1.h>
#include <VisItDataInterface_V1.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 *********
 *********
 ********* COMMON FUNCTIONS, DATA, MACROS
 *********
 *********
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 *****************************************************************************/

/* F77_ID - attempt a uniform naming of FORTRAN 77 functions which
 *        - gets around loader naming conventions
 *        -
 *        - F77_ID(foo_, foo, FOO)(x, y, z)
 */
#ifndef F77_ID
#ifdef _INT_MAX_46              /* UNICOS */
#define F77_ID(x_, x, X)  X
#endif
#ifdef _H_LIMITS                /* AIX */
#define F77_ID(x_, x, X)  x
#endif
#ifdef _INCLUDE_HPUX_SOURCE     /* HPUX */
#define F77_ID(x_, x, X)  x
#endif
#ifndef F77_ID
#define F77_ID(x_, x, X)  x_
#endif
#endif

#define FORTRAN int
#define VISIT_F77STRING   char*
#define VISIT_F77CALLBACK void**

#define COPY_FORTRAN_STRING(dest, src, srclen) \
    if (strcmp(visit_fstring_to_cstring(src,*srclen), VISIT_F77NULLSTRING) == 0)\
        dest = NULL;\
    else\
        dest = visit_fstring_copy_string(visit_fstring_to_cstring(src,*srclen), *srclen);

#define ALLOC(T,N) (T*)malloc(sizeof(T) * (N))
#define FREE(ptr)  if(ptr != NULL) free(ptr);
#define VISIT_INVALID_HANDLE -1
#define VISIT_ERROR           1
#define VISIT_OKAY            0

/* This is a new owner type for arrays that can be passed to the mesh functions to
 * cause the input arrays to be copied to new storage. We have this option to make
 * it possible to pass local storage to VisIt and have it not matter that the original
 * data went out of scope.
 */
#define VISIT_OWNER_COPY      2

const char *VISIT_F77NULLSTRING = "NULLSTRING";

static char *fcdtocp_ptr = NULL;
static char *fcdtocp_data = NULL;

char *visit_fstring_copy_string(char *src, int len)
{
    char *newstr = (char *)malloc(len+1);
    memcpy(newstr, src, len);
    newstr[len] = '\0';
    return newstr;
}

char *visit_fstring_to_cstring(char *ptr, int len)
{
    if((ptr != fcdtocp_ptr) ||
       (fcdtocp_data != NULL && strncmp(fcdtocp_data, ptr, len) != 0))
    {
        fcdtocp_ptr = ptr;
        FREE(fcdtocp_data);
        fcdtocp_data = visit_fstring_copy_string(ptr, len);
    }

    return fcdtocp_data;
}


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

/* Functions to be provided by the FORTRAN simulation. */
#define F_VISITSLAVEPROCESSCALLBACK    F77_ID(visitslaveprocesscallback_,visitslaveprocesscallback,VISITSLAVEPROCESSCALLBACK)
#define F_VISITBROADCASTINTFUNCTION    F77_ID(visitbroadcastintfunction_,visitbroadcastintfunction,VISITBROADCASTINTFUNCTION)
#define F_VISITBROADCASTSTRINGFUNCTION F77_ID(visitbroadcaststringfunction_,visitbroadcaststringfunction,VISITBROADCASTSTRINGFUNCTION)
#define F_VISITCOMMANDCALLBACK         F77_ID(visitcommandcallback_,visitcommandcallback,VISITCOMMANDCALLBACK)
extern void F_VISITSLAVEPROCESSCALLBACK(void);
extern int  F_VISITBROADCASTINTFUNCTION(int *, int *);
extern int  F_VISITBROADCASTSTRINGFUNCTION(char *, int *, int *);
extern void F_VISITCOMMANDCALLBACK(const char*, int*, int*, float*, const char*, int *);

/* Functions provided by this module. */
#define F_VISITSETDIRECTORY         F77_ID(visitsetdirectory_,visitsetdirectory,VISITSETDIRECTORY)
#define F_VISITSETOPTIONS           F77_ID(visitsetoptions_,visitsetoptions,VISITSETOPTIONS)
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
f_visit_internal_commandcallback(const char *cmd, int intdata, float floatdata,
    const char *stringdata)
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
            int i;
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

    F_VISITCOMMANDCALLBACK(realcmd, &lcmd, &intdata, &floatdata, stringdata, &lstringdata);

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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Function: F_VISITSETUPENV
 *
 * Purpose:   Allows FORTRAN to setup the VisIt environment variables.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Wed Jan 10 16:01:58 PST 2007
 *   Applied David Stuebe's change.
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

    /* These functions need to be set up but they can't be set up until after
     * the VisItAttemptToCompleteConnection function completes.
     */
    VisItSetSlaveProcessCallback(f_visit_internal_slaveprocesscallback);
    VisItSetCommandCallback(f_visit_internal_commandcallback);

    return ret;
}

/******************************************************************************
 * Function: F_VISITPROCESSENGINECOMMAND
 *
 * Purpose:   Allows FORTRAN to process VisIt engine commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Thu Jan 25 15:14:13 PST 2007
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
 * Date:       Thu Jan 25 15:14:13 PST 2007
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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

/*****************************************************************************
 * These functions help us allocate objects that we can look for from the
 * Fortran functions that call back into this library.
 *****************************************************************************/

#define MAX_F_POINTERS 100
static int   f_pointers_initialized = 0;
static void *f_pointers[MAX_F_POINTERS];

int
AllocFortranPointer(void **ptr, size_t sz)
{
    int i;

    if(!f_pointers_initialized)
    {
        memset(f_pointers, 0, MAX_F_POINTERS * sizeof(void*));
        f_pointers_initialized = 1;
    }

    /* Look for the first non-NULL value. */
    for(i = 0; i < MAX_F_POINTERS; ++i)
    {
        if(f_pointers[i] == NULL)
        {
            *ptr = f_pointers[i] = malloc(sz);
            memset(*ptr, 0, sz);
            return i;
        }
    }

    return VISIT_INVALID_HANDLE;    
}

void
FreeFortranPointer(int index)
{
    if(index >= 0 && index < MAX_F_POINTERS)
        f_pointers[index] = NULL;
}

void *
GetFortranPointer(int index)
{
    return (index >= 0 && index < MAX_F_POINTERS) ? f_pointers[index] : NULL;
}

/*****************************************************************************
 * Functions for making material specification from Fortran safer.
 *****************************************************************************/

typedef struct
{
    int    have_mixed;
    int    *mix_zone;
    int    *mix_mat;
    float  *mix_vf;
    int    *mix_next;
    int    *matlist;
    char  **matnames;
    int    *dims;
    int     ndims;
    int     nmatnames;
    int    _array_size;
    int    _array_index;
    int    _array_growth;
} MaterialList;

void MaterialList_Create(MaterialList *);
void MaterialList_Destroy(MaterialList *);
void MaterialList_AddMaterial(MaterialList *, const char *);
void MaterialList_AddClean(MaterialList *, int , int);
void MaterialList_AddMixed(MaterialList *, int , int *, float *, int);
void MaterialList_AllocClean(MaterialList *, int*, int);
int  MaterialList_GetMixedSize(MaterialList *);
void MaterialList_Resize(MaterialList *, int);

/*****************************************************************************/

/* Functions to be provided in the Fortran application */
#define F_VISITGETMETADATA   F77_ID(visitgetmetadata_,visitgetmetadata,VISITGETMETADATA)
#define F_VISITGETMESH       F77_ID(visitgetmesh_,visitgetmesh,VISITGETMESH)
#define F_VISITGETSCALAR     F77_ID(visitgetscalar_,visitgetscalar,VISITGETSCALAR)
#define F_VISITGETMATERIAL   F77_ID(visitgetmaterial_,visitgetmaterial,VISITGETMATERIAL)
#define F_VISITGETCURVE      F77_ID(visitgetcurve_,visitgetcurve,VISITGETCURVE)
#define F_VISITGETDOMAINLIST F77_ID(visitgetdomainlist_,visitgetdomainlist,VISITGETDOMAINLIST)
extern int F_VISITGETMETADATA(int *);
extern int F_VISITGETMESH(int *, int *, const char *, int *);
extern int F_VISITGETSCALAR(int *, int *, const char *, int *);
extern int F_VISITGETMATERIAL(int *, int *, const char *, int *);
extern int F_VISITGETCURVE(int *, const char *, int *);
extern int F_VISITGETDOMAINLIST(int *);

/* Functions provided by this module. */
#define F_VISITMDSETCYCLETIME   F77_ID(visitmdsetcycletime_,visitmdsetcycletime, VISITMDSETCYCLETIME)
#define F_VISITMDSETRUNNING     F77_ID(visitmdsetrunning_,visitmdsetrunning,VISITMDSETRUNNING)
#define F_VISITMDMESHCREATE     F77_ID(visitmdmeshcreate_,visitmdmeshcreate,VISITMDMESHCREATE)
#define F_VISITMDMESHSETUNITS   F77_ID(visitmdmeshsetunits_,visitmdmeshsetunits,VISITMDMESHSETUNITS)
#define F_VISITMDMESHSETLABELS  F77_ID(visitmdmeshsetlabels_,visitmdmeshsetlabels,VISITMDMESHSETLABELS)
#define F_VISITMDMESHSETBLOCKTITLE      F77_ID(visitmdmeshsetblocktitle_,visitmdmeshsetblocktitle,VISITMDMESHSETBLOCKTITLE)
#define F_VISITMDMESHSETBLOCKPIECENAME  F77_ID(visitmdmeshsetblockpiecename_,visitmdmeshsetblockpiecename,VISITMDMESHSETBLOCKPIECENAME)
#define F_VISITMDMESHSETGROUPTITLE      F77_ID(visitmdmeshsetgrouptitle_,visitmdmeshsetgrouptitle,VISITMDMESHSETGROUPTITLE)
#define F_VISITMDMESHSETGROUPPIECENAME  F77_ID(visitmdmeshsetgrouppiecename_,visitmdmeshsetgrouppiecename,VISITMDMESHSETGROUPPIECENAME)
#define F_VISITMDSCALARCREATE   F77_ID(visitmdscalarcreate_,visitmdscalarcreate,VISITMDSCALARCREATE)
#define F_VISITMDSCALARSETUNITS F77_ID(visitmdscalarsetunits_,visitmdscalarsetunits,VISITMDSCALARSETUNITS)
#define F_VISITMDADDSIMCOMMAND  F77_ID(visitmdaddsimcommand_,visitmdaddsimcommand,VISITMDADDSIMCOMMAND)
#define F_VISITMDCURVECREATE    F77_ID(visitmdcurvecreate_,visitmdcurvecreate,VISITMDCURVECREATE)
#define F_VISITMDCURVESETUNITS  F77_ID(visitmdcurvesetunits_,visitmdcurvesetunits,VISITMDCURVESETUNITS)
#define F_VISITMDCURVESETLABELS F77_ID(visitmdcurvesetlabels_,visitmdcurvesetlabels,VISITMDCURVESETLABELS)
#define F_VISITMDMATERIALCREATE F77_ID(visitmdmaterialcreate_,visitmdmaterialcreate,VISITMDMATERIALCREATE)
#define F_VISITMDMATERIALADD    F77_ID(visitmdmaterialadd_,visitmdmaterialadd,VISITMDMATERIALADD)
#define F_VISITMDEXPRESSIONCREATE      F77_ID(visitmdexpressioncreate_,visitmdexpressioncreate,VISITMDEXPRESSIONCREATE)

#define F_VISITMESHRECTILINEAR   F77_ID(visitmeshrectilinear_,visitmeshrectilinear,VISITMESHRECTILINEAR)
#define F_VISITMESHRECTILINEAR2  F77_ID(visitmeshrectilinear2_,visitmeshrectilinear2,VISITMESHRECTILINEAR2)
#define F_VISITMESHCURVILINEAR   F77_ID(visitmeshcurvilinear_,visitmeshcurvilinear,VISITMESHCURVILINEAR)
#define F_VISITMESHCURVILINEAR2  F77_ID(visitmeshcurvilinear2_,visitmeshcurvilinear2,VISITMESHCURVILINEAR2)
#define F_VISITMESHPOINT         F77_ID(visitmeshpoint_,visitmeshpoint,VISITMESHPOINT)
#define F_VISITMESHPOINT2        F77_ID(visitmeshpoint2_,visitmeshpoint2,VISITMESHPOINT2)
#define F_VISITMESHUNSTRUCTURED  F77_ID(visitmeshunstructured_,visitmeshunstructured,VISITMESHUNSTRUCTURED)
#define F_VISITMESHUNSTRUCTURED2 F77_ID(visitmeshunstructured2_,visitmeshunstructured2,VISITMESHUNSTRUCTURED2)

#define F_VISITSCALARSETDATA    F77_ID(visitscalarsetdata_,visitscalarsetdata,VISITSCALARSETDATA)
#define F_VISITSCALARSETDATAC   F77_ID(visitscalarsetdatac_,visitscalarsetdatac,VISITSCALARSETDATAC)
#define F_VISITSCALARSETDATAI   F77_ID(visitscalarsetdatai_,visitscalarsetdatai,VISITSCALARSETDATAI)
#define F_VISITSCALARSETDATAF   F77_ID(visitscalarsetdataf_,visitscalarsetdataf,VISITSCALARSETDATAF)
#define F_VISITSCALARSETDATAD   F77_ID(visitscalarsetdatad_,visitscalarsetdatad,VISITSCALARSETDATAD)

#define F_VISITSETDOMAINLIST    F77_ID(visitsetdomainlist_,visitsetdomainlist,VISITSETDOMAINLIST)

#define F_VISITCURVESETDATAF    F77_ID(visitcurvesetdataf_,visitcurvesetdataf,VISITCURVESETDATAF)
#define F_VISITCURVESETDATAD    F77_ID(visitcurvesetdatad_,visitcurvesetdatad,VISITCURVESETDATAD)

#define F_VISITMATERIALSETDIMS  F77_ID(visitmaterialsetdims_,visitmaterialsetdims,VISITMATERIALSETDIMS)
#define F_VISITMATERIALADD      F77_ID(visitmaterialadd_,visitmaterialadd,VISITMATERIALADD)
#define F_VISITMATERIALADDCLEAN F77_ID(visitmaterialaddclean_,visitmaterialaddclean,VISITMATERIALADDCLEAN)
#define F_VISITMATERIALADDMIXED F77_ID(visitmaterialaddmixed_,visitmaterialaddmixed,VISITMATERIALADDMIXED)

#define F_VISITSTRCMP           F77_ID(visitstrcmp_,visitstrcmp,VISITSTRCMP)

/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE ARE CALLBACK FUNCTIONS THAT CALL THEIR FORTRAN EQUIVALENTS,
 **** ALLOWING THE SimV1 DATABASE PLUGIN TO OBTAIN DATA FROM THE SIMULATION.
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: VisItGetMetaData
 *
 * Purpose:   Calls FORTRAN "visitgetmetadata" to populate a metadata object.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_SimulationMetaData *VisItGetMetaData()
{
    /* Allocate a metadata object. */
    VisIt_SimulationMetaData *md = NULL;
    int mdi = AllocFortranPointer((void**)&md, sizeof(VisIt_SimulationMetaData));

    /* Pass the handle to the metadata object into the visitgetmetadata
       function.
     */
    if(mdi == VISIT_INVALID_HANDLE)
    {
        fprintf(stderr, "VisItGetMetaData: Could not allocate meta-data.\n");
    }
    else
    {
        if(F_VISITGETMETADATA(&mdi) != VISIT_OKAY)
        {
            fprintf(stderr, "visitgetmetadata did not return VISIT_OKAY\n");
        }
    }

    /* Remove the pointer from the Fortran pointer array since we're going to
     * donate it to the caller.
     */
    FreeFortranPointer(mdi);

    return md;
}

/******************************************************************************
 * Function: VisItGetMesh
 *
 * Purpose:   Calls FORTRAN "visitgetmesh" to populate a mesh object.
 *
 * Arguments:
 *   domain : The domain for which we want the scalar.
 *   name   : The name of the mesh to return.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_MeshData *VisItGetMesh(int domain, const char *name)
{
    VisIt_MeshData *mesh = NULL;
    int meshid = AllocFortranPointer((void**)&mesh, sizeof(VisIt_MeshData));
    
    if(meshid == VISIT_INVALID_HANDLE)
    {
        fprintf(stderr, "VisItGetMesh: Cannot allocate a mesh.\n");
    }
    else
    {
        int okay;
        int lname;

        mesh->meshType = VISIT_MESHTYPE_UNKNOWN;

        /* Get the mesh */
        okay = VISIT_ERROR;
        lname = strlen(name);
        if((okay = F_VISITGETMESH(&meshid, &domain, name, &lname)) != VISIT_OKAY)
        {
            FREE(mesh);
            mesh = NULL;
        }
        else if(mesh->meshType == VISIT_MESHTYPE_UNKNOWN)
        {
            FREE(mesh);
            mesh = NULL;
            fprintf(stderr, "VisItGetMesh: The mesh type is unknown so the "
                "mesh will not be used.\n");
        }

        FreeFortranPointer(meshid);
    }

    return mesh;
}

/******************************************************************************
 * Function: VisItGetMaterial
 *
 * Purpose:   Calls FORTRAN "visitgetmaterial" to populate a material object.
 *
 * Arguments:
 *   domain : The domain for which we want the material.
 *   name   : The name of the material to return.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_MaterialData *VisItGetMaterial(int domain, const char *name)
{
    VisIt_MaterialData *mat = NULL;
    MaterialList *ml = NULL;

    int matid = AllocFortranPointer((void**)&ml, sizeof(MaterialList));
    if(matid == VISIT_INVALID_HANDLE)
    {
        fprintf(stderr, "VisItGetMaterial: Cannot allocate a material.\n");
    }
    else
    {
        /* Get the material */
        int okay = VISIT_ERROR;
        int lname = strlen(name);
        MaterialList_Create(ml);
        if((okay = F_VISITGETMATERIAL(&matid, &domain, name, &lname)) == VISIT_OKAY)
        {
            /* Convert the MaterialList object into VisIt_MaterialData */
            int i, sz = 1;
            mat = ALLOC(VisIt_MaterialData,1);
            memset(mat, 0, sizeof(VisIt_MaterialData));
            mat->nMaterials = ml->nmatnames;
            mat->materialNumbers = (int*)malloc(ml->nmatnames * sizeof(int));
            for(i = 0; i < ml->nmatnames; ++i)
                mat->materialNumbers[i] = i;
            for(i = 0; i < ml->ndims; ++i)
                sz *= ml->dims[i];
            mat->nzones = sz;

            /* donate some arrays */
            mat->matlist = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT,
                                                        ml->matlist);
            ml->matlist = NULL;
            mat->materialNames = (const char **)ml->matnames; ml->matnames = NULL;
            ml->nmatnames = 0;
            mat->mixlen = MaterialList_GetMixedSize(ml);
            mat->mix_mat = ml->mix_mat; ml->mix_mat = NULL;
            mat->mix_zone = ml->mix_zone; ml->mix_zone = NULL;
            mat->mix_next = ml->mix_next; ml->mix_next = NULL;
            mat->mix_vf = ml->mix_vf; ml->mix_vf = NULL;
        }

        MaterialList_Destroy(ml);
        FREE(ml);
        FreeFortranPointer(matid);
    }

    return mat;
}

/******************************************************************************
 * Function: VisItGetScalar
 *
 * Purpose:   Calls FORTRAN "visitgetscalar" to populate a scalar object.
 *
 * Arguments:
 *   domain : The domain for which we want the scalar.
 *   name   : The name of the scalar to return.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_ScalarData *VisItGetScalar(int domain, const char *name)
{
    VisIt_ScalarData *scalar = NULL;
    int sid = AllocFortranPointer((void**)&scalar, sizeof(VisIt_ScalarData));
    
    if(sid == VISIT_INVALID_HANDLE)
    {
        fprintf(stderr, "VisItGetScalar: Cannot allocate a scalar.\n");
    }
    else
    {
        /* Get the scalar */
        int okay = VISIT_ERROR;
        int lname = strlen(name);
        if((okay = F_VISITGETSCALAR(&sid, &domain, name, &lname)) != VISIT_OKAY)
        {
            FREE(scalar);
            scalar = NULL;
        }

        FreeFortranPointer(sid);
    }

    return scalar;
}

/******************************************************************************
 * Function: VisItGetCurve
 *
 * Purpose:   Calls FORTRAN "visitgetcurve" to populate a curve object.
 *
 * Arguments:
 *   name   : The name of the curve to return.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_CurveData *VisItGetCurve(const char *name)
{
    VisIt_CurveData *curve = NULL;
    int cid = AllocFortranPointer((void**)&curve, sizeof(VisIt_CurveData));
    
    if(cid == VISIT_INVALID_HANDLE)
    {
        fprintf(stderr, "VisItGetCurve: Cannot allocate a curve.\n");
    }
    else
    {
        /* Get the scalar */
        int okay = VISIT_ERROR;
        int lname = strlen(name);
        if((okay = F_VISITGETCURVE(&cid, name, &lname)) != VISIT_OKAY)
        {
            FREE(curve);
            curve = NULL;
        }

        FreeFortranPointer(cid);
    }

    return curve;
}

/******************************************************************************
 * Function: VisItGetDomainList
 *
 * Purpose:   Calls FORTRAN "visitgetdomainlist" to populate a domain list object.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_DomainList *VisItGetDomainList()
{
    VisIt_DomainList *dl = NULL;
    int dlid = AllocFortranPointer((void**)&dl, sizeof(VisIt_DomainList));

    /* Get the domain list */
    if(dlid == VISIT_INVALID_HANDLE)
    {
        fprintf(stderr, "VisItGetDomainList: Could not allocate a pointer\n");
    }
    else if(F_VISITGETDOMAINLIST(&dlid) == VISIT_OKAY)
    {
        /* The Fortran code must call visitsetdomainlist*/
        FreeFortranPointer(dlid);
    }
    else
    {
        fprintf(stderr, "visitgetdomainlist returned an invalid handle, indicating that "
                "no scalar was created\n");
        FreeFortranPointer(dlid);
    }

    return dl;
}

/*
 * The SimV1 database plugin looks for this structure in the executable in order
 * to have access to the simulation's functions to return data.
 */
VisIt_SimulationCallback visitCallbacks =
{
    &VisItGetMetaData,
    &VisItGetMesh,
    &VisItGetMaterial,
    NULL, /* species */
    &VisItGetScalar,
    &VisItGetCurve,
    NULL,  /* mixed scalar */
    VisItGetDomainList
};

/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE FUNCTIONS ARE CALLABLE FROM FORTRAN
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: F_VISITMDSETCYCLETIME
 *
 * Purpose:   Allows FORTRAN to set the cycle and time for a metadata object.
 *
 * Arguments:
 *   mdhandle : The handle to the metadata object we're using.
 *   cycle    : The cycle to use
 *   time     : The time to use
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDSETCYCLETIME(int *mdhandle, int *cycle, float *time)
{
    int retval = 0;
    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        md->currentCycle = *cycle;
        md->currentTime  = *time;
        retval = 1;
    }
    else
        fprintf(stderr, "visitmdsetcycletime: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDSETRUNNING
 *
 * Purpose:   Allows FORTRAN to set whether the simulation is running in a 
 *            metadata object.
 *
 * Arguments:
 *   mdhandle : The handle to the metadata object we're using.
 *   runflag  : Whether or not the simulation is running.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDSETRUNNING(int *mdhandle, int *runflag)
{
    int retval = 0;
    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        md->currentMode  = *runflag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
        retval = 1;
    }
    else
        fprintf(stderr, "visitmdsetrunning: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHCREATE
 *
 * Purpose:   Allows FORTRAN to create mesh metadata
 *
 * Arguments:
 *   mdhandle  : The handle to the metadata object we're using.
 *   meshname  : Fortran string containing the mesh name.
 *   lmeshname : Length of the mesh name string.
 *   meshtype  : The mesh type
 *   tdim      : The mesh's topological dimension.
 *   sdim      : The mesh's spatial dimension.
 *   ndomains  : The mesh's number of domains.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHCREATE(int *mdhandle, VISIT_F77STRING meshname, int *lmeshname,
    int *meshtype, int *tdim, int *sdim, int *ndomains)
{
    int retval = VISIT_INVALID_HANDLE;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        int index = VISIT_INVALID_HANDLE;
        VisIt_MeshMetaData *mmd = NULL;
        char *f_meshname = NULL;
        COPY_FORTRAN_STRING(f_meshname, meshname, lmeshname);

        mmd = ALLOC(VisIt_MeshMetaData, 1 + md->numMeshes);
        if(mmd)
        {
            /* Grow the scalar array. */
            if(md->numMeshes > 0)
            {
                memcpy(mmd, md->meshes, sizeof(VisIt_MeshMetaData) * md->numMeshes);
                free(md->meshes);
            }
            md->meshes = mmd;
            index = md->numMeshes++;

            /* Set some properties about the new scalar. */
            md->meshes[index].name = f_meshname;
            md->meshes[index].meshType = *meshtype;
            md->meshes[index].topologicalDimension = *tdim;
            md->meshes[index].spatialDimension = *sdim;
            md->meshes[index].numBlocks = *ndomains;

            /* Finish initializing the new VisIt_MeshMetaData. */
            md->meshes[index].blockTitle = strdup("Domains");
            md->meshes[index].blockPieceName = strdup("domain");
            md->meshes[index].numGroups = 0;
            md->meshes[index].units = NULL;
            md->meshes[index].xLabel = NULL;
            md->meshes[index].yLabel = NULL;
            md->meshes[index].zLabel = NULL;
        }
        else
        {
            fprintf(stderr, "visitmdmeshcreate: Could not create new mesh\n");
            FREE(f_meshname);
        }

        retval = index;
    }
    else
        fprintf(stderr, "visitmdmeshcreate: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHSETUNITS
 *
 * Purpose:   Allows FORTRAN to create set the units for mesh metadata
 *
 * Arguments:
 *   mdhandle  : The handle to the metadata object we're using.
 *   meshindex : The index of the mesh in the metadata.
 *   units     : Fortran string containing the mesh units.
 *   lunits    : Length of the mesh units string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHSETUNITS(int *mdhandle, int *meshindex, VISIT_F77STRING units,
    int *lunits)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*meshindex >= 0 && *meshindex < md->numMeshes)
        {
            char *f_units = NULL;
            COPY_FORTRAN_STRING(f_units, units, lunits);
            FREE((char *)md->meshes[*meshindex].units);
            md->meshes[*meshindex].units = f_units;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmeshsetunits: An invalid index was passed\n");
    }
    else
        fprintf(stderr, "visitmdmeshsetunits: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHSETLABELS
 *
 * Purpose:   Allows FORTRAN to create set the axis labels for mesh metadata
 *
 * Arguments:
 *   mdhandle  : The handle to the metadata object we're using.
 *   meshindex : The index of the mesh in the metadata.
 *   xlabel    : Fortran string containing the mesh's x label.
 *   lxlabel   : Length of the xlabel string.
 *   ylabel    : Fortran string containing the mesh's y label.
 *   lylabel   : Length of the ylabel string.
 *   zlabel    : Fortran string containing the mesh's z label.
 *   lzlabel   : Length of the zlabel string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHSETLABELS(int *mdhandle, int *meshindex, 
    VISIT_F77STRING xlabel, int *lxlabel, 
    VISIT_F77STRING ylabel, int *lylabel, 
    VISIT_F77STRING zlabel, int *lzlabel)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*meshindex >= 0 && *meshindex < md->numMeshes)
        {
            char *f_xl = NULL, *f_yl = NULL, *f_zl = NULL;
            COPY_FORTRAN_STRING(f_xl, xlabel, lxlabel);
            COPY_FORTRAN_STRING(f_yl, ylabel, lylabel);
            COPY_FORTRAN_STRING(f_zl, zlabel, lzlabel);
            FREE((char *)md->meshes[*meshindex].xLabel);
            FREE((char *)md->meshes[*meshindex].yLabel);
            FREE((char *)md->meshes[*meshindex].zLabel);
            md->meshes[*meshindex].xLabel = f_xl;
            md->meshes[*meshindex].yLabel = f_yl;
            md->meshes[*meshindex].zLabel = f_zl;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmeshsetlabels: An invalid index was passed\n");
    }
    else
        fprintf(stderr, "visitmdmeshsetlabels: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHSETBLOCKTITLE
 *
 * Purpose:   Allows FORTRAN to create set the blockTitle for mesh metadata
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   meshindex   : The index of the mesh in the metadata.
 *   blockTitle  : Fortran string containing the mesh blockTitle.
 *   lblockTitle : Length of the mesh blockTitle string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHSETBLOCKTITLE(int *mdhandle, int *meshindex, 
    VISIT_F77STRING blockTitle, int *lblockTitle)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*meshindex >= 0 && *meshindex < md->numMeshes)
        {
            char *f_blockTitle = NULL;
            COPY_FORTRAN_STRING(f_blockTitle, blockTitle, lblockTitle);
            FREE((char *)md->meshes[*meshindex].blockTitle);
            md->meshes[*meshindex].blockTitle = f_blockTitle;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmeshsetblocktitle: An invalid index was passed\n");
    }
    else
        fprintf(stderr, "visitmdmeshsetblocktitle: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHSETBLOCKPIECENAME
 *
 * Purpose:   Allows FORTRAN to create set the blockPieceName for mesh metadata
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   meshindex   : The index of the mesh in the metadata.
 *   blockPieceName  : Fortran string containing the mesh blockPieceName.
 *   lblockPieceName : Length of the mesh blockPieceName string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHSETBLOCKPIECENAME(int *mdhandle, int *meshindex, 
    VISIT_F77STRING blockPieceName, int *lblockPieceName)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*meshindex >= 0 && *meshindex < md->numMeshes)
        {
            char *f_blockPieceName = NULL;
            COPY_FORTRAN_STRING(f_blockPieceName, blockPieceName, lblockPieceName);
            FREE((char *)md->meshes[*meshindex].blockPieceName);
            md->meshes[*meshindex].blockPieceName = f_blockPieceName;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmeshsetblocktitle: An invalid index was passed\n");
    }
    else
        fprintf(stderr, "visitmdmeshsetblocktitle: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHSETGROUPTITLE
 *
 * Purpose:   Allows FORTRAN to create set the groupTitle for mesh metadata
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   meshindex   : The index of the mesh in the metadata.
 *   groupTitle  : Fortran string containing the mesh groupTitle.
 *   lgroupTitle : Length of the mesh groupTitle string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHSETGROUPTITLE(int *mdhandle, int *meshindex, 
    VISIT_F77STRING groupTitle, int *lgroupTitle)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*meshindex >= 0 && *meshindex < md->numMeshes)
        {
            char *f_groupTitle = NULL;
            COPY_FORTRAN_STRING(f_groupTitle, groupTitle, lgroupTitle);
            FREE((char *)md->meshes[*meshindex].groupTitle);
            md->meshes[*meshindex].groupTitle = f_groupTitle;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmeshsetgrouptitle: An invalid index was passed\n");
    }
    else
        fprintf(stderr, "visitmdmeshsetgrouptitle: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMESHSETGROUPPIECENAME
 *
 * Purpose:   Allows FORTRAN to create set the groupPieceName for mesh metadata
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   meshindex   : The index of the mesh in the metadata.
 *   groupPieceName  : Fortran string containing the mesh groupPieceName.
 *   lgroupPieceName : Length of the mesh groupPieceName string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMESHSETGROUPPIECENAME(int *mdhandle, int *meshindex, 
    VISIT_F77STRING groupPieceName, int *lgroupPieceName)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*meshindex >= 0 && *meshindex < md->numMeshes)
        {
            char *f_groupPieceName = NULL;
            COPY_FORTRAN_STRING(f_groupPieceName, groupPieceName, lgroupPieceName);
            FREE((char *)md->meshes[*meshindex].groupPieceName);
            md->meshes[*meshindex].groupPieceName = f_groupPieceName;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmeshsetgrouptitle: An invalid index was passed\n");
    }
    else
        fprintf(stderr, "visitmdmeshsetgrouptitle: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDSCALARCREATE
 *
 * Purpose:   Allows FORTRAN to create scalar metadata
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   scalarname  : Fortran string containing the scalar name.
 *   lscalarname : Length of the scalar name string.
 *   meshname    : Fortran string containing the mesh name.
 *   lmeshname   : Length of the mesh name string.
 *   centering   : The scalar's centering
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDSCALARCREATE(int *mdhandle, VISIT_F77STRING scalarname, int *lscalarname,
    VISIT_F77STRING meshname, int *lmeshname, int *centering)
{
    int retval = VISIT_INVALID_HANDLE;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        int index = VISIT_INVALID_HANDLE;
        VisIt_ScalarMetaData *smd = NULL;
        char *f_scalarname = NULL;
        char *f_meshname = NULL;
        COPY_FORTRAN_STRING(f_scalarname, scalarname, lscalarname);
        COPY_FORTRAN_STRING(f_meshname, meshname, lmeshname);

        smd = ALLOC(VisIt_ScalarMetaData, 1 + md->numScalars);
        if(smd)
        {
            /* Grow the scalar array. */
            if(md->numScalars > 0)
            {
                memcpy(smd, md->scalars, sizeof(VisIt_ScalarMetaData) * md->numScalars);
                free(md->scalars);
            }
            md->scalars = smd;
            index = md->numScalars++;

            /* Set some properties about the new scalar. */
            md->scalars[index].name = f_scalarname;
            md->scalars[index].meshName = f_meshname;
            md->scalars[index].centering = (*centering==VISIT_VARCENTERING_ZONE) ?
                VISIT_VARCENTERING_ZONE : VISIT_VARCENTERING_NODE;

            /* Finish initializing the new VisIt_ScalarMetaData. */
            md->scalars[index].treatAsASCII = 0;
        }
        else
        {
            fprintf(stderr, "visitmdscalarcreate: Could not create new scalar\n");
            FREE(f_scalarname);
            FREE(f_meshname);
        }

        retval = index;
    }
    else
        fprintf(stderr, "visitmdscalarcreate: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDSCALARSETUNITS
 *
 * Purpose:   Allows FORTRAN to create set the units for mesh metadata
 *
 * Arguments:
 *   mdhandle : The handle to the metadata object we're using.
 *   sindex   : The index of the scalar to modify.
 *   units    : Fortran string containing the units.
 *   lunits   : Length of the units string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDSCALARSETUNITS(int *mdhandle, int *sindex, VISIT_F77STRING units, int *lunits)
{
    int retval = VISIT_ERROR;

#ifdef SUPPORTED_IN_V1
/* Units are apparently not supported in the V1 data interface. */
    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*sindex >= 0 && *sindex < md->numScalars)
        {
            char *f_units = NULL;
            COPY_FORTRAN_STRING(f_units, units, lunits);
            FREE(md->scalars[*sindex].units);
            md->scalars[*sindex].units = f_units;
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdscalarsetunits: An invalid scalar handle was passed\n");
    }
    else
        fprintf(stderr, "visitmdscalarsetunits: Could not access metadata\n");
#endif

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDCURVECREATE
 *
 * Purpose:   Allows FORTRAN to create curve metadata
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   curvename   : Fortran string containing the curve name.
 *   lcurvename  : Length of the curve name string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDCURVECREATE(int *mdhandle, VISIT_F77STRING curvename, int *lcurvename)
{
    int retval = VISIT_INVALID_HANDLE;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        int index = VISIT_INVALID_HANDLE;
        VisIt_CurveMetaData *cmd = NULL;
        char *f_curvename = NULL;
        COPY_FORTRAN_STRING(f_curvename, curvename, lcurvename);

        cmd = ALLOC(VisIt_CurveMetaData, 1 + md->numCurves);
        if(cmd)
        {
            /* Grow the curve array. */
            if(md->numCurves > 0)
            {
                memcpy(cmd, md->curves, sizeof(VisIt_CurveMetaData) * md->numCurves);
                free(md->curves);
            }
            md->curves = cmd;
            index = md->numCurves++;

            /* Set some properties about the new Curve. */
            md->curves[index].name = f_curvename;

            /* Finish initializing the new VisIt_CurveMetaData. */
            md->curves[index].xUnits = NULL;
            md->curves[index].yUnits = NULL;
            md->curves[index].xLabel = NULL;
            md->curves[index].yLabel = NULL;
        }
        else
        {
            fprintf(stderr, "visitmdcurvecreate: Could not create new curve\n");
            FREE(f_curvename);
        }

        retval = index;
    }
    else
        fprintf(stderr, "visitmdcurvecreate: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDCURVESETUNITS
 *
 * Purpose:   Allows FORTRAN to set the units on curve metadata.
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   cindex      : The index of the curve that we want to change.
 *   xunits      : Fortran string containing the x units.
 *   lxunits     : Length of the xunits string.
 *   yunits      : Fortran string containing the y units.
 *   lyunits     : Length of the yunits string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDCURVESETUNITS(int *mdhandle, int *cindex, VISIT_F77STRING xunits, int *lxunits,
    VISIT_F77STRING yunits, int *lyunits)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*cindex >= 0 && *cindex < md->numCurves)
        {
            char *f_xunits = NULL;
            char *f_yunits = NULL;
            COPY_FORTRAN_STRING(f_xunits, xunits, lxunits);
            COPY_FORTRAN_STRING(f_yunits, yunits, lyunits);
            FREE((char *)md->curves[*cindex].xUnits);
            FREE((char *)md->curves[*cindex].yUnits);
            md->curves[*cindex].xUnits = f_xunits;
            md->curves[*cindex].yUnits = f_yunits;

            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdcurvesetunits: An invalid curve handle was passed\n");
    }
    else
        fprintf(stderr, "visitmdcurvesetunits: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDCURVESETLABELS
 *
 * Purpose:   Allows FORTRAN to set the labels for curve metadata.
 *
 * Arguments:
 *   mdhandle    : The handle to the metadata object we're using.
 *   cindex      : The index of the curve that we'll modify.
 *   xlabels     : Fortran string containing the x label.
 *   lxlabels    : Length of the x label string.
 *   ylabels     : Fortran string containing the y label.
 *   lylabels    : Length of the y label string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDCURVESETLABELS(int *mdhandle, int *cindex, VISIT_F77STRING xlabels,
    int *lxlabels, VISIT_F77STRING ylabels, int *lylabels)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*cindex >= 0 && *cindex < md->numCurves)
        {
            char *f_xlabels = NULL;
            char *f_ylabels = NULL;
            COPY_FORTRAN_STRING(f_xlabels, xlabels, lxlabels);
            COPY_FORTRAN_STRING(f_ylabels, ylabels, lylabels);
            FREE((char *)md->curves[*cindex].xLabel);
            FREE((char *)md->curves[*cindex].yLabel);
            md->curves[*cindex].xLabel = f_xlabels;
            md->curves[*cindex].yLabel = f_ylabels;

            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdcurvesetlabels: An invalid curve handle was passed\n");
    }
    else
        fprintf(stderr, "visitmdcurvesetlabels: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMATERIALCREATE
 *
 * Purpose:   Allows FORTRAN to create material metadata.
 *
 * Arguments:
 *   mdhandle  : The handle to the metadata object we're using.
 *   matname   : Fortran string containing the name of the material.
 *   lmatname  : Length of the matname string.
 *   meshanme  : Fortran string containing the name of the mesh.
 *   lmeshname : Length of the mesh string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMATERIALCREATE(int *mdhandle, VISIT_F77STRING matname, int *lmatname,
    VISIT_F77STRING meshname, int *lmeshname)
{
    int retval = VISIT_INVALID_HANDLE;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        int index = VISIT_INVALID_HANDLE;
        VisIt_MaterialMetaData *mmd = NULL;
        char *f_matname = NULL;
        char *f_meshname = NULL;
        COPY_FORTRAN_STRING(f_matname, matname, lmatname);
        COPY_FORTRAN_STRING(f_meshname, meshname, lmeshname);

        mmd = ALLOC(VisIt_MaterialMetaData, 1 + md->numMaterials);
        if(mmd)
        {
            /* Grow the scalar array. */
            if(md->numMaterials > 0)
            {
                memcpy(mmd, md->materials, sizeof(VisIt_MaterialMetaData) * md->numMaterials);
                FREE(md->materials);
            }
            md->materials = mmd;
            index = md->numMaterials++;

            /* Set some properties about the new scalar. */
            md->materials[index].name = f_matname;
            md->materials[index].meshName = f_meshname;

            /* Finish initializing the new VisIt_ScalarMetaData. */
            md->materials[index].numMaterials = 0;
            md->materials[index].materialNames = NULL;
        }
        else
        {
            fprintf(stderr, "visitmdmaterialcreate: Could not create new material\n");
            FREE(f_matname);
            FREE(f_meshname);
        }

        retval = index;
    }
    else
        fprintf(stderr, "visitmdmaterialcreate: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDMATERIALADD
 *
 * Purpose:   Allows FORTRAN to add a material name to material metadata.
 *
 * Arguments:
 *   mdhandle : The handle to the metadata object we're using.
 *   mhandle  : The index of the material we'll modify.
 *   matname  : Fortran string containing the name of material to add.
 *   lmatname : Length of the material name string.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDMATERIALADD(int *mdhandle, int *mhandle, VISIT_F77STRING matname, int *lmatname)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        if(*mhandle >= 0 && *mhandle < md->numMaterials)
        {
            VisIt_MaterialMetaData *mmd = &md->materials[*mhandle];
            char *f_matname = NULL;
            COPY_FORTRAN_STRING(f_matname, matname, lmatname);

            if(mmd->numMaterials == 0)
            {
                mmd->materialNames = (const char **)malloc(sizeof(char*));
                mmd->materialNames[0] = f_matname;
            }
            else
            {
                mmd->materialNames = (const char **)realloc(
                    mmd->materialNames, sizeof(char*) * (mmd->numMaterials + 1));
                mmd->materialNames[mmd->numMaterials] = f_matname;
            }

            ++(mmd->numMaterials);
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitmdmaterialadd: An invalid material handle was passed.\n");
    }
    else
        fprintf(stderr, "visitmdmaterialadd: Could not access metadata\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMDADDSIMCOMMAND
 *
 * Purpose:   Allows FORTRAN to add sim commands to the metadata.
 *
 * Arguments:
 *   mdhandle     : The handle to the metadata object we're using.
 *   commandname  : Fortran string containing the name of the sim command.
 *   lcommandname : Length of the command name string.
 *   argtype      : The type of arguments the command takes.
 *   enabled      : Whether the command is enabled.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Wed Jan 10 12:50:54 PDT 2007
 *   Fill in some of the newer members of VisIt_SimulationControlCommand.
 *
 *****************************************************************************/

FORTRAN
F_VISITMDADDSIMCOMMAND(int *mdhandle, VISIT_F77STRING commandname, int *lcommandname,
    int *argtype, int *enabled)
{
    int retval = VISIT_ERROR;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        int index = VISIT_INVALID_HANDLE;
        VisIt_SimulationControlCommand *scc = NULL;
        char *f_commandname = NULL, *f_commandname2 = NULL;
        COPY_FORTRAN_STRING(f_commandname,  commandname, lcommandname);
        COPY_FORTRAN_STRING(f_commandname2, commandname, lcommandname);

        scc = ALLOC(VisIt_SimulationControlCommand, 1 + md->numGenericCommands);
        if(scc)
        {
            /* Grow the generic command array. */
            if(md->numGenericCommands > 0)
            {
                memcpy(scc, md->genericCommands,
                       sizeof(VisIt_SimulationControlCommand) *
                       md->numGenericCommands);
                free(md->genericCommands);
            }
            md->genericCommands = scc;
            index = md->numGenericCommands++;

            /* Set some properties about the new command. */
            memset(&md->genericCommands[index], 0, sizeof(VisIt_SimulationControlCommand));
            md->genericCommands[index].name = f_commandname;
            md->genericCommands[index].text = f_commandname2;
            md->genericCommands[index].argType = *argtype;
            md->genericCommands[index].enabled = *enabled?1:0;
            retval = VISIT_OKAY;
        }
        else
        {
            fprintf(stderr, "visitmdaddsimcommand: Could not create new command\n");
            FREE(f_commandname);
        }
    }
    else
        fprintf(stderr, "visitmdaddsimcommand: Could not access metadata\n");

    return retval;
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
 * Date:       Fri Jan 27 16:15:11 PST 2006
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

/******************************************************************************
 * Function: F_VISITMDEXPRESSIONCREATE
 *
 * Purpose:   Allows FORTRAN to create expression metadata
 *
 * Arguments:
 *   mdhandle        : The handle to the metadata object we're using.
 *   expressionname  : Fortran string containing the expression name.
 *   lexpressionname : Length of the expression name string.
 *   definition      : Fortran string containing the mesh name.
 *   ldefinition     : Length of the mesh name string.
 *   vartype         : The expression's type
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 1 15:23:48 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMDEXPRESSIONCREATE(int *mdhandle, VISIT_F77STRING expressionname, int *lexpressionname,
    VISIT_F77STRING definition, int *ldefinition, int *vartype)
{
    int retval = VISIT_INVALID_HANDLE;

    VisIt_SimulationMetaData *md = (VisIt_SimulationMetaData *)GetFortranPointer(*mdhandle);
    if(md)
    {
        int index = VISIT_INVALID_HANDLE, vt;
        VisIt_ExpressionMetaData *emd = NULL;
        char *f_expressionname = NULL;
        char *f_definition = NULL;
        COPY_FORTRAN_STRING(f_expressionname, expressionname, lexpressionname);
        COPY_FORTRAN_STRING(f_definition, definition, ldefinition);

        vt = (*vartype < 0 || *vartype >= VISIT_VARTYPE_UNKNOWN) ? VISIT_VARTYPE_UNKNOWN : *vartype;

        emd = ALLOC(VisIt_ExpressionMetaData, 1 + md->numExpressions);
        if(emd)
        {
            /* Grow the expression array. */
            if(md->numExpressions > 0)
            {
                memcpy(emd, md->expressions, sizeof(VisIt_ExpressionMetaData) * md->numExpressions);
                free(md->expressions);
            }
            md->expressions = emd;
            index = md->numExpressions++;

            /* Set some properties about the new expression. */
            md->expressions[index].name = f_expressionname;
            md->expressions[index].definition = f_definition;
            md->expressions[index].vartype = vt;
        }
        else
        {
            fprintf(stderr, "visitmdexpressioncreate: Could not create new expression\n");
            FREE(f_expressionname);
            FREE(f_definition);
        }

        retval = index;
    }
    else
        fprintf(stderr, "visitmdexpressioncreate: Could not access metadata\n");

    return retval;
}

static VisIt_DataArray
VisIt_CreateDataArrayFromFloat_C(int o, float *f, int sz)
{
    VisIt_DataArray da;

    if(o == VISIT_OWNER_COPY)
    {
        da.dataType = VISIT_DATATYPE_FLOAT;
        da.owner    = VISIT_OWNER_VISIT;
        if(f != NULL)
        {
            da.fArray   = (float*)malloc(sizeof(float) * sz);
            memcpy(da.fArray, f, sizeof(float) * sz);
        }
        else
            da.fArray = NULL;
    }
    else
        da = VisIt_CreateDataArrayFromFloat(o, f);

    return da;   
}

static VisIt_DataArray
VisIt_CreateDataArrayFromInt_C(int o, int *i, int sz)
{
    VisIt_DataArray da;

    if(o == VISIT_OWNER_COPY)
    {
        da.dataType = VISIT_DATATYPE_FLOAT;
        da.owner    = VISIT_OWNER_VISIT;
        if(i != NULL)
        {
            da.iArray   = (int*)malloc(sizeof(int) * sz);
            memcpy(da.iArray, i, sizeof(int) * sz);
        }
        else
            da.iArray = NULL;
    }
    else
        da = VisIt_CreateDataArrayFromInt(o, i);

    return da;   
}


/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE FUNCTIONS ARE CALLABLE FROM FORTRAN AND ARE FOR PASSING REAL 
 **** SIMULATION DATA BACK TO THIS MODULE
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: F_VISITMESHRECTILINEAR
 *
 * Purpose:   Allows FORTRAN to pass back data for a rectilinear mesh.
 *
 * Arguments:
 *   meshid : The handle to the mesh that was passed by VisItGetMesh.
 *   ...
 *   dims   : The x,y,z dimensions of the mesh.
 *   ndims  : The number of dimensions in the mesh.
 *   x      : The x coordinate array.
 *   y      : The y coordinate array.
 *   z      : The z coordinate array.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Thu Jan 11 15:43:02 PST 2007
 *   Added VISIT_OWNER_COPY support and support for setting the ownser.
 *
 *****************************************************************************/

FORTRAN
F_VISITMESHRECTILINEAR2(int *meshid, int *baseindex, int *minrealindex, int *maxrealindex, 
    int *dims, int *ndims, float *x, float *y, float *z, int *owner)
{
    int retval = VISIT_ERROR;
    VisIt_MeshData *mesh = (VisIt_MeshData *)GetFortranPointer(*meshid);
    if(mesh)
    {
         int i;
         mesh->meshType = VISIT_MESHTYPE_RECTILINEAR;
         mesh->rmesh = ALLOC(VisIt_RectilinearMesh,1);
         memset(mesh->rmesh, 0, sizeof(VisIt_RectilinearMesh));

         mesh->rmesh->ndims = *ndims;

         for(i = 0; i < 3; ++i)
         {
             mesh->rmesh->baseIndex[i] = baseindex[i];
             mesh->rmesh->minRealIndex[i] = minrealindex[i];
             mesh->rmesh->maxRealIndex[i] = maxrealindex[i];
             mesh->rmesh->dims[i] = dims[i];
         }

         mesh->rmesh->xcoords = VisIt_CreateDataArrayFromFloat_C(*owner, x, dims[0]);
         if(*ndims > 1)
             mesh->rmesh->ycoords = VisIt_CreateDataArrayFromFloat_C(*owner, y, dims[1]);
         if(*ndims > 2)
             mesh->rmesh->zcoords = VisIt_CreateDataArrayFromFloat_C(*owner, z, dims[2]);
         retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitmeshrectlinear: An invalid handle was used.\n");

    return retval;
}

FORTRAN
F_VISITMESHRECTILINEAR(int *meshid, int *baseindex, int *minrealindex, int *maxrealindex, 
    int *dims, int *ndims, float *x, float *y, float *z)
{
    int owner = VISIT_OWNER_SIM;
    return F_VISITMESHRECTILINEAR2(meshid, baseindex, minrealindex, maxrealindex,
        dims, ndims, x, y, z, &owner);
}

/******************************************************************************
 * Function: F_VISITMESHCURVILINEAR
 *
 * Purpose:   Allows FORTRAN to pass back data for a curvilinear mesh.
 *
 * Arguments:
 *   meshid : The handle to the mesh that was passed by VisItGetMesh.
 *   ...
 *   dims   : The x,y,z dimensions of the mesh.
 *   ndims  : The number of dimensions in the mesh.
 *   x      : The x coordinate array.
 *   y      : The y coordinate array.
 *   z      : The z coordinate array.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Thu Jan 11 15:43:02 PST 2007
 *   Added VISIT_OWNER_COPY support and support for setting the ownser.
 *
 *****************************************************************************/

FORTRAN
F_VISITMESHCURVILINEAR2(int *meshid, int *baseindex, int *minrealindex, int *maxrealindex, 
    int *dims, int *ndims, float *x, float *y, float *z, int *owner)
{
    int retval = VISIT_ERROR;
    VisIt_MeshData *mesh = (VisIt_MeshData *)GetFortranPointer(*meshid);
    if(mesh)
    {
         int i, sz = 1;
         mesh->meshType = VISIT_MESHTYPE_CURVILINEAR;
         mesh->cmesh = ALLOC(VisIt_CurvilinearMesh,1);
         memset(mesh->cmesh, 0, sizeof(VisIt_CurvilinearMesh));

         mesh->cmesh->ndims = *ndims;

         for(i = 0; i < 3; ++i)
         {
             mesh->cmesh->baseIndex[i] = baseindex[i];
             mesh->cmesh->minRealIndex[i] = minrealindex[i];
             mesh->cmesh->maxRealIndex[i] = maxrealindex[i];
             mesh->cmesh->dims[i] = dims[i];
         }

         /* Compute the size of the coordinate arrays so we can copy if needed. */
         sz = dims[0];
         if(*ndims > 1)
             sz *= dims[1];
         if(*ndims > 2)
             sz *= dims[2];

         mesh->cmesh->xcoords = VisIt_CreateDataArrayFromFloat_C(*owner, x, sz);
         if(*ndims > 1)
             mesh->cmesh->ycoords = VisIt_CreateDataArrayFromFloat_C(*owner, y, sz);
         if(*ndims > 2)
             mesh->cmesh->zcoords = VisIt_CreateDataArrayFromFloat_C(*owner, z, sz);
         retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitmeshcurvilinear: An invalid handle was used.\n");

    return retval;
}

FORTRAN
F_VISITMESHCURVILINEAR(int *meshid, int *baseindex, int *minrealindex, int *maxrealindex, 
    int *dims, int *ndims, float *x, float *y, float *z)
{
    int owner = VISIT_OWNER_SIM;
    return F_VISITMESHCURVILINEAR2(meshid, baseindex, minrealindex, maxrealindex, 
        dims, ndims, x, y, z, &owner);
}

/******************************************************************************
 * Function: F_VISITMESHPOINT
 *
 * Purpose:   Allows FORTRAN to pass back data for a point mesh.
 *
 * Arguments:
 *   meshid : The handle to the mesh that was passed by VisItGetMesh.
 *   ...
 *   dims   : The x,y,z dimensions of the mesh.
 *   ndims  : The number of dimensions in the mesh.
 *   x      : The x coordinate array.
 *   y      : The y coordinate array.
 *   z      : The z coordinate array.
 *   owner  : The owner.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Thu Jan 11 15:43:02 PST 2007
 *   Added VISIT_OWNER_COPY support and support for setting the ownser.
 *
 *****************************************************************************/

FORTRAN
F_VISITMESHPOINT2(int *meshid, int *ndims, int *nnodes, float *x, float *y,
    float *z, int *owner)
{
    int retval = VISIT_ERROR;
    VisIt_MeshData *mesh = (VisIt_MeshData *)GetFortranPointer(*meshid);
    if(mesh)
    {
         int i;
         mesh->meshType = VISIT_MESHTYPE_POINT;
         mesh->pmesh = ALLOC(VisIt_PointMesh,1);
         memset(mesh->pmesh, 0, sizeof(VisIt_PointMesh));

         mesh->pmesh->nnodes = *nnodes;
         mesh->pmesh->ndims = *ndims;

         mesh->pmesh->xcoords = VisIt_CreateDataArrayFromFloat_C(*owner, x, *nnodes);
         mesh->pmesh->ycoords = VisIt_CreateDataArrayFromFloat_C(*owner, y, *nnodes);
         if(*ndims > 2)
             mesh->pmesh->zcoords = VisIt_CreateDataArrayFromFloat_C(*owner, z, *nnodes);
         retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitmeshpoint: An invalid handle was used.\n");

    return retval;
}

FORTRAN
F_VISITMESHPOINT(int *meshid, int *ndims, int *nnodes, float *x, float *y,
    float *z)
{
    int owner = VISIT_OWNER_SIM;
    return F_VISITMESHPOINT2(meshid, ndims, nnodes, x, y, z, &owner);
}

/******************************************************************************
 * Function: F_VISITMESHUNSTRUCTURED
 *
 * Purpose:   Allows FORTRAN to pass back data for an unstructured mesh.
 *
 * Arguments:
 *   meshid : The handle to the mesh that was passed by VisItGetMesh.
 *   ...
 *   dims   : The x,y,z dimensions of the mesh.
 *   ndims  : The number of dimensions in the mesh.
 *   x      : The x coordinate array.
 *   y      : The y coordinate array.
 *   z      : The z coordinate array.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *   Brad Whitlock, Thu Jan 11 15:43:02 PST 2007
 *   Added VISIT_OWNER_COPY support and support for setting the ownser.
 *
 *****************************************************************************/

FORTRAN
F_VISITMESHUNSTRUCTURED2(int *meshid, int *ndims, int *nnodes, int *nzones,
    int *firstrealzone, int *lastrealzone, float *x, float *y, float *z,
    int *connectivitylen, int *connectivity, int *owner)
{
    int retval = VISIT_ERROR;
    VisIt_MeshData *mesh = (VisIt_MeshData *)GetFortranPointer(*meshid);
    if(mesh)
    {
         int i;
         mesh->meshType = VISIT_MESHTYPE_UNSTRUCTURED;
         mesh->umesh = ALLOC(VisIt_UnstructuredMesh,1);
         memset(mesh->umesh, 0, sizeof(VisIt_UnstructuredMesh));

         mesh->umesh->ndims = *ndims;
         mesh->umesh->nnodes = *nnodes;
         mesh->umesh->nzones = *nzones;
         mesh->umesh->firstRealZone = *firstrealzone;
         mesh->umesh->lastRealZone = *lastrealzone;

         mesh->umesh->xcoords = VisIt_CreateDataArrayFromFloat_C(*owner, x, *nnodes);
         mesh->umesh->ycoords = VisIt_CreateDataArrayFromFloat_C(*owner, y, *nnodes);
         if(*ndims > 2)
             mesh->umesh->zcoords = VisIt_CreateDataArrayFromFloat_C(*owner, z, *nnodes);

         mesh->umesh->connectivityLen = *connectivitylen;
         mesh->umesh->connectivity = VisIt_CreateDataArrayFromInt_C(*owner, connectivity,
              *connectivitylen);

         retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitmeshunstructured: An invalid handle was used.\n");

    return retval;
}

FORTRAN
F_VISITMESHUNSTRUCTURED(int *meshid, int *ndims, int *nnodes, int *nzones,
    int *firstrealzone, int *lastrealzone, float *x, float *y, float *z,
    int *connectivitylen, int *connectivity)
{
    int owner = VISIT_OWNER_SIM;

    return F_VISITMESHUNSTRUCTURED2(meshid, ndims, nnodes, nzones,
        firstrealzone, lastrealzone, x, y, z, connectivitylen, connectivity,
        &owner);
}

/******************************************************************************
 * Function: F_VISITSCALARSETDATA
 *
 * Purpose:   Allows FORTRAN to pass back scalar data.
 *
 * Arguments:
 *   sid      : The handle to the scalar object that was passed by VisItGetScalar.
 *   ...
 *   scalar   : The simulation's scalar data array.
 *   dims     : The dimensions of the scalar data array.
 *   ndims    : The number of dimensions.
 *   datatype : The type of data being passed.
 *   owner    : Who will own the scalar data array.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSCALARSETDATA(int *sid, void *scalar, int *dims, int *ndims,
    int *datatype, int *owner)
{
    int retval = VISIT_ERROR;
    VisIt_ScalarData *sd = (VisIt_ScalarData *)GetFortranPointer(*sid);
    if(sd)
    {
        int i, sz = 1;
        for(i = 0; i < *ndims; ++i)
            sz *= dims[i];
        sd->len = sz;

        if(*owner == VISIT_OWNER_SIM)
        {
            retval = VISIT_OKAY;
            if(*datatype == VISIT_DATATYPE_CHAR)
                sd->data = VisIt_CreateDataArrayFromChar(VISIT_OWNER_SIM, scalar);
            else if(*datatype == VISIT_DATATYPE_INT)
                sd->data = VisIt_CreateDataArrayFromInt(VISIT_OWNER_SIM, scalar);
            else if(*datatype == VISIT_DATATYPE_FLOAT)
                sd->data = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM, scalar);
            else if(*datatype == VISIT_DATATYPE_DOUBLE)
                sd->data = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_SIM, scalar);
            else
            {
                fprintf(stderr, "visitsetscalardata: Invalid data type value.\n");
                retval = VISIT_ERROR;
            }
        }
        else if(*owner == VISIT_OWNER_VISIT)
        {
            retval = VISIT_OKAY;
            if(*datatype == VISIT_DATATYPE_CHAR)
            {
                char *cptr = ALLOC(char, sz);
                sd->data = VisIt_CreateDataArrayFromChar(VISIT_OWNER_VISIT, cptr);
                memcpy((void *)cptr, scalar, sz * sizeof(char));
            }
            else if(*datatype == VISIT_DATATYPE_INT)
            {
                int *iptr = ALLOC(int, sz);
                sd->data = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT, iptr);
                memcpy((void *)iptr, scalar, sz * sizeof(int));
            }
            else if(*datatype == VISIT_DATATYPE_FLOAT)
            {
                float *fptr = ALLOC(float, sz);
                sd->data = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_VISIT, fptr);
                memcpy((void *)fptr, scalar, sz * sizeof(float));
            }
            else if(*datatype == VISIT_DATATYPE_DOUBLE)
            {
                double *dptr = ALLOC(double, sz);
                sd->data = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT, dptr);
                memcpy((void *)dptr, scalar, sz * sizeof(double));
            }
            else
            {
                fprintf(stderr, "visitsetscalardata: Invalid data type value.\n");
                retval = VISIT_ERROR;
            }
        }
        else
            fprintf(stderr, "visitsetscalardata: Invalid owner value.\n");
    }
    else
        fprintf(stderr, "visitsetscalardata: Could not set scalar data\n");

    return retval;
}

FORTRAN
F_VISITSCALARSETDATAC(int *sid, char *scalar, int *dims, int *ndims)
{
    int retval = VISIT_ERROR;
    VisIt_ScalarData *sd = (VisIt_ScalarData *)GetFortranPointer(*sid);
    if(sd)
    {
        int i;
        sd->len = 1;
        for(i = 0; i < *ndims; ++i)
            sd->len *= dims[i];

        sd->data = VisIt_CreateDataArrayFromChar(VISIT_OWNER_SIM, scalar);
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitscalarsetdatac: Could not set scalar data\n");

    return retval;
}

FORTRAN
F_VISITSCALARSETDATAI(int *sid, int *scalar, int *dims, int *ndims)
{
    int retval = VISIT_ERROR;
    VisIt_ScalarData *sd = (VisIt_ScalarData *)GetFortranPointer(*sid);
    if(sd)
    {
        int i;
        sd->len = 1;
        for(i = 0; i < *ndims; ++i)
            sd->len *= dims[i];

        sd->data = VisIt_CreateDataArrayFromInt(VISIT_OWNER_SIM, scalar);
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitscalarsetdatai: Could not set scalar data\n");

    return retval;
}

FORTRAN
F_VISITSCALARSETDATAF(int *sid, float *scalar, int *dims, int *ndims)
{
    int retval = VISIT_ERROR;
    VisIt_ScalarData *sd = (VisIt_ScalarData *)GetFortranPointer(*sid);
    if(sd)
    {
        int i;
        sd->len = 1;
        for(i = 0; i < *ndims; ++i)
            sd->len *= dims[i];

        sd->data = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM, scalar);
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitscalarsetdataf: Could not set scalar data\n");

    return retval;
}

FORTRAN
F_VISITSCALARSETDATAD(int *sid, double *scalar, int *dims, int *ndims)
{
    int retval = VISIT_ERROR;
    VisIt_ScalarData *sd = (VisIt_ScalarData *)GetFortranPointer(*sid);
    if(sd)
    {
        int i;
        sd->len = 1;
        for(i = 0; i < *ndims; ++i)
            sd->len *= dims[i];

        sd->data = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_SIM, scalar);
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitscalarsetdatad: Could not set scalar data\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITSETDOMAINLIST
 *
 * Purpose:   Allows FORTRAN to pass back the domain list.
 *
 * Arguments:
 *   dlhandle : The handle to the domain list object that was passed by
 *              VisItGetDomainList.
 *   totaldomains : The total number of domains in the simulation.
 *   domainids    : The list of domains owned by this processor.
 *   ndomids      : The number of domains in the list.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITSETDOMAINLIST(int *dlhandle, int *totaldomains, int *domainids, int *ndomids)
{
    int retval = VISIT_ERROR;

    if(*ndomids > 0 && *totaldomains > 0)
    {
        VisIt_DomainList *dl = (VisIt_DomainList *)GetFortranPointer(*dlhandle);
        if(dl)
        {
            dl->nTotalDomains = *totaldomains;
            dl->nMyDomains = *ndomids;
            dl->myDomains = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT,
                                                         ALLOC(int, *ndomids));
            memcpy(dl->myDomains.iArray, domainids, *ndomids * sizeof(int));
            retval = VISIT_OKAY;
        }
        else
            fprintf(stderr, "visitsetdomainlist: Could not access domain list.\n");
    }
    else
    {
        fprintf(stderr, "visitsetdomainlist: Number of domains must be greater "
                        "than zero.\n");
    }

    return retval;
}

/******************************************************************************
 * Function: F_VISITCURVESETDATAF
 *
 * Purpose:   Allows FORTRAN to pass back curve data.
 *
 * Arguments:
 *   cid   : The handle to the curve object that was passed by VisItGetCurve.
 *   cx    : The x coordinate array for the curve.
 *   cy    : The y coordinate array for the curve.
 *   nvals : The number of coordinates.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITCURVESETDATAF(int *cid, float *cx, float *cy, int *nvals)
{
    int retval = VISIT_ERROR;
    VisIt_CurveData *cd = (VisIt_CurveData *)GetFortranPointer(*cid);
    if(cd)
    {
        cd->len = *nvals;
        cd->x = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_VISIT,
                                               ALLOC(float,*nvals));
        cd->y = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_VISIT,
                                               ALLOC(float,*nvals));
        memcpy(cd->x.fArray, cx, *nvals * sizeof(float));
        memcpy(cd->y.fArray, cy, *nvals * sizeof(float));
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitcurvesetdataf: Could not set curve data\n");

    return retval;
}

FORTRAN
F_VISITCURVESETDATAD(int *cid, double *cx, double *cy, int *nvals)
{
    int retval = VISIT_ERROR;
    VisIt_CurveData *cd = (VisIt_CurveData *)GetFortranPointer(*cid);
    if(cd)
    {
        cd->len = *nvals;
        cd->x = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT,
                                                ALLOC(double,*nvals));
        cd->y = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_VISIT,
                                                ALLOC(double,*nvals));
        memcpy(cd->x.dArray, cx, *nvals * sizeof(double));
        memcpy(cd->y.dArray, cy, *nvals * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitcurvesetdatad: Could not set curve data\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMATERIALSETDIMS
 *
 * Purpose:   Allows FORTRAN to set the dimensions for the specified material.
 *
 * Arguments:
 *   mhandle : The handle to the material object that was passed by
 *             VisItGetMaterial.
 *   dims    : The dimensions of the material.
 *   ndims   : The number of dimensions.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMATERIALSETDIMS(int *mhandle, int *dims, int *ndims)
{
    int retval = VISIT_ERROR;
    MaterialList *ml = (MaterialList *)GetFortranPointer(*mhandle);
    if(ml)
    {
        MaterialList_AllocClean(ml, dims, *ndims);
        retval = VISIT_OKAY;
    }
    else
        fprintf(stderr, "visitcreatematerial: An invalid material handle was passed.\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMATERIALADD
 *
 * Purpose:   Allows FORTRAN to add a material name to the material
 *
 * Arguments:
 *   mhandle  : The handle to the material object that was passed by
 *              VisItGetMaterial.
 *   matname  : Fortran string containing the name of the material to add.
 *   lmatname : The length of the material name.
 * 
 * Returns:    The new material's material number is returned.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMATERIALADD(int *mhandle, VISIT_F77STRING matname, int *lmatname)
{
    int retval = VISIT_ERROR;
    MaterialList *ml = (MaterialList *)GetFortranPointer(*mhandle);
    if(ml)
    {
        char *f_matname = NULL;
        COPY_FORTRAN_STRING(f_matname, matname, lmatname);
        MaterialList_AddMaterial(ml, f_matname);
        retval = ml->nmatnames-1;
        FREE(f_matname);
    }
    else
        fprintf(stderr, "visitmaterialadd: An invalid material handle was passed.\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMATERIALADDCLEAN
 *
 * Purpose:   Allows FORTRAN to set the material for a clean zone.
 *
 * Arguments:
 *   mhandle : The handle to the material object that was passed by 
 *             VisItGetMaterial.
 *   cell    : The i,j,k coordinate of the zone in the mesh.
 *   matno   : The material number to use for the zone.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMATERIALADDCLEAN(int *mhandle, int *cell, int *matno)
{
    int retval = VISIT_ERROR;
    MaterialList *ml = (MaterialList *)GetFortranPointer(*mhandle);
    if(ml)
    {
        if(ml->ndims < 1)
        {
            fprintf(stderr, "visitmaterialaddclean: Number of material "
                "dimensions is less than 1. The visitmaterialsetdims must "
                "be called before this function is called.\n");
        }
        else
        {
            int i, cellid = -1;
            if(ml->ndims == 1)
                cellid = cell[0]-1;
            else if(ml->ndims == 2)
            {
                cellid = ml->dims[0] * (cell[1]-1) + (cell[0]-1);
            }
            else if(ml->ndims == 3)
            {
                cellid =  (ml->dims[0] * ml->dims[1] * (cell[2]-1)) +
                          (ml->dims[0] * (cell[1]-1)) +
                          (cell[0]-1);
            }

            if(cellid != -1)
                MaterialList_AddClean(ml, cellid, *matno);
            retval = VISIT_OKAY;
        }
    }
    else
        fprintf(stderr, "visitmaterialaddclean: An invalid material handle was passed.\n");

    return retval;
}

/******************************************************************************
 * Function: F_VISITMATERIALADDMIXED
 *
 * Purpose:   Allows FORTRAN to set the materials for a mixed zone.
 *
 * Arguments:
 *   mhandle : The handle to the material object that was passed by 
 *             VisItGetMaterial.
 *   cell    : The i,j,k coordinate of the zone in the mesh.
 *   matnos  : The material numbers to use for the zone.
 *   matvf   : The volume fractions for each material.
 *   nmats   : The number of materials.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 27 16:15:11 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

FORTRAN
F_VISITMATERIALADDMIXED(int *mhandle, int *cell, int *matnos, float *matvf,
    int *nmats)
{
    int retval = VISIT_ERROR;
    MaterialList *ml = (MaterialList *)GetFortranPointer(*mhandle);
    if(ml)
    {
        if(ml->ndims < 1)
        {
            fprintf(stderr, "visitmaterialaddmixed: Number of material "
                "dimensions is less than 1. The visitmaterialsetdims must "
                "be called before this function is called.\n");
        }
        else
        {
            int i, cellid = -1;
            if(ml->ndims == 1)
                cellid = cell[0]-1;
            else if(ml->ndims == 2)
            {
                cellid = ml->dims[0] * (cell[1]-1) + (cell[0]-1);
            }
            else if(ml->ndims == 3)
            {
                cellid =  (ml->dims[0] * ml->dims[1] * (cell[2]-1)) +
                          (ml->dims[0] * (cell[1]-1)) +
                          (cell[0]-1);
            }

            if(cellid != -1)
                MaterialList_AddMixed(ml, cellid, matnos, matvf, *nmats);
            retval = VISIT_OKAY;
        }
    }
    else
        fprintf(stderr, "visitmaterialaddmixed: An invalid material handle was passed.\n");

    return retval;
}

/*****************************************************************************
 *****************************************************************************
 *****************************************************************************
 ****
 **** THESE FUNCTIONS ARE FUNCTIONS FOR THE MaterialList ADT.
 ****
 *****************************************************************************
 *****************************************************************************
 *****************************************************************************/
void
MaterialList_Create(MaterialList *ml)
{
    ml->have_mixed = 0;
    ml->mix_zone = NULL;
    ml->mix_mat = NULL;
    ml->mix_vf = NULL;
    ml->mix_next = NULL;
    ml->matlist = NULL;
    ml->matnames = NULL;
    ml->nmatnames = 0;
    ml->dims = NULL;
    ml->ndims = 0;
    ml->_array_size = 0;
    ml->_array_index = 1;
    ml->_array_growth = 1000;
}

void
MaterialList_Destroy(MaterialList *ml)
{
    int i;
    FREE(ml->matlist);
    FREE(ml->dims);
    if (ml->have_mixed)
    {
        FREE(ml->mix_zone);   
        FREE(ml->mix_mat);    
        FREE(ml->mix_vf);      
        FREE(ml->mix_next);    
    }
    for(i = 0; i < ml->nmatnames; ++i)
        FREE(ml->matnames[i]);
    FREE(ml->matnames);
}

void
MaterialList_AddMaterial(MaterialList *ml, const char *mat)
{
    if(ml->nmatnames == 0)
    {
        ml->matnames = (char **)malloc(sizeof(char *));
        ml->matnames[0] = strdup(mat);
    }
    else
    {
        ml->matnames = (char **)realloc(ml->matnames, (ml->nmatnames+1)*sizeof(char*));
        ml->matnames[ml->nmatnames] = strdup(mat);
    }
    ++ml->nmatnames;
}

void
MaterialList_AddClean(MaterialList *ml, int zone, int matNumber)
{
    ml->matlist[zone] = matNumber;
}

void
MaterialList_AddMixed(MaterialList *ml, int zone, int *matNumbers,
    float *matVf, int nMats)
{
    int i;

    /* Grow the arrays if they will not fit nMats materials. */
    MaterialList_Resize(ml, nMats);

    /* Record the mixed zone as a negative offset into the mix arrays. */
    ml->matlist[zone] = -ml->_array_index;

    /* Update the mix arrays. */
    for(i = 0; i < nMats; ++i)
    {
        int index = ml->_array_index - 1;

        ml->mix_zone[index] = zone;
        ml->mix_mat[index]  = matNumbers[i];
        ml->mix_vf[index]   = matVf[i];

        if(i < nMats - 1)
            ml->mix_next[index] = index + 2;
        else
            ml->mix_next[index] = 0;

        ++(ml->_array_index);
    }

    /* indicate that we have mixed materials. */
    ml->have_mixed = 1;
}

void
MaterialList_AllocClean(MaterialList *ml, int *dims, int ndims)
{
    int i, sz = 1;
    ml->dims = (int*)malloc(ndims * sizeof(int));
    ml->ndims = ndims;
    for(i = 0; i < ndims; ++i)
    {
        ml->dims[i] = dims[i];
        sz *= dims[i];
    }
    ml->matlist = (int*)malloc(sz * sizeof(int));
}

int
MaterialList_GetMixedSize(MaterialList *ml)
{
    return ml->_array_index - 1;
}

void MaterialList_Resize(MaterialList *ml, int nMats)
{ 
    if(ml->_array_index + nMats >= ml->_array_size)
    {
        int new_size = ml->_array_size + ml->_array_growth;

        if(ml->_array_size == 0)
        {
            /* Reallocate arrays in large increments. */
            ml->mix_zone = (int*)malloc(new_size * sizeof(int));
            ml->mix_mat  = (int*)malloc(new_size * sizeof(int));
            ml->mix_vf   = (float*)malloc(new_size * sizeof(float));
            ml->mix_next = (int*)malloc(new_size * sizeof(int));
        }
        else
        {
            /* Reallocate arrays in large increments. */
            ml->mix_zone = (int*)realloc(ml->mix_zone, new_size * sizeof(int));
            ml->mix_mat  = (int*)realloc(ml->mix_mat, new_size * sizeof(int));
            ml->mix_vf   = (float*)realloc(ml->mix_vf, new_size * sizeof(float));
            ml->mix_next = (int*)realloc(ml->mix_next, new_size * sizeof(int));
        }

        ml->_array_size = new_size;
    }
}

