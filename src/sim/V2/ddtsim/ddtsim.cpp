/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#include "ddtsim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <string>

#include "pwd.h"
#include "sys/stat.h"
#include "visitmesh.h"
#include "visitvariable.h"
#include "visitsim.h"
#include "visitdata.h"
#include "mpicompat.h"

#ifndef VISIT_STATIC
  extern "C"
  {
  #include "openmpi.h"
  #include "mpich.h"
    }
#else
  #include "mpi.h"
#endif

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

#define CMD_HALT   "stop"
#define CMD_RUN    "run"
#define CMD_STEP   "step"
#define CMD_UPDATE "update"
#define CMD_TOGGLE "toggle autoupdate"
#define CMD_DDT    "DDT"

DDTSim* DDTSim::mInstance = NULL;
MpiMode DDTSim::mMpiMode = NOMPI;
char DDTSim::mProgramName[MAX_NAME_LENGTH];
char DDTSim::mProgramPath[MAX_PATH_LENGTH];
char DDTSim::mAbsSimFileName[MAX_SIMFILE_NAME_LENGTH];
char DDTSim::mDDTSharedPath[MAX_PATH_LENGTH];
char DDTSim::mVisItPath[MAX_PATH_LENGTH];
char DDTSim::mVisItArchPath[MAX_PATH_LENGTH];

static ddtsim_interface_type ddtsim_interface;

void ddtsim_interface_init()
{
    ddtsim_interface.break_to_ddt = false;
    ddtsim_interface.max_name_length__no_modify = MAX_NAME_LENGTH;
}


/**************** MPI ***************************************************************************/

static void ddtMpiCompat_Init()
{
#ifndef VISIT_STATIC
    switch(DDTSim::mpi())
    {
    case OPENMPI:
        mpicompat_openmpi_init();
        break;
    case MPICH2:
    case MPT3:    // Cray, same as mpich2
        mpicompat_mpich_init();
        break;
    case NOMPI:
    default:
        DDTSim::error("(ddtsim,ddtMpiCompat_Init) Unsupported MPI mode\n");
    }
#endif
    // No init needed if we're using static compilation
}

static int ddtMpiCompat_Bcast(void *buffer, int count, int type, int root, int comm)
{
    if (comm != DDTSIM_MPI_COMM_WORLD)
    {
        DDTSim::error("(ddtsim) Unexpected use of ddtSim_Bcast with comm other than DDT_MPI_COMM_WORLD\n");
        exit(1); // HOOKS_IGNORE
    }
    if (type != DDTSIM_MPI_INT && type != DDTSIM_MPI_CHAR )
    {
        DDTSim::error("(ddtsim) Unexpected use of ddtSim_Bcast with type other than DDT_MPI_INT or DDT_MPI_CHAR\n");
        exit(1); // HOOKS_IGNORE
    }

#ifndef VISIT_STATIC
    switch(DDTSim::mpi())
    {
    case OPENMPI:
        return mpicompat_openmpi_Bcast(buffer,count,type,root);
    case MPICH2:
    case MPT3:    // Cray, same as mpich2
        return mpicompat_mpich_Bcast(buffer,count,type,root);
    case NOMPI:
    default:
        DDTSim::error("(ddtsim,ddtMpiCompat_Bcast) Unsupported MPI mode\n");
        exit(1); // HOOKS_IGNORE
    }
#else
    MPI_Datatype mpitype;
    switch(type)
    {
    case DDTSIM_MPI_INT:  mpitype = MPI_INT;  break;
    case DDTSIM_MPI_CHAR: mpitype = MPI_CHAR; break;
    default:
        DDTSim::error("(ddtsim,ddtMpiCompat_Bcast) Unsupported MPI mode\n");
        exit(1); // HOOKS_IGNORE
    }
    return MPI_Bcast(buffer,count,mpitype,root,MPI_COMM_WORLD);
#endif
}

static void ddtMpiCompat_rankAndSize(int *rank, int *size)
{
#ifndef VISIT_STATIC
    switch(DDTSim::mpi())
    {
    case OPENMPI:
        mpicompat_openmpi_rankAndSize(rank,size);
        break;
    case MPICH2:
    case MPT3:    // Cray, same as mpich2
        mpicompat_mpich_rankAndSize(rank,size);
        break;
    case NOMPI:
    default:
        DDTSim::error("(ddtsim,ddtMpiCompat_rankAndSize) Unsupported MPI mode\n");
    }
#else
    MPI_Comm_rank(MPI_COMM_WORLD,rank);
    MPI_Comm_size(MPI_COMM_WORLD,size);
#endif
}

