#include "solvers.h"
#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

#ifdef PARALLEL
int visit_broadcast_int_callback(int *value, int sender)
{
  return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
}

int visit_broadcast_string_callback(char *str, int len, int sender)
{
  return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
#endif

const char *cmd_names[] = {"halt", "step", "run", "update"};

visit_handle
SimGetMetaData(void *cbdata)
{
  visit_handle md = VISIT_INVALID_HANDLE;

  if(VisIt_SimulationMetaData_alloc(&md) == VISIT_OKAY)
    {
    int i;
    double simtime = iter * 0.1;
    visit_handle m1 = VISIT_INVALID_HANDLE;
    visit_handle vmd = VISIT_INVALID_HANDLE;
    visit_handle cmd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
    VisIt_SimulationMetaData_setMode(md, (runMode == SIM_STOPPED) ?
                VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
    VisIt_SimulationMetaData_setCycleTime(md, iter, simtime);

        /* Set the first mesh's properties.*/
    if(VisIt_MeshMetaData_alloc(&m1) == VISIT_OKAY)
      {
      /* Set the mesh's properties.*/
      VisIt_MeshMetaData_setName(m1, "mesh");
      VisIt_MeshMetaData_setMeshType(m1, VISIT_MESHTYPE_RECTILINEAR);
      VisIt_MeshMetaData_setTopologicalDimension(m1, 2);
      VisIt_MeshMetaData_setSpatialDimension(m1, 2);
      VisIt_MeshMetaData_setNumDomains(m1, par_size);
      VisIt_MeshMetaData_setXUnits(m1, "cm");
      VisIt_MeshMetaData_setYUnits(m1, "cm");
      VisIt_MeshMetaData_setXLabel(m1, "Width");
      VisIt_MeshMetaData_setYLabel(m1, "Height");
    
      VisIt_SimulationMetaData_addMesh(md, m1);
      }

      /* Add a zonal scalar variable on mesh. */
    if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
      {
      VisIt_VariableMetaData_setName(vmd, "temperature");
      VisIt_VariableMetaData_setMeshName(vmd, "mesh");
      VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
      VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
      VisIt_SimulationMetaData_addVariable(md, vmd);
      }

      /* Add some custom commands. */
    for(i = 0; i < sizeof(cmd_names)/sizeof(const char *); ++i)
      {
      visit_handle cmd = VISIT_INVALID_HANDLE;
      if(VisIt_CommandMetaData_alloc(&cmd) == VISIT_OKAY)
        {
        VisIt_CommandMetaData_setName(cmd, cmd_names[i]);
        VisIt_SimulationMetaData_addGenericCommand(md, cmd);
        }
      }
    }

  return md;
}

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
  visit_handle h = VISIT_INVALID_HANDLE;

  if(strcmp(name, "mesh") == 0)
    {
    if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
      {
      int minRealIndex[3], maxRealIndex[3];
      minRealIndex[0] = 0;
      minRealIndex[1] = 0;
      minRealIndex[2] = 0;
      maxRealIndex[0] = m+1;
      maxRealIndex[1] = mp;
      maxRealIndex[2] = 0;
      if(par_rank == (par_size-1))
        maxRealIndex[1]++;

      visit_handle hxc, hyc;
      VisIt_VariableData_alloc(&hxc);
      VisIt_VariableData_alloc(&hyc);
      VisIt_VariableData_setDataF(hxc, VISIT_OWNER_SIM, 1, m+2, cx);
      VisIt_VariableData_setDataF(hyc, VISIT_OWNER_SIM, 1, mp+2, cy);
      VisIt_RectilinearMesh_setCoordsXY(h, hxc, hyc);

      VisIt_RectilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex);
      }
    }
  return h;
}

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
  visit_handle h = VISIT_INVALID_HANDLE;
  int nComponents = 1, nTuples;

  if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
    if(strcmp(name, "temperature") == 0)
      {
      nTuples = (m+2)*(mp+2);
      VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, nComponents, nTuples, Temp);
      }
    else
      {
      VisIt_VariableData_free(h);
      h = VISIT_INVALID_HANDLE;
      }
    }
  return h;
}

visit_handle
SimGetDomainList(const char *name, void *cbdata)
{
  visit_handle h = VISIT_INVALID_HANDLE;
  if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
    {
    visit_handle hdl;
    int i, *iptr = NULL;

    iptr = (int *)malloc(sizeof(int));
    *iptr = par_rank;

    if(VisIt_VariableData_alloc(&hdl) == VISIT_OKAY)
      {
      VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, iptr);
      VisIt_DomainList_setDomains(h, par_size, hdl);
      }
    }
  return h;
}

/* Helper function for ProcessVisItCommand */
void BroadcastSlaveCommand(int *command)
{
#ifdef PARALLEL
  MPI_Bcast(command, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
}

/* Callback involved in command communication. */
void SlaveProcessCallback(void)
{
   int command = VISIT_COMMAND_PROCESS;
   BroadcastSlaveCommand(&command);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand()
{
    int command;
    if (par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success == VISIT_OKAY)
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
        /* Note: only through the SlaveProcessCallback callback
         * above can the rank 0 process send a VISIT_COMMAND_PROCESS
         * instruction to the non-rank 0 processes. */
        while (1)
        {
            BroadcastSlaveCommand(&command);
            switch (command)
            {
            case VISIT_COMMAND_PROCESS:
                VisItProcessEngineCommand();
                break;
            case VISIT_COMMAND_SUCCESS:
                return 1;
            case VISIT_COMMAND_FAILURE:
                return 0;
            }
        }
    }
}

void ControlCommandCallback(const char *cmd, const char *args, void *cbdata)
{
    if(strcmp(cmd, "halt") == 0)
        runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
      {
      simulate_one_timestep();
      VisItTimeStepChanged();
      VisItUpdatePlots();
      }
    else if(strcmp(cmd, "run") == 0)
      runMode = SIM_RUNNING;
    else if(strcmp(cmd, "update") == 0)
      {
      VisItTimeStepChanged();
      VisItUpdatePlots();
      }
}
