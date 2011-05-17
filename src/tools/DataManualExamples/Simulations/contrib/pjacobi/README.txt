This set of examples is contributed to demonstrate the VisIt
in-situ coupling of a simple Jacobi solver for Laplace's Equation
in 2D.

Three sub-directories are provided to show the implementation
with C, FORTRAN 90, and Python

The examples are focused on parallel execution, as they also demonstrate
how ghost-cells are handled by VisIt and libsim.

--------------- How to compile ----------------------------------------------
in the F90 directory, you need a symbolic link to your source directory for
/path-to-a-file/src/tools/DataManualExamples/Simulations/simulationexamplev2.f

in the C directory, you need a symbolic link to your source directory for
/path-to-a-file/src/tools/DataManualExamples/Simulations/SimulationExample.h

The CMakeLists.txt files should be modified to include the correct
path to your VisIt installation. Make sure to choose a parallel C and
FORTRAN compilers. You can initialize ccmake with

ccmake -DCMAKE_Fortran_COMPILER=mpif90 .

and

ccmake -DCMAKE_C_COMPILER=mpicc .

A single compile-time flag is used to enable compilation with in-situ coupling.
The makefiles generate two executables, a standalone MPI program, and an in-situ
enabled counter-part. If you look at the source code, you would clearly see what
instrumentation steps are required to enable in-situ. The source code is flagged by

#ifdef _VISIT_
#else
#endif

A simple MPI-IO support is added to dump solutions files. The 2-D grid data
can then simply be loaded in VisIt with the included BOV-format template.
See file jacobi.bov


in the Python directory, just run
mpiexec -n 4 ./jacobi_insitu.py

----------------------------------
These are examples are provided by:
Last updated: Mon May 16 2011
Jean M. Favre of the Swiss National Supercomputing Center, and
Brad Whitlock from Lawrence Livermore National Laboratory.