/**************** Visualisation callbacks ********************************************************/

//! Callback used by VisIt to obtain domain information
static visit_handle
ddtSimGetDomainList(const char */*name*/, void *cbdata)
{
    VisItDataPointers& pointers = *((VisItDataPointers*) cbdata);
    VisItSim& visitSim = *(pointers.sim);
    //VisItData& visitData  = *(pointers.data);
    LibsimLibrary& libsim = *(pointers.libsim);

    /* Single domain per processor, so domain list will consist of just
    * the rank of this processor */
    visit_handle h = VISIT_INVALID_HANDLE;

    if (libsim.DomainList_alloc(&h) == VISIT_OKAY)
    {
        visit_handle hd1;

        int rank = visitSim.rank();

        libsim.VariableData_alloc(&hd1);
        libsim.VariableData_setDataI(hd1, VISIT_OWNER_COPY, 1, 1, &rank);

        libsim.DomainList_setDomains(h, visitSim.size(), hd1);
    }
    return h;
}

//! Callback used by VisIt to obtain metadata
static visit_handle
ddtSimGetMetaData(void *cbdata)
{
    VisItDataPointers& pointers = *((VisItDataPointers*) cbdata);
    VisItSim& visitSim = *(pointers.sim);
    VisItData& visitData  = *(pointers.data);
    LibsimLibrary& libsim = *(pointers.libsim);

    visit_handle md = VISIT_INVALID_HANDLE;

    /* Create metadata with no variables */
    if (libsim.SimulationMetaData_alloc(&md) == VISIT_OKAY)
    {
        /* Set the simulation state */
        if (visitSim.runmode() == VISIT_SIMMODE_STOPPED)
            libsim.SimulationMetaData_setMode(md, VISIT_SIMMODE_STOPPED);
        else if (visitSim.runmode() == VISIT_SIMMODE_RUNNING)
            libsim.SimulationMetaData_setMode(md,  VISIT_SIMMODE_RUNNING);
        else
            DDTSim::error("Unknown runMode in ddtSimGetMetaData\n");
        libsim.SimulationMetaData_setCycleTime(md, visitSim.cycle(), visitSim.time());

        /* Send commands for VisIt users to use */
        const char *cmd_names[] = { CMD_RUN, CMD_STEP, CMD_UPDATE,
                                    CMD_HALT, CMD_DDT, CMD_TOGGLE };
        for (unsigned int i=0; i<sizeof(cmd_names)/sizeof(const char*); ++i)
        {
            visit_handle cmd = VISIT_INVALID_HANDLE;
            if (libsim.CommandMetaData_alloc(&cmd) == VISIT_OKAY)
            {
                libsim.CommandMetaData_setName(cmd, cmd_names[i]);
                libsim.SimulationMetaData_addGenericCommand(md,cmd);
            }
        }

        /* Send a mesh to represent the tables array */
        VisItMesh *mesh = visitData.getMesh();
        if (mesh)
            mesh->applyMetadata(libsim, visitSim, md);

        /* Send metadata for tables array variable */
        VisItVariable *var = visitData.getVariable();
        if (var)
            var->applyMetadata(libsim, md);
    }
    else
        DDTSim::error("Unable to allocate metadata handle in ddtSimGetMetadata\n");
    return md;
}

//! Callback used by VisIt to obtain mesh data
static visit_handle
ddtSimGetMesh(int /*domain*/, const char *name, void *cbdata)
{
    VisItDataPointers& pointers = *((VisItDataPointers*) cbdata);
    //VisItSim& visitSim = *(pointers.sim);
    VisItData& visitData  = *(pointers.data);
    LibsimLibrary& libsim = *(pointers.libsim);

    VisItMesh *mesh = visitData.getMesh();
    visit_handle h = VISIT_INVALID_HANDLE;

    if (mesh && mesh->isNamed(name))
        mesh->applyMesh(libsim,h);
    else
        DDTSim::error("(ddtsim) Unknown mesh name\n",name);

    return h;
}

