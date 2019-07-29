// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "mpich.h"
#include "mpicompat/mpich2/mpi.h"
#include "mpicompat.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static void* DDTSIM_dlhandle = NULL;
static MPI_Comm DDTSIM_mpi_comm_world;
static MPI_Datatype DDTSIM_mpi_int;
static MPI_Datatype DDTSIM_mpi_char;


void mpicompat_mpich_init()
{
    DDTSIM_mpi_comm_world = MPI_COMM_WORLD;
    DDTSIM_mpi_int = MPI_INT;
    DDTSIM_mpi_char = MPI_CHAR;
}

int mpicompat_mpich_Bcast(void *buffer, int count, int type, int root)
{
    int (*fptr)(void*,int,MPI_Datatype,int,MPI_Comm) =
            (int (*)(void*,int,MPI_Datatype,int,MPI_Comm)) dlsym(DDTSIM_dlhandle,"MPI_Bcast");

    if (fptr == NULL)
    {
        fprintf(stderr,"(ddtsim) Unable to load MPI_Bcast command: %s",dlerror());
        exit(1); // HOOKS_IGNORE
    }
    
    MPI_Datatype mpitype;
    switch(type)
    {
    case DDTSIM_MPI_INT:  mpitype = DDTSIM_mpi_int;  break;
    case DDTSIM_MPI_CHAR: mpitype = DDTSIM_mpi_char; break;
    default:
        fprintf(stderr,"(ddtsim) Unexpected use of ddtSim_Bcast with unhandled datatype\n");
        exit(1);  // HOOKS_IGNORE
    }

    return fptr(buffer,count,mpitype,root,DDTSIM_mpi_comm_world);
}

void mpicompat_mpich_rankAndSize(int *rank, int *size)
{
    ((int(*)(MPI_Comm,int*)) dlsym(DDTSIM_dlhandle,"MPI_Comm_rank"))(DDTSIM_mpi_comm_world,rank);
    ((int(*)(MPI_Comm,int*)) dlsym(DDTSIM_dlhandle,"MPI_Comm_size"))(DDTSIM_mpi_comm_world,size);
}
