/*
 A parallel Jacobi solver for the Laplacian equation in 2D
 Written by Jean M. Favre, Swiss National Supercomputing Center
 May 16, 2011
 Code inspired from an older example by Kadin Tseng, Boston University, November 1999
 The compile flag -D_VISIT_ enables compilation with VisIt. Otherwise, the pogram runs
 in stand-alone mode
*/
#include "solvers.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#ifdef _VISIT_
#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
#include "SimulationExample.h"

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);
void ControlCommandCallback(const char *cmd, const char *args, void *cbdata);
void SlaveProcessCallback();
#ifdef PARALLEL
int visit_broadcast_int_callback(int *value, int sender);
int visit_broadcast_string_callback(char *str, int len, int sender);
#endif
#endif

int iter, par_rank, par_size, below, above, runMode;
int m, mp;
double gdel, *v, *vnew;
float *cx, *cy;

int main(int argc, char *argv[])
{
  int i;
  int blocking, visitstate, err = 0;
  par_rank=0; par_size = 1;

#ifdef PARALLEL
  MPI_Init(&argc, &argv);                       /* starts MPI */
  MPI_Comm_rank(MPI_COMM_WORLD, &par_rank); /* get current process id */
  MPI_Comm_size(MPI_COMM_WORLD, &par_size); /* get # procs from env or */
  neighbors();
#endif

#ifdef _VISIT_
  SimulationArguments(argc, argv);
  VisItSetupEnvironment();

#ifdef PARALLEL
  VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
  VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
  VisItSetParallel(par_size > 1);
  VisItSetParallelRank(par_rank);
#endif
  if(!par_rank)
    {
    VisItInitializeSocketAndDumpSimFile(
#ifdef PARALLEL
            "pjacobi",
#else
            "jacobi",
#endif
            "Jacobi solver for Laplace Equation",
            "/path/to/where/sim/was/started",
            NULL, NULL, NULL);
    }

#endif

  runMode = 0;
  m = 20; // mesh size = (m+2)x(m+2) including the bc grid lines
// We make no attempt to check that the number of grid points divides evenly
// with the number of MPI tasks.
// rank 0 will display the bottom (southern) boundary wall
// rank (size-1) will display the top (northern) boundary wall
// if run with m=20 and 4 MPI tasks, we will have 5 grid lines per rank
// and VisIt will display a 22x22 grid
#ifdef PARALLEL
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
// We use (m + 2) grid points in the X direction, i.e. interior points plus 2 b.c. points
  mp = m/par_size;
// We use (mp + 2) grid points in the Y direction, i.e. interior points plus 2 b.c. points

  v  = (double *)malloc((mp + 2)* (m + 2) * sizeof(double));
  vnew = (double *)malloc((mp + 2) * (m + 2) * sizeof(double));
  cx = (float *)malloc(sizeof(float) * (m + 2));
  cy = (float *)malloc(sizeof(float) * (mp + 2));

  float hsize = 1.0/(m+1.0);

  for(i = 0; i < (m + 2); i++)
    cx[i] = i * hsize;
  for(i = 0; i < (mp + 2); i++)
    cy[i] = (i + par_rank*mp) * hsize;

  gdel = 1.0;
  iter = 0;
  set_initial_bc();

  if(runMode == SIM_STOPPED)
    simulate_one_timestep();

  while (gdel > TOL)
    {  // iterate until error below threshold
    if(iter > MAXSTEPS)
      {
      fprintf(stdout,"Iteration terminated (exceeds %6d", MAXSTEPS);
      fprintf(stdout," )\n");
      break;       /* nonconvergent solution */
      }
#ifdef _VISIT_
    blocking = (runMode == VISIT_SIMMODE_RUNNING) ? 0 : 1;

        /* Get input from VisIt or timeout so the simulation can run. */
    if(par_rank == 0)
      visitstate = VisItDetectInput(blocking, -1);
#ifdef PARALLEL
    MPI_Bcast(&visitstate, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
        /* Do different things depending on the output from VisItDetectInput. */
    switch(visitstate)
      {
      case 0:
        /* There was no input from VisIt, return control to sim. */
        simulate_one_timestep();
      break;
      case 1:
            /* VisIt is trying to connect to sim. */
        if(VisItAttemptToCompleteConnection() == VISIT_OKAY)
          {
          if(!par_rank)
            {
            fprintf(stderr, "VisIt connected\n");
            }
          VisItSetCommandCallback(ControlCommandCallback, (void*)NULL);
          VisItSetSlaveProcessCallback(SlaveProcessCallback);

          VisItSetGetMetaData(SimGetMetaData, NULL);
          VisItSetGetMesh(SimGetMesh, NULL);
          VisItSetGetVariable(SimGetVariable, NULL);
          VisItSetGetDomainList(SimGetDomainList, NULL);
          }
        else 
          {
           /* Print the error message */
          char *err = VisItGetLastError();
          fprintf(stderr, "VisIt did not connect: %s\n", err);
          free(err);
          }
      break;
      case 2:
        /* VisIt wants to tell the engine something. */
        if(!ProcessVisItCommand())
          {
          /* Disconnect on an error or closed connection. */
          VisItDisconnect();
          /* Start running again if VisIt closes. */
          runMode = SIM_RUNNING;
          }
      break;
      default:
        fprintf(stderr, "Can't recover from error %d!\n", visitstate);
      break;
      }
#else
    simulate_one_timestep();
#endif
    }
  if (!par_rank)
    {
    fprintf(stdout,"Stopped at iteration %d\n",iter);
    fprintf(stdout,"The maximum error = %f\n",gdel);
    }

#ifdef PARALLEL
  MPIIOWriteData("/tmp/Jacobi.bin");

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
#endif
  free(v); free(vnew);
  free(cx); free(cy);
  return (0);
}