//! Callback used by VisIt to obtain variable data
static visit_handle
ddtSimGetVariable(int /*domain*/, const char *name, void *cbdata)
{
    VisItDataPointers& pointers = *((VisItDataPointers*) cbdata);
    //VisItSim& visitSim = *(pointers.sim);
    VisItData& visitData  = *(pointers.data);
    LibsimLibrary& libsim = *(pointers.libsim);

    VisItVariable *var = visitData.getVariable();
    visit_handle h = VISIT_INVALID_HANDLE;

    if (var && var->isNamed(name))
        var->applyVariable(libsim, h);
    else
        DDTSim::error("(ddtsim) Unknown variable name\n", name);

    return h;
}



/******* VisIt Command and Control helpers & callbacks ****************/

static int
visit_broadcast_int_callback(int *value, int sender)
{
    if (DDTSim::getInstance()->isParallel())
        return ddtMpiCompat_Bcast(value, 1, DDTSIM_MPI_INT, sender, DDTSIM_MPI_COMM_WORLD);
    else
        return 0;
}
static int
visit_broadcast_string_callback(char *str, int len, int sender)
{
    if (DDTSim::getInstance()->isParallel())
        return ddtMpiCompat_Bcast(str, len, DDTSIM_MPI_CHAR, sender, DDTSIM_MPI_COMM_WORLD);
    else
        return 0;
}

/* Helper function for ProcessVisItCommand */
static void
BroadcastSlaveCommand(int *command)
{
    if (DDTSim::getInstance()->isParallel())
        ddtMpiCompat_Bcast(command, 1, DDTSIM_MPI_INT, 0, DDTSIM_MPI_COMM_WORLD);
}

/* Callback involved in command communication */
static void
SlaveProcessCallback()
{
    int command = VISIT_COMMAND_PROCESS;
    BroadcastSlaveCommand(&command);
}

//! Callback for processing user issued program control commands issued from VisIt
static void
ControlCommandCallback(const char *cmd, const char */*args*/, void *cbdata)
{
    VisItDataPointers& pointers = *((VisItDataPointers*) cbdata);
    VisItSim& visitSim = *(pointers.sim);
    //VisItData& visitData  = *(pointers.data);
    LibsimLibrary& libsim = *(pointers.libsim);

    if (strcmp(cmd, CMD_HALT)==0) {
        visitSim.setRunmode(VISIT_SIMMODE_STOPPED);
    } else if (strcmp(cmd, CMD_STEP)==0) {
        visitSim.setRunmode(VISIT_SIMMODE_RUNNING);
        visitSim.setStepping(true);
    } else if (strcmp(cmd, CMD_DDT)==0) {
        visitSim.setRunmode(VISIT_SIMMODE_RUNNING);
        visitSim.setStepping(true);
        ddtsim_interface.break_to_ddt = true;
    } else if (strcmp(cmd, CMD_RUN)==0) {
        visitSim.setRunmode(VISIT_SIMMODE_RUNNING);
    } else if (strcmp(cmd, CMD_UPDATE)==0) {
        libsim.TimeStepChanged();
        libsim.UpdatePlots();
    } else if (strcmp(cmd, CMD_TOGGLE)==0) {
        visitSim.setAutoupdate(!visitSim.autoupdate());
    } else {
        DDTSim::error("(ddtsim) Received unknown command string: %s\n",cmd);
    }
}

/******* DDTSim Class ****************************************/

