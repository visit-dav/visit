/*
 *  This file is part of Cognomen.
 *
 *  Cognomen is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Cognomen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Cognomen.  If not, see <http://www.gnu.org/licenses/>.
 *****
 * Main library implementation.
 *****/
#include "cognomen.h"

/** Needed for gethostid on some systems. */
#ifndef __USE_BSD
#   define __USE_BSD
#endif
#include <assert.h>
#include <stdlib.h>

#include <mpi.h>

/** Map which associates MPI ranks with host ids.  Element `x' is the host id
 * for rank `x'. */
static long *map_id = NULL;

static int mpi_size();
static int mpi_rank();
static void *xmalloc(size_t bytes);

/** Initialization for the Cognomen library.
 * identify must be called by all nodes in lockstep.  It initializes
 * the library and performs node identification.  You only need to do this once
 * per process (run). */
void
cog_identify()
{
    int sz;
    long id;
    const int rank = mpi_rank();

    id = gethostid();
    sz = mpi_size();

    if(map_id) {
        free(map_id);
    }
    map_id = (long *) xmalloc(sizeof(long) * sz);

    for(int proc=0; proc < sz; ++proc) {
        if(proc == rank) {
            MPI_Bcast(&id, 1, MPI_LONG, proc, MPI_COMM_WORLD);
            map_id[proc] = id;
        } else {
            long recv;
            MPI_Bcast(&recv, 1, MPI_LONG, proc, MPI_COMM_WORLD);
            map_id[proc] = recv;
        }
    }
}

/** Returns the cognomen identifier for this process.  All processes on the
 * same node will receive the same identifier. */
void
cog_me(cog_id * const id)
{
    if(NULL == map_id) {
        return;
    }
    id->id = map_id[mpi_rank()];
}

/** Fill a set with the ranks of processes local to the given process. */
void
cog_set_local(cog_set * const local, const int rank)
{
    long hostid = map_id[rank];

    /* first we need to know how big their set should be. */
    size_t n_local=0;
    for(int i=0; i < mpi_size(); ++i) {
        if(map_id[i] == hostid) {
            ++n_local;
        }
    }
    local->set.v = (cog_id *) xmalloc(sizeof(cog_id) * n_local);
    local->set.size = n_local;

    /* Now we can copy all of the ids into the set. */
    size_t count=0;
    for(int i=0; i < mpi_size(); ++i) {
        if(map_id[i] == hostid) {
            cog_id v;
            v.id = i;
            local->set.v[count++] = v;
        }
    }
    assert(count == n_local);
}

/** @return the minimum rank defined in the given set */
int
cog_set_min(const cog_set * const cset)
{
    /* We might be able to rely on the set being ordered ...
     * For now we'll just search, though. */
    int min = INT_MAX;
    for(size_t i=0; i < cset->set.size; ++i) {
        if(cset->set.v[i].id < min) {
            min = cset->set.v[i].id;
        }
    }
    return min;
}

/** @return the maximum rank defined in the given set */
int
cog_set_max(const cog_set * const cset)
{
    /* Likewise to _min, it seems like we have an ordering guarantee we could
     * optimize this with.  Ignore for now .. */
    int max = -1;
    for(size_t i=0; i < cset->set.size; ++i) {
        if(cset->set.v[i].id > max) {
            max = cset->set.v[i].id;
        }
    }
    return max;
}

/** @return true if the given rank falls in the given set. */
bool
cog_set_intersect(const cog_set * const cset, int id)
{
    /* Essentially a search for the given rank in a set. */
    for(size_t i=0; i < cset->set.size; ++i) {
        if(cset->set.v[i].id == id) {
            return true;
        }
    }
    return false;
}

static int
mpi_size()
{
    int sz;
    MPI_Comm_size(MPI_COMM_WORLD, &sz);
    return sz;
}

static int
mpi_rank()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}

static void *
xmalloc(size_t bytes)
{
    void *x = malloc(bytes);
    if(NULL == x) {
        abort();
    }
    return x;
}
