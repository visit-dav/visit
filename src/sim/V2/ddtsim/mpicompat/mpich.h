// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MPICH_H
#define MPICH_H

void mpicompat_mpich_init();
int mpicompat_mpich_Bcast(void *buffer, int count, int type, int root);
void mpicompat_mpich_rankAndSize(int *rank, int *size);

#endif // OPENMPI_H