DDTSim::DDTSim(const char *libsimPath,int par_rank, int par_size) :
    mLogging(false), visitSim(par_rank,par_size), libsim(libsimPath)
{
    pointers.sim = &visitSim;
    pointers.data = &visitData;
    pointers.libsim = &libsim;

    if (!libsim.isLoaded())
    {
        DDTSim::error("(ddtsim) Failed to load %s, terminating.\n\n",libsimPath);
        exit(1); // HOOKS_IGNORE
    }

    /* Set the path to the VisIt install */
    libsim.SetDirectory(mVisItPath);

    /* Initialize environment variables */
    libsim.SetupEnvironment();

    /* Install callback functions for global communication */
    libsim.SetBroadcastIntFunction(visit_broadcast_int_callback);
    libsim.SetBroadcastStringFunction(visit_broadcast_string_callback);

    /* Tell libsim if simulation is parallel */
    libsim.SetParallel(visitSim.size() > 1);
    libsim.SetParallelRank(visitSim.rank());

    /* If we are rank 0, initialises the libsim library and writes a
    * .sim2 file to ~/.visit/simulations */
    if (visitSim.rank() == 0)
    {
        //char name[MAX_NAME_LENGTH+50];
        char desc[MAX_NAME_LENGTH+50];

        // Ensure visualizations directory exists
        char dir[MAX_SIMFILE_NAME_LENGTH];
        snprintf(dir,MAX_SIMFILE_NAME_LENGTH-1,"%s/.ddt",mDDTSharedPath);
        mkdir(dir, 7*64 + 7*8 + 7);
        snprintf(dir,MAX_SIMFILE_NAME_LENGTH-1,"%s/.ddt/visualizations",mDDTSharedPath);
        mkdir(dir, 7*64 + 7*8 + 7);

        // NB: Postfix of sim file also used in VisIt patch (0030_connectddt.patch)
        // DDTManager::isDatabaseDDTSim(QString).
        // If changing .sim file name here, also modify that patch.
        //sprintf(name,"%s-ddt",mProgramName);

        snprintf(mAbsSimFileName,MAX_SIMFILE_NAME_LENGTH-1,
                 "%s/.ddt/visualizations/%012d.%s-ddt.sim2",
                 mDDTSharedPath,
                 (int)time(NULL),
                 mProgramName);

        sprintf(desc,"DDT provided visualisation of %s", mProgramName);
        libsim.InitializeSocketAndDumpSimFile(
                    mProgramName,
                    desc,
                    mProgramPath,
                    NULL,NULL,
                    mAbsSimFileName);
    }
    else
        mAbsSimFileName[0] = '\0';
}

void DDTSim::init()
{
    std::string libsimPath = mVisItArchPath;

#ifndef VISIT_STATIC
    std::string libsimdyn1 = libsimPath + "/libsim/V2/lib/libsimV2dyn.so";
    std::string libsimdyn2 = libsimPath + "/lib/libsimV2dyn.so";
    struct stat buffer;
    if (stat(libsimdyn1.c_str(),&buffer)==0)
        libsimPath =  libsimdyn1;
    else if (stat(libsimdyn2.c_str(),&buffer)==0)
        libsimPath = libsimdyn2;
    else
    {
        DDTSim::error("Unable to find %s or %s\n",libsimdyn1.c_str(), libsimdyn2.c_str());
        exit(1); // HOOKS_IGNORE
    }
#endif
    // NB If compiled statically, libsimPath won't be used for anything

    if (mMpiMode != NOMPI)
    {
        ddtMpiCompat_Init();

        int rank,size;
        ddtMpiCompat_rankAndSize(&rank,&size);
        if (mInstance!=NULL)
        {
            DDTSim::error("(ddtsim) DDTSim singleton already initialized\n");
            exit(1); // HOOKS_IGNORE
        }
        mInstance = new DDTSim(libsimPath.c_str(),rank,size);
    }
    else
    {
        mInstance = new DDTSim(libsimPath.c_str(),0,1);
    }
}

DDTSim*
DDTSim::getInstance()
{
    if (mInstance==NULL)
    {
        DDTSim::error("(ddtsim) DDTSim::getInstance() called before DDTSim::init()\n");
    }
    return mInstance;
}


//! Process visualisation command from ViSIt viewer on all processes
int
DDTSim::processVisItCommand()
{
    int command;
    if (visitSim.rank() == 0)
    {
        int success = libsim.ProcessEngineCommand();
        if (success)
        {
            command = VISIT_COMMAND_SUCCESS;
            BroadcastSlaveCommand(&command);
            return 1;
        }
        else
        {
            command = VISIT_COMMAND_FAILURE;
            BroadcastSlaveCommand(&command);
            return 0;
        }
    }
    else
    {
        /* Note: only through the SlaveProcessCallback callback above can the
        * rank 0 process send a VISIT_COMMAND_PROCESS instruction to the
        * non-rank-0 processes */
        while(1)
        {
            BroadcastSlaveCommand(&command);
            switch(command)
            {
                case VISIT_COMMAND_PROCESS:
                    libsim.ProcessEngineCommand();
                    break;
                case VISIT_COMMAND_SUCCESS:
                    return 1;
                case VISIT_COMMAND_FAILURE:
                    return 0;
            }
        }
    }
    return 1;
}

//! Updates the internal VisItData object using the latest instructions from ddt-debugger
/*! ddt-debugger communicates with this library by editing the ddtsim_interface struct, then
 *  triggering the signal handler that will then calling this method. This allows a memory
 *  allocations to take place in the signal handler, avoiding potential deadlocks that could
 *  arise from invoking memory allocation functions from gdb directly. */
