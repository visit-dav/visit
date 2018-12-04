#ifndef _SOLVERS_H_INCLUDED_
#define _SOLVERS_H_INCLUDED_

extern int m, mp, iter, par_rank, par_size, below, above, runMode;
extern double gdel, *oldTemp, *Temp;
extern float *cx, *cy;

#define INCREMENT 50       /* number of steps between convergence check  */
#define MAXSTEPS 50000     /* Maximum number of iterations               */
#define TOL 1e-04          /* Numerical Tolerance */

void set_initial_bc();
double update_jacobi();
void simulate_one_timestep();
void CopyTempValues_2_OldValues();
#ifdef PARALLEL
void exchange_ghost_lines();
void neighbors();
void MPIIOWriteData(char *filename);
#endif

#define SIM_STOPPED       0
#define SIM_RUNNING       1
#endif
