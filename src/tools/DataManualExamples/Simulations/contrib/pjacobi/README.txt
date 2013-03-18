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

The CMakeLists.txt files include a reference to your Visit install directory,
which is found under $VISITHOME. Make sure this is correctly set.
Cmake should be able to detect automatically
which MPI compilers to use for Fortran and C.

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
mpiexec -n 4 python ./jacobi_insitu.py

----------------------------------
These are examples are provided by:
Last updated: Sun Mar 17 12:28:31 CET 2013
Jean M. Favre of the Swiss National Supercomputing Center, and
Brad Whitlock from Lawrence Livermore National Laboratory.