void
DDTSim::update(ddtsim_interface_type &interface)
{
    // TODO: Support having multiple meshes and variables. Add a flag to ddt_interface_type
    // so we know when to add/delete a mesh/variable/both to the VisItData object.
    visitData.updateMesh(interface.mesh);
    visitData.updateVariable(interface.variable);
}

//! Use to report any sort of error encountered in this library
void
DDTSim::error(const char *message, const char *variable)
{
    if (variable)
        fprintf(stderr,message,variable);
    else
        fprintf(stderr,message);
}

//! Use to report any sort of error encountered in this library
void
DDTSim::error(const char *message, const char *variable1, const char *variable2)
{
    fprintf(stderr,message,variable1,variable2);
}

//! Use to report any sort of error encountered in this library
void
DDTSim::error(const char *message, int intvar, const char *variable)
{
    if (variable)
        fprintf(stderr,message,intvar,variable);
    else
        fprintf(stderr,message,intvar);
}

//! Use to report any sort of error encountered in this library
void
DDTSim::error(const char *message, int intvar1, int intvar2)
{
    fprintf(stderr,message,intvar1,intvar2);
}

//! Set the name and path of the program in which this library is loaded.
/*! May be called directly within gdb */
void
DDTSim::setProgram(const char *name, const char *path)
{
    if (strlen(name) > MAX_NAME_LENGTH-1) {
        strncpy(mProgramName,name,MAX_NAME_LENGTH-2);
        mProgramName[MAX_NAME_LENGTH-1] = '\0';
    } else
        strcpy(mProgramName, name);

    if (strlen(path) > MAX_PATH_LENGTH-1) {
        strncpy(mProgramPath,path,MAX_PATH_LENGTH-2);
        mProgramPath[MAX_PATH_LENGTH-1] = '\0';
    } else
        strcpy(mProgramPath, path);
}

//! Set the path to a writable directory shared by the target and the DDT frontend
void
DDTSim::setSharedPath(const char *ddtSharedPath)
{
    if (strlen(ddtSharedPath) > MAX_PATH_LENGTH-1) {
        strncpy(mDDTSharedPath,ddtSharedPath,MAX_PATH_LENGTH-2);
        mDDTSharedPath[MAX_PATH_LENGTH-1] = '\0';
    } else
        strcpy(mDDTSharedPath, ddtSharedPath);
}

//! Set the paths to the VisIt launch script and VisIt architecture-specific directory
/*! May be called directly within gdb */
void
DDTSim::setVisItPaths(const char *visitPath, const char *visitArchPath)
{
    if (strlen(visitPath) > MAX_PATH_LENGTH-1) {
        strncpy(mVisItPath,visitPath,MAX_PATH_LENGTH-2);
        mVisItPath[MAX_PATH_LENGTH-1] = '\0';
    } else
        strcpy(mVisItPath, visitPath);

    if (strlen(visitArchPath) > MAX_PATH_LENGTH-1) {
        strncpy(mVisItArchPath,visitArchPath,MAX_PATH_LENGTH-2);
        mVisItArchPath[MAX_PATH_LENGTH-1] = '\0';
    } else
        strcpy(mVisItArchPath, visitArchPath);
}

/******* VisIt loop ****************************************/

