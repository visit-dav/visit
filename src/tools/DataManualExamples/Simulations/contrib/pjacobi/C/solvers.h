#ifndef _SOLVERS_H_INCLUDED_
#define _SOLVERS_H_INCLUDED_

extern int m, mp, iter, par_rank, par_size, below, above, runMode;
extern double gdel, *v, *vnew;
extern float *cx, *cy;

#define INCREMENT 100       /* number of steps between convergence check  */
#define MAXSTEPS 50000      /* Maximum number of iterations               */
#define TOL 0.000001         /* Numerical Tolerance */

void set_initial_bc();
double update_jacobi();
void simulate_one_timestep();
#ifdef PARALLEL
void update_bc_2();
void neighbors();
void MPIIOWriteData(char *filename);
#endif

#define SIM_STOPPED       0
#define SIM_RUNNING       1
#endif
