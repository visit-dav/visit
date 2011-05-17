/*
this file uses original content I found on the Web. Orginal author is K.T (see below)

*/
/********** U T I L I T Y ************************************
 * Utility functions for use with the Jacobi and SOR solvers *
 * Kadin Tseng, Boston University, November 1999             *
 *************************************************************/

#include "solvers.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif
void set_initial_bc()
{
/*********** Boundary Conditions ****************************************
 *  PDE: Laplacian u = 0;      0<=x<=1;  0<=y<=1                        *
 *  B.C.: u(x,0)=sin(pi*x); u(x,1)=sin(pi*x)*exp(-pi); u(0,y)=u(1,y)=0  *
 *  SOLUTION: u(x,y)=sin(pi*x)*exp(-pi*y)                               *
 ************************************************************************/
  int i;
  double x;
  memset(v, 0, sizeof(double)*(m+2)*(mp+2));

  if(par_size > 1)
    {
    if (par_rank == 0)
      {
      for (i = 0; i < (m+2); i++)
        {
        v[i] = sin(M_PI*i/(m+1));              /* at y = 0; all x */
        }
      }
    if (par_rank == par_size-1) {
      for (i = 0; i < (m+2); i++) {
        v[i+(m+2)*(mp+1)] = sin(M_PI*i/(m+1))*exp(-M_PI);   /* at y = 1; all x */
      }
    }
  }
  else //  single -proc version
    {
    for (i = 0; i < (m+2); i++)
      {
      x = sin(M_PI*i/(m+1));    /* at y = 0; all x */
      v[i] = x;    /* at y = 0; all x */
      v[i+(m+2)*(mp+1)] = x*exp(-M_PI);   /* at y = 1; all x */
      }
    }
}

void simulate_one_timestep()
{
/* compute new solution according to the Jacobi scheme */
  double del = update_jacobi();
  /* find global max error */
#ifdef PARALLEL
  MPI_Allreduce( &del, &gdel, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD );
#else
  gdel = del;
#endif
  
  if(iter%INCREMENT == 0)  // only print global error every N steps
    {
    if(!par_rank)
      {
      fprintf(stdout,"iter,del,gdel: %6d, %lf %lf\n",iter,del,gdel);
      }
    }
#ifdef PARALLEL
  update_bc_2(); // update boundary conditions
#endif
  iter++;
#ifdef _VISIT_
  VisItTimeStepChanged();
  VisItUpdatePlots();
#endif
}

double update_jacobi()
{
  int i, j;
  double del = 0.0;

  for (j = 1; j < mp+1; j++)
    {
    for (i = 1; i < m+1; i++)
      {
      vnew[i+(m+2)*j] = ( v[i+(m+2)*(j+1)] +
                                v[(i+1)+(m+2)*j] +
                                v[(i-1)+(m+2)*j] +
                                v[i+(m+2)*(j-1)] )*0.25;
      del += fabs(vnew[i+(m+2)*j] - v[i+(m+2)*j]); /* find local max error */
      }
    }
  for (j = 1; j < (mp+1); j++)
    {
    for (i = 1; i < (m+1); i++)
      {
      v[i+(m+2)*j] = vnew[i+(m+2)*j];
      }
  }
      
  return del;
}

#ifdef PARALLEL
void update_bc_2()
{
  MPI_Status status;
// send my last computed row above and receive from below my south boundary wall
  MPI_Sendrecv(&v[1+mp*(m+2)], m, MPI_DOUBLE, above, 0,
               &v[1+0*(m+2)], m, MPI_DOUBLE, below, 0,
                MPI_COMM_WORLD, &status );
// send my first computed row below and receive from above my north boundary wa
  MPI_Sendrecv(&v[1 + 1*(m+2) ], m, MPI_DOUBLE, below, 1,
               &v[1+(mp+1)*(m+2)], m, MPI_DOUBLE, above, 1,
                MPI_COMM_WORLD, &status );
}

void neighbors()
{
  if(par_rank == 0) {
    below = MPI_PROC_NULL;     /* tells MPI not to perform send/recv */
    above = par_rank+1;
  } else if(par_rank == par_size-1) {
    below = par_rank-1;
    above = MPI_PROC_NULL;     /* tells MPI not to perform send/recv */
  } else {
    below = par_rank-1;
    above = par_rank+1;
  }
}

void MPIIOWriteData(char *filename)
{
  // global size of array on disk
  int dimuids[2]={m+2, m+2};
  int ustart[2], ucount[2];
  int disp = 0;

  MPI_File      filehandle;
  MPI_Datatype  filetype;

  MPI_File_open(MPI_COMM_WORLD, filename,
                          MPI_MODE_CREATE | MPI_MODE_WRONLY,
                          MPI_INFO_NULL, &filehandle);

  if(par_rank == 0)
    {
    MPI_File_write(filehandle, dimuids, 2, MPI_INT, MPI_STATUS_IGNORE);
    }
  disp = 2 * sizeof(int); // offset because we just wrote 2 integers
  ustart[0] = mp * par_rank + 1;
  if(par_rank == 0)
    {
  ustart[0] = 0;
    }
  ustart[1] = 0; // X direction

  ucount[0] = mp;
  ucount[1] = m+2;
// all tasks write mp lines except rank 0 and last rank which write (mp+1)
// in total, we have  the (m+2)*(m+2) grid written, including the b.c.
  if((par_rank == 0) || (par_rank == (par_size-1)))
    {
    ucount[0]++;
    }

 // Create the subarray representing the local block
  MPI_Type_create_subarray(2, dimuids, ucount, ustart,
                                        MPI_ORDER_C, MPI_DOUBLE, &filetype);
  MPI_Type_commit(&filetype);

  MPI_File_set_view(filehandle, disp, MPI_DOUBLE,
                                filetype, "native", MPI_INFO_NULL);

  MPI_File_write_all(filehandle, v, ucount[0]*ucount[1], MPI_DOUBLE, MPI_STATUS_IGNORE);
  MPI_File_close(&filehandle);
  MPI_Type_free(&filetype);

}

#endif