//! Go into VisIt processing loop. Will return when receives a 'STEP' or 'RUN' command from VisIt
void DDTSim::visitloop()
{
    // Clear the break_to_ddt flag
    ddtsim_interface.break_to_ddt = false;

    // If we've just taken one step, OR if we're automatically updating after every step,
    // tell VisIt to update our data.
    if (visitSim.stepping() || visitSim.autoupdate())
    {
        // If we have just taken a step, change us to the stopped mode - otherwise we'll keep stepping
        // Do this before updating timestep/plots, otherwise VisIt will think the simulation is
        // still in 'running' mode rather than the correct 'stopped' mode.
        if (visitSim.stepping())
        {
            visitSim.setRunmode(VISIT_SIMMODE_STOPPED);
            visitSim.setStepping(false);
        }

        libsim.TimeStepChanged();  // Update meshes
        libsim.UpdatePlots();      // Update data
    }

    visitSim.incCycleAndTime();

    // Do the VisIt control loop.
    int blocking, err = 0;
    int visitstate = -6;
    do
    {
        blocking = (visitSim.runmode() == VISIT_SIMMODE_RUNNING) ? 0 : 1;

        /* Get input from VisIt or timeout so the simulation can run. */
        if (!isParallel() || visitSim.rank() == 0)
            visitstate = libsim.DetectInput(blocking, -1);

        if (isParallel())
            ddtMpiCompat_Bcast(&visitstate, 1, DDTSIM_MPI_INT, 0, DDTSIM_MPI_COMM_WORLD);

        /* Do different things depending on the output from VisItDetectInput */
        if (visitstate >= -6 && visitstate <= -1)
        {
            DDTSim::error("(ddtsim) Rank %i can't recover from VisIt/libsim error (code=%i)!\n",visitSim.rank(),visitstate);
            err = 1;
        }
        else if (visitstate == 0)
        {
            /* There was no input from VisIt, return control to sim. */
            return;
        }
        else if (visitstate == 1)
        {
            /* VisIt is trying to connect to sim */
            int connected = libsim.AttemptToCompleteConnection();
            const char* error = libsim.GetLastError();

            if (isParallel())
                ddtMpiCompat_Bcast(&connected, 1, DDTSIM_MPI_INT, 0, DDTSIM_MPI_COMM_WORLD);

            if (connected)
            {
                visitSim.setAutoupdate(true);
                libsim.SetSlaveProcessCallback(SlaveProcessCallback);

                /* Register command callback */
                libsim.SetCommandCallback(ControlCommandCallback,(void*)&pointers);

                /* Register data access callbacks */
                libsim.SetGetDomainList(ddtSimGetDomainList, (void*)&pointers);
                libsim.SetGetMetaData(ddtSimGetMetaData, (void*)&pointers);
                libsim.SetGetMesh(ddtSimGetMesh, (void*)&pointers);
                libsim.SetGetVariable(ddtSimGetVariable, (void*)&pointers);
            }
            else
                DDTSim::error("(ddtsim) Rank %i: VisIt did not connect: %s\n",visitSim.rank(), error);
        }
        else if (visitstate == 2)
        {
            /* VisIt wants to tell the us something */
            visitSim.setRunmode(VISIT_SIMMODE_STOPPED);
            if (!processVisItCommand())
            {
                /* Disconnect on error or closed connection */
                libsim.Disconnect();
                /* Stop running if VisIt closes */
                visitSim.setRunmode(VISIT_SIMMODE_STOPPED);
                ddtsim_interface.break_to_ddt = true;
                break;  // Break out of while loop
            }
        }
    } while (!err);
}

/******* VisIt insertion hooks ****************************************/

//! Signal handler. Captures SIGPROF signals and does VisIt program loop
void
sigprof_handler(int)
{
    if (!DDTSim::initialised())
    {
        // First call of signal handler is used for setup.
        DDTSim::init();
    }
    else
    {
        DDTSim::getInstance()->update(ddtsim_interface);
        DDTSim::getInstance()->visitloop();
    }
}

//! Initialise libsim and attach signal handler. Use for single-process jobs.
void
DDTSIM_Init(const char *progName, const char *progPath,
            const char *ddtSharedPath, const char *visitPath,
            const char *visitArchPath)
{
    ddtsim_interface_init();

    // Set up signal handler
    struct sigaction new_action;

    new_action.sa_handler = sigprof_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGPROF, &new_action, NULL);
    DDTSim::setMpi(NOMPI);
    DDTSim::setProgram(progName, progPath);
    DDTSim::setVisItPaths(visitPath,visitArchPath);
    DDTSim::setSharedPath(ddtSharedPath);
}

//! Initialise libsim and attach single handler. Use for MPI jobs.
void
DDTSIM_MPI_Init(const char *progName, const char *progPath,
                const char *ddtSharedPath, const char *visitPath,
                const char *visitArchPath, const char *mpiName)
{
    DDTSIM_Init(progName, progPath, ddtSharedPath, visitPath, visitArchPath);
    if (strncmp(mpiName,"openmpi",7)==0)
        DDTSim::setMpi(OPENMPI);
    else if (strncmp(mpiName,"smp-mpich",9)==0)
        DDTSim::setMpi(MPICH2);
    else if (strncmp(mpiName,"craycnl",7)==0)
        DDTSim::setMpi(MPT3);
    else
    {
        DDTSim::error("(ddtsim) Unsupported MPI implementation: %s\n",mpiName);
        exit(1); // HOOKS_IGNORE
    }
}
