/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <avtIsenburgSGG.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtParallel.h>

#include <DebugStream.h>


// ****************************************************************************
//  Constructor:  avtIsenburgSGG::avtIsenburgSGG
//
//  Programmer:  Hank Childs
//  Creation:    February 10, 2008
//
// ****************************************************************************

avtIsenburgSGG::avtIsenburgSGG()
{
    ghost3dmodule = new Ghost3Dmodule;
    blocks        = NULL;
    numDomains    = 0;
}


// ****************************************************************************
//  Destructor:  avtIsenburgSGG::~avtIsenburgSGG
//
//  Programmer:  Hank Childs
//  Creation:    February 10, 2008
//
// ****************************************************************************

avtIsenburgSGG::~avtIsenburgSGG()
{
    delete ghost3dmodule;
    delete [] blocks;
}


// ****************************************************************************
//  Method: avtIsenburgSGG::SetNumberOfDomains
//
//  Purpose:
//      Declares how many domains there will be.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

void
avtIsenburgSGG::SetNumberOfDomains(int nd)
{
    numDomains = nd;
    blocks = new Ghost3Dblock[numDomains];
}


// ****************************************************************************
//  Method: avtIsenburgSGG::SetInfoForDomain
//
//  Purpose:
//      Declares information for a given domain.
//
//  Arguments:
//      index   The index of the domain.
//      ori     The origin of the domain.
//      sz      The size of the domain.
//      nei     The neighbors of the domain.  First entry is the index of
//              the min-I neighbor, second entry is the max-I, etc.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

void
avtIsenburgSGG::SetInfoForDomain(int index, int *ori, int *sz,
                                 int *nei)
{
    blocks[index].id           = index;
    blocks[index].proc_id      = 0;
    blocks[index].origin[0]    = ori[0];
    blocks[index].origin[1]    = ori[1];
    blocks[index].origin[2]    = ori[2];
    blocks[index].size[0]      = sz[0];
    blocks[index].size[1]      = sz[1];
    blocks[index].size[2]      = sz[2];
    blocks[index].neighbors[0] = (nei[0] < 0 ? NULL : blocks + nei[0]);
    blocks[index].neighbors[1] = (nei[1] < 0 ? NULL : blocks + nei[1]);
    blocks[index].neighbors[2] = (nei[2] < 0 ? NULL : blocks + nei[2]);
    blocks[index].neighbors[3] = (nei[3] < 0 ? NULL : blocks + nei[3]);
    blocks[index].neighbors[4] = (nei[4] < 0 ? NULL : blocks + nei[4]);
    blocks[index].neighbors[5] = (nei[5] < 0 ? NULL : blocks + nei[5]);
}


// ****************************************************************************
//  Method: avtIsenburgSGG::FinalizeDomainInformation
//
//  Purpose:
//      This method indicates that all of the domain information has been set
//      up and that the internal data structures should be assembled.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

void
avtIsenburgSGG::FinalizeDomainInformation(void)
{
    int sizeOfData = 4; // single float

    // HACK: Reproducing the load balancing taking place in "LoadBalancer" here.
    int nProcs = PAR_Size();
    int amountPer = numDomains / nProcs;
    int oneExtraUntil = numDomains % nProcs;
    int domain = 0;
    for (int i = 0 ; i < nProcs ; i++)
    {
        int amount = amountPer + (i < oneExtraUntil ? 1 : 0);
        for (int j = 0 ; j < amount ; j++)
        {
            blocks[domain++].proc_id = i;
        }
    }

#ifdef PARALLEL
    ghost3dmodule->init(VISIT_MPI_COMM, blocks, numDomains, sizeOfData, 0);
#else
    ghost3dmodule->init(blocks, numDomains, sizeOfData);
#endif
}


// ****************************************************************************
//  Method: avtIsenburgSGG::GetNextDomain
//
//  Purpose:
//      A virtual method that declares which domain should be processed next.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

int
avtIsenburgSGG::GetNextDomain(void)
{
    curBlock = ghost3dmodule->selectBlock();
    return curBlock;
}


// ****************************************************************************
//  Method: avtIsenburgSGG::StreamDataset
//
//  Purpose:
//      The method that gives the current domain to the module and then
//      creates the output, which may have more or less data.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

vtkDataSet *
avtIsenburgSGG::StreamDataset(vtkDataSet *ds)
{
    int  i, j, k;
    double pt1[3];
    double pt2[3];
    int dims[3];
    vtkRectilinearGrid *rg_in = (vtkRectilinearGrid *) ds;
    rg_in->GetDimensions(dims);
    vtkDataArray *X_in = rg_in->GetXCoordinates();
    vtkDataArray *Y_in = rg_in->GetYCoordinates();
    vtkDataArray *Z_in = rg_in->GetZCoordinates();

    Ghost3Dblock *block = blocks + curBlock;
    int origin_out[3];
    int size_out[3];
    int sz = ds->GetNumberOfPoints();
    
    vtkDataArray *in_array  = rg_in->GetPointData()->GetScalars();
    void *data = in_array->GetVoidPointer(0);
    unsigned char *data_out = ghost3dmodule->processBlock(
                                  (unsigned char*) data, origin_out, size_out);

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(size_out[0], size_out[1], size_out[2]);

    // Set up X
    vtkFloatArray *X = vtkFloatArray::New();
    X->SetNumberOfTuples(size_out[0]);
    int offset = 0;
    if (origin_out[0] < block->origin[0])
    {
        ds->GetPoint(0, pt1);
        ds->GetPoint(1, pt2);
        int numExtra = block->origin[0] - origin_out[0];
        for (i = 0 ; i < numExtra ; i++)
        {
            X->SetTuple1(offset, pt1[0] - (numExtra-i)*(pt2[0]-pt1[0]));
            offset++;
        }
    }
    for (i = 0 ; i < dims[0] ; i++)
    {
        X->SetTuple1(offset+i, X_in->GetTuple1(i));
    }
    offset += dims[0];
    if (offset < size_out[0])
    {
        ds->GetPoint(dims[0]-2, pt1);
        ds->GetPoint(dims[0]-1, pt2);
        int numExtra = size_out[0] - offset;
        for (i = 0 ; i < numExtra ; i++)
        {
            X->SetTuple1(offset, pt2[0] + (i+1)*(pt2[0]-pt1[0]));
            offset++;
        }
    }
    rgrid->SetXCoordinates(X);
    X->Delete();

    // Set up Y
    vtkFloatArray *Y = vtkFloatArray::New();
    Y->SetNumberOfTuples(size_out[1]);
    offset = 0;
    if (origin_out[1] < block->origin[1])
    {
        ds->GetPoint(0, pt1);
        ds->GetPoint(dims[0], pt2);
        int numExtra = block->origin[1] - origin_out[1];
        for (i = 0 ; i < numExtra ; i++)
        {
            Y->SetTuple1(offset, pt1[1] - (numExtra-i)*(pt2[1]-pt1[1]));
            offset++;
        }
    }
    for (i = 0 ; i < dims[1] ; i++)
    {
        Y->SetTuple1(offset+i, Y_in->GetTuple1(i));
    }
    offset += dims[1];
    if (offset < size_out[1])
    {
        ds->GetPoint(dims[0]*dims[1] - dims[0] , pt1);
        ds->GetPoint(dims[0]*dims[1]-1, pt2);
        int numExtra = size_out[1] - offset;
        for (i = 0 ; i < numExtra ; i++)
        {
            Y->SetTuple1(offset, pt2[1] + (i+1)*(pt2[1]-pt1[1]));
            offset++;
        }
    }
    rgrid->SetYCoordinates(Y);
    Y->Delete();

    // Set up Z
    vtkFloatArray *Z = vtkFloatArray::New();
    Z->SetNumberOfTuples(size_out[2]);
    offset = 0;
    if (origin_out[2] < block->origin[2])
    {
        ds->GetPoint(0, pt1);
        ds->GetPoint(dims[0]*dims[1], pt2);
        int numExtra = block->origin[2] - origin_out[2];
        for (i = 0 ; i < numExtra ; i++)
        {
            Z->SetTuple1(offset, pt1[2] - (numExtra-i)*(pt2[2]-pt1[2]));
            offset++;
        }
    }
    for (i = 0 ; i < dims[2] ; i++)
    {
        Z->SetTuple1(offset+i, Z_in->GetTuple1(i));
    }
    offset += dims[2];
    if (offset < size_out[2])
    {
        ds->GetPoint(dims[0]*dims[1]*(dims[2]-1)-1, pt1);
        ds->GetPoint(dims[0]*dims[1]*dims[2] -1, pt2);
        int numExtra = size_out[2] - offset;
        for (i = 0 ; i < numExtra ; i++)
        {
            Z->SetTuple1(offset, pt2[2] + (i+1)*(pt2[2]-pt1[2]));
            offset++;
        }
    }
    rgrid->SetZCoordinates(Z);
    Z->Delete();

    // 
    // Set up the output variable.
    //
    vtkFloatArray *out_array = vtkFloatArray::New();
    int npts = rgrid->GetNumberOfPoints();
    out_array->SetNumberOfTuples(npts);
    void *ptr = out_array->GetVoidPointer(0);
    memcpy(ptr, data_out, 4*npts);
    delete [] data_out;
    out_array->SetName(in_array->GetName());
    rgrid->GetPointData()->AddArray(out_array);
    rgrid->GetPointData()->SetScalars(out_array);
    out_array->Delete();

    //
    // Set up the ghost data.
    //
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    int ncells = rgrid->GetNumberOfCells();
    int ndims[3] = { size_out[0]-1, size_out[1]-1, size_out[2]-1 };
    ghostCells->SetNumberOfTuples(ncells);
    for (i = 0 ; i < ncells ; i++)
        ghostCells->SetValue(i, '\0');
    unsigned char val = 0;
    avtGhostData::AddGhostZoneType(val, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    if (block->neighbors[0] != NULL)
    {
        for (k = 0 ; k < ndims[2] ; k++)
            for (j = 0 ; j < ndims[1] ; j++)
            {
                int idx = k*(ndims[1])*(ndims[0])+j*(ndims[0]);
                ghostCells->SetValue(idx, val);
            }
    }
/*
    if (block->neighbors[1] != NULL)
    {
        for (k = 0 ; k < ndims[2] ; k++)
            for (j = 0 ; j < ndims[1] ; j++)
            {
                int idx = k*(ndims[1])*(ndims[0])+j*(ndims[0])+ (ndims[0]-1);
                ghostCells->SetValue(idx, val);
            }
    }
 */
    if (block->neighbors[2] != NULL)
    {
        for (k = 0 ; k < ndims[2] ; k++)
            for (i = 0 ; i < ndims[0] ; i++)
            {
                int idx = k*(ndims[1])*(ndims[0])+(i);
                ghostCells->SetValue(idx, val);
            }
    }
/*
    if (block->neighbors[3] != NULL)
    {
        for (k = 0 ; k < ndims[2] ; k++)
            for (i = 0 ; i < ndims[0] ; i++)
            {
                int idx = k*(ndims[1])*(ndims[0])+(ndims[1]-1)*(ndims[0])+(i);
                ghostCells->SetValue(idx, val);
            }
    }
 */
    if (block->neighbors[4] != NULL)
    {
        for (j = 0 ; j < ndims[1] ; j++)
            for (i = 0 ; i < ndims[0] ; i++)
            {
                int idx = (j)*(ndims[0])+(i);
                ghostCells->SetValue(idx, val);
            }
    }
/*
    if (block->neighbors[5] != NULL)
    {
        for (j = 0 ; j < ndims[1] ; j++)
            for (i = 0 ; i < ndims[0] ; i++)
            {
                int idx = (ndims[2]-1)*(ndims[1])*(ndims[0])+(j)*(ndims[0])+(i);
                ghostCells->SetValue(idx, val);
            }
    }
 */
    rgrid->GetCellData()->AddArray(ghostCells);
    ghostCells->Delete();

    return rgrid;
}

/*
===============================================================================

  FILE:  ghost3dmodule.cpp
  
  CONTENTS:
  
    see header file
  
  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    copyright (C) 2007 martin isenburg@cs.unc.edu
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see header file
  
===============================================================================
*/

#define HAVE_NO_METIS

#define LOG_FILE_OUTPUT 0
#define DEBUG_OUTPUT 0

#define FACE_TAG 4711
#define REQUESTS_PER_PROC 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "ghost3dmodule.h"
//#include "ghost3dface.h"

extern "C" {
void METIS_PartMeshNodal(int *, int *, int *, int *, int *, int *, int *, int *, int *);
void METIS_PartMeshDual(int *, int *, int *, int *, int *, int *, int *, int *, int *);
void METIS_PartGraphRecursive(int*, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);
void METIS_PartGraphKway(int*, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);
void METIS_PartGraphVKway(int*, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);
};

#ifdef ENABLE_PARALLEL_MODE
bool Ghost3Dmodule::init(MPI_Comm comm, Ghost3Dblock* blocks, int nblocks, int dsize, int partition_method)
{
  int i,j,k,size;

  Ghost3Dblock* block;
  Ghost3Dblock* neighbor;

  // for statistics
  faces_size = 0;
  faces_maxsize = 0;
  
  // get our rank and the number of processors
  communicator = comm;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &nprocs);

  // the size of each field entry
  this->dsize = dsize;

  if (nprocs > 1)
  {
    if (partition_method) // partition the blocks
    {
      if (partition_method == 1) // with MeTiS
	partition_blocks(blocks, nblocks, nprocs, false);
      else if (partition_method == 2) // randomly
	partition_blocks_random(blocks, nblocks, nprocs);
      else if (partition_method == 3) // by slicing up the array
      partition_blocks_simple(blocks, nblocks, nprocs);
      else if (partition_method == 4) // with MeTiS (and weights)
	partition_blocks(blocks, nblocks, nprocs, true);
      else
      {
	fprintf(stderr, "ERROR: unknown partition_method %d\n", partition_method);
	return false;
      }
    }
    else // the user has partitioned the blocks and preset the proc_id's
    {
      for (i = 0; i < nblocks; i++)
      {
	if (blocks[i].proc_id < 0 || blocks[i].proc_id >= nprocs)
	{
	  fprintf(stderr, "ERROR: preset proc_id of block %d is %d (invalid)\n", i, blocks[i].proc_id);
	  return false;
	}
      }
    }
  }
  else
  {
    for (i = 0; i < nblocks; i++)
    {
      blocks[i].proc_id = 0;
    }  
  }

  // find out which blocks are ours, count them, put them in the right queue
  int* num_recv_from_proc = new int[nprocs];
  int* max_data_from_proc = new int[nprocs];
  for (i = 0; i < nprocs; i++)
  {
    num_recv_from_proc[i] = 0;
    max_data_from_proc[i] = 0;
  }

  // blocks are maintained in five queues of decreasing priority
  candidate_queue[0] = new my_queue();
  candidate_queue[1] = new my_queue();
  candidate_queue[2] = new my_queue();
  candidate_queue[3] = new my_queue();
  candidate_queue[4] = new my_queue();

  // the hash is used to map incoming faces to the right block
  recv_hash = new my_hash();

  // count number of blocks we own and faces we send/recv
  num_blocks = 0;
  num_processed_blocks = 0;
  num_boundary_faces_recv = 0;
  num_boundary_faces_send = 0;
  selected_block = 0;

  for (i = 0; i < nblocks; i++)
  {
    block = &(blocks[i]);
    // is this block on our processor
    if (block->proc_id == rank)
    {
      num_blocks++;
      for (k = 0; k < 6; k++)
      {
	neighbor = block->neighbors[k];
	// does this neighbor exist and is it on a different processor 
	if (neighbor && neighbor->proc_id != rank)
	{
	  if (k&1)
	  {
	    // we will receive an face from this neighbour
	    num_boundary_faces_recv++;
	    block->receiving++;
	    num_recv_from_proc[neighbor->proc_id]++;
	    // what is the maximal amount of data for this face
	    size = 2*(neighbor->size[((k/2)+1)%3]+2+2)*(neighbor->size[((k/2)+2)%3]+2+2);
	    if (size > max_data_from_proc[neighbor->proc_id])
	    {
	      max_data_from_proc[neighbor->proc_id] = size;
	    }
	  }
	  else
	  {
	    // we will send an face to this neighbour
	    num_boundary_faces_send++;
	    block->sending++;
	  }
	}
      }
      if (block->receiving == 0)
      {
	// add processable blocks to appropriate candidate queue
	if (block->sending == 3)
	{
	  block->queued = true;
	  candidate_queue[0]->push_back(block);
	}
	else if (block->sending == 2)
	{
	  block->queued = true;
	  candidate_queue[1]->push_back(block);
	}
	else if (block->sending == 1)
	{
	  block->queued = true;
	  candidate_queue[2]->push_back(block);
	}
	else 
	{
	  candidate_queue[4]->push_back(block);
	}
      }
      else
      {
	// add waiting blocks to recv hash
	recv_hash->insert(my_hash::value_type(block->id, block));
      }

      // this is the special dependency we need in the parallel case. there are three
      // diagonal edge adjacencies and one diagonal vertex adjacency that we need to
      // handle.

      // first diagonal edge adjacency

      if (block->neighbors[1] == 0)
      {
	if (block->neighbors[3] && block->neighbors[3]->proc_id == rank)
	{
	  if (block->neighbors[3]->neighbors[1] && block->neighbors[3]->neighbors[1]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[3]->neighbors[1]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) a\n", rank, block->id, block->neighbors[3]->neighbors[1]->id, block->neighbors[3]->neighbors[1]->proc_id);fflush(0);}
	  }
	}
      }
      else if (block->neighbors[3] == 0 || block->neighbors[3]->proc_id == rank)
      {
	if (block->neighbors[1] && block->neighbors[1]->proc_id == rank)
	{
	  if (block->neighbors[1]->neighbors[3] && block->neighbors[1]->neighbors[3]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[1]->neighbors[3]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) b\n", rank, block->id, block->neighbors[1]->neighbors[3]->id, block->neighbors[1]->neighbors[3]->proc_id);fflush(0);}
	  }
	}
      }

      // second diagonal edge adjacency

      if (block->neighbors[1] == 0)
      {
	if (block->neighbors[5] && block->neighbors[5]->proc_id == rank)
	{
	  if (block->neighbors[5]->neighbors[1] && block->neighbors[5]->neighbors[1]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[5]->neighbors[1]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) c\n", rank, block->id, block->neighbors[5]->neighbors[1]->id, block->neighbors[5]->neighbors[1]->proc_id);fflush(0);}
	  }
	}
      }
      else if (block->neighbors[5] == 0 || block->neighbors[5]->proc_id == rank)
      {
	if (block->neighbors[1] && block->neighbors[1]->proc_id == rank)
	{
	  if (block->neighbors[1]->neighbors[5] && block->neighbors[1]->neighbors[5]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[1]->neighbors[5]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) d\n", rank, block->id, block->neighbors[1]->neighbors[5]->id, block->neighbors[1]->neighbors[5]->proc_id);fflush(0);}
	  }
	}
      }

      // third diagonal edge adjacency

      if (block->neighbors[3] == 0)
      {
	if (block->neighbors[5] && block->neighbors[5]->proc_id == rank)
	{
	  if (block->neighbors[5]->neighbors[3] && block->neighbors[5]->neighbors[3]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[5]->neighbors[3]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) e\n", rank, block->id, block->neighbors[5]->neighbors[3]->id, block->neighbors[5]->neighbors[3]->proc_id);fflush(0);}
	  }
	}
      }
      else if (block->neighbors[5] == 0 || block->neighbors[5]->proc_id == rank)
      {
	if (block->neighbors[3] && block->neighbors[3]->proc_id == rank)
	{
	  if (block->neighbors[3]->neighbors[5] && block->neighbors[3]->neighbors[5]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[3]->neighbors[5]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) f\n", rank, block->id, block->neighbors[3]->neighbors[5]->id, block->neighbors[3]->neighbors[5]->proc_id);fflush(0);}
	  }
	}
      }

      // only diagonal vertex edjacency

      if ((block->neighbors[1] == 0 || block->neighbors[1]->proc_id == rank) &&
	  (block->neighbors[3] == 0 || block->neighbors[3]->proc_id == rank) &&
	  (block->neighbors[5] == 0 || block->neighbors[5]->proc_id == rank))
      {
	if ((block->neighbors[1] && (block->neighbors[1]->neighbors[3] == 0 || block->neighbors[1]->neighbors[3]->proc_id == rank)) &&
	    (block->neighbors[1] && (block->neighbors[1]->neighbors[5] == 0 || block->neighbors[1]->neighbors[5]->proc_id == rank)) &&
	    (block->neighbors[3] && (block->neighbors[3]->neighbors[1] == 0 || block->neighbors[3]->neighbors[1]->proc_id == rank)) &&
	    (block->neighbors[3] && (block->neighbors[3]->neighbors[5] == 0 || block->neighbors[3]->neighbors[5]->proc_id == rank)) &&
	    (block->neighbors[5] && (block->neighbors[5]->neighbors[1] == 0 || block->neighbors[5]->neighbors[1]->proc_id == rank)) &&
	    (block->neighbors[5] && (block->neighbors[5]->neighbors[3] == 0 || block->neighbors[5]->neighbors[3]->proc_id == rank)))
	{
	  if (block->neighbors[1] && block->neighbors[1]->neighbors[3] && block->neighbors[1]->neighbors[3]->neighbors[5] && block->neighbors[1]->neighbors[3]->neighbors[5]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[1]->neighbors[3]->neighbors[5]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) A\n", rank, block->id, block->neighbors[1]->neighbors[3]->neighbors[5]->id, block->neighbors[1]->neighbors[3]->neighbors[5]->proc_id);fflush(0);}
	  }
	  else if (block->neighbors[1] && block->neighbors[1]->neighbors[5] && block->neighbors[1]->neighbors[5]->neighbors[3] && block->neighbors[1]->neighbors[5]->neighbors[3]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[1]->neighbors[5]->neighbors[3]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) B\n", rank, block->id, block->neighbors[1]->neighbors[5]->neighbors[3]->id, block->neighbors[1]->neighbors[5]->neighbors[3]->proc_id);fflush(0);}
	  }
	  else if (block->neighbors[3] && block->neighbors[3]->neighbors[1] && block->neighbors[3]->neighbors[1]->neighbors[5] && block->neighbors[3]->neighbors[1]->neighbors[5]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[3]->neighbors[1]->neighbors[5]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) C\n", rank, block->id, block->neighbors[3]->neighbors[1]->neighbors[5]->id, block->neighbors[3]->neighbors[1]->neighbors[5]->proc_id);fflush(0);}
	  }
	  else if (block->neighbors[3] && block->neighbors[3]->neighbors[5] && block->neighbors[3]->neighbors[5]->neighbors[1] && block->neighbors[3]->neighbors[5]->neighbors[1]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[3]->neighbors[5]->neighbors[1]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) D\n", rank, block->id, block->neighbors[3]->neighbors[5]->neighbors[1]->id, block->neighbors[3]->neighbors[5]->neighbors[1]->proc_id);fflush(0);}
	  }
	  else if (block->neighbors[5] && block->neighbors[5]->neighbors[1] && block->neighbors[5]->neighbors[1]->neighbors[3] && block->neighbors[5]->neighbors[1]->neighbors[3]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[5]->neighbors[1]->neighbors[3]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) E\n", rank, block->id, block->neighbors[5]->neighbors[1]->neighbors[3]->id, block->neighbors[5]->neighbors[1]->neighbors[3]->proc_id);fflush(0);}
	  }
	  else if (block->neighbors[5] && block->neighbors[5]->neighbors[3] && block->neighbors[5]->neighbors[3]->neighbors[1] && block->neighbors[5]->neighbors[3]->neighbors[1]->proc_id != rank)
	  {
	    add_dependency(block, block->neighbors[5]->neighbors[3]->neighbors[1]);
	    if (LOG_FILE_OUTPUT) {printf( "[%d] add dependency from %d to %d (on %d) F\n", rank, block->id, block->neighbors[5]->neighbors[3]->neighbors[1]->id, block->neighbors[5]->neighbors[3]->neighbors[1]->proc_id);fflush(0);}
	  }
	}
      }
      if (LOG_FILE_OUTPUT) {printf( "[%d] has block %d (recv %d / send %d)\n", rank, block->id, block->receiving, block->sending); fflush(0);} 
    }
  }

  /* make a shorter list for the processors we receive from */

  from_proc_num = 0;
  for (i = 0; i < nprocs; i++)
  {
    if (num_recv_from_proc[i]) from_proc_num++;
  }
  if (from_proc_num)
  {
    from_proc_num_recv = new int[from_proc_num];
    from_proc_max_data = new int[from_proc_num];
    from_proc_proc_id = new int[from_proc_num];
    // space for the receive requests per processor
    from_proc_requests = new MPI_Request[REQUESTS_PER_PROC*from_proc_num];
    from_proc_buffers = new unsigned char*[REQUESTS_PER_PROC*from_proc_num];
    for (i = 0,j = 0; i < nprocs; i++)
    {
      if (num_recv_from_proc[i])
      {
	from_proc_num_recv[j] = num_recv_from_proc[i];
	from_proc_max_data[j] = max_data_from_proc[i];
	from_proc_proc_id[j] = i;
	j++;
      }
    }
  }
  else
  {
    from_proc_num_recv = 0;
    from_proc_max_data = 0;
    from_proc_proc_id = 0;
    from_proc_requests = 0;
    from_proc_buffers = 0;
  }

  delete [] num_recv_from_proc; num_recv_from_proc = 0;
  delete [] max_data_from_proc; max_data_from_proc = 0;

  /* put out REQUESTS_PER_PROC receive request for each processor that has something for us */

  for (j = 0; j < from_proc_num; j++)
  {
    for (i = 0; i < REQUESTS_PER_PROC; i++)
    {
      if (from_proc_num_recv[j])
      {
	// create the receive buffer
	size = dsize*from_proc_max_data[j]+sizeof(Ghost3Dheader);
	from_proc_buffers[j+i*from_proc_num] = new unsigned char[size];
	// post a receive request
	MPI_Irecv(from_proc_buffers[j+i*from_proc_num],
		  size,
		  MPI_BYTE,
		  from_proc_proc_id[j],
		  FACE_TAG,
		  MPI_COMM_WORLD,
		  &(from_proc_requests[j+i*from_proc_num]));
      	if (DEBUG_OUTPUT) { printf("[%d] Scheduling receipt for %d elements from proc %d\n",rank,from_proc_max_data[j],from_proc_proc_id[j]); fflush(NULL); }
	from_proc_num_recv[j]--;
      }
      else
      {
	from_proc_buffers[j+i*from_proc_num] = 0;
	from_proc_requests[j+i*from_proc_num] = MPI_REQUEST_NULL;
      }
    }
  }

  // create space for send requests

  send_request_num = 0;
  send_request_num_max = 0;
  if (num_boundary_faces_send)
  {
    send_request_alloc = 10;
    send_requests = new MPI_Request[send_request_alloc];
    send_faces = new Ghost3Dface*[send_request_alloc];
  }
  else
  {
    send_request_alloc = 0;
    send_requests = 0;
    send_faces = 0;
  }
  return true;
}
#else // ENABLE_PARALLEL_MODE
bool Ghost3Dmodule::init(Ghost3Dblock* blocks, int nblocks, int dsize)
{
  int i;
  Ghost3Dblock* block;

  // for statistics
  faces_size = 0;
  faces_maxsize = 0;

  // the size of each field entry
  this->dsize = dsize;

  // count number of blocks we own and faces we send/recv
  num_blocks = nblocks;
  num_processed_blocks = 0;
  selected_block = 0;

  // blocks are maintained in five queues of decreasing priority
  candidate_queue[0] = new my_queue();
  candidate_queue[1] = new my_queue();
  candidate_queue[2] = new my_queue();
  candidate_queue[3] = new my_queue();
  candidate_queue[4] = new my_queue();

  // get all starts
  for (i = 0; i < nblocks; i++)
  {
    block = &(blocks[i]);
    if (block->neighbors[0] == 0 && block->neighbors[2] == 0 && block->neighbors[4] == 0)
    {
      candidate_queue[3]->push_back(block); // a start
    }
    else
    {
      candidate_queue[4]->push_back(block);  // all others
    }
    block->loaded = false;
  }
  return true;
}
#endif // ENABLE_PARALLEL_MODE

#ifdef ENABLE_PARALLEL_MODE
int Ghost3Dmodule::selectBlock()
{
  // return -1 if we already processed all the blocks
  if (num_processed_blocks == num_blocks)
  {
    return -1;
  }

  // return the id of a block that was selected previously
  if (selected_block != 0)
  {
    printf( "WARNING: block already selected. need to call processBlock() first.\n");
    return selected_block->id;
  }

  // first we check if we have sent something
  while (send_request_num)
  {
    int k;
    int flag;
    MPI_Status status;
    MPI_Testany( send_request_num, send_requests, &k, &flag, &status );
    if (!flag) break;
    delete send_faces[k];
    send_request_num--;
    send_faces[k] = send_faces[send_request_num];
    send_requests[k] = send_requests[send_request_num];
  }

  // then we check if we have received something
  while (num_boundary_faces_recv)
  {
    if (!receive_face(false)) break;
    num_boundary_faces_recv--;
  }

  // loop over the queues and first select blocks that result in sending data
  int i,j;
  Ghost3Dblock* block;
  while (true)
  {
    for (i = 0; i < 5; i++)
    {
      block = 0;
      j = candidate_queue[i]->size();
      if (DEBUG_OUTPUT) {printf("[%d] looking at queue %d with %d after processing %d of %d blocks\n",rank,i,j,num_processed_blocks,num_blocks); fflush(0);}
      while (j--)
      {
	// get the front element
	block = candidate_queue[i]->front();
	candidate_queue[i]->pop_front();
	if (DEBUG_OUTPUT) {printf("[%d] looking at queue %d block %d (%d)\n",rank,i,block->id,block->loaded); fflush(0);}
	// has this block already been processed
	if (block->loaded)
	{
	  // remove it and keep looking
	  block = 0;
	  continue;
	}
	// does this block have an allowed adjacency to previously processed blocks
	if (!allowed_for_processing(block))
	{
	  if (DEBUG_OUTPUT) {printf("[%d] looking at queue %d block %d not allowed\n",rank,i,block->id); fflush(0);}
	  // put it at the back of the queue and keep looking
	  candidate_queue[i]->push_back(block);
	  block = 0;
	  continue;
	}
	break;
      }
      
      if (block)
      {
	if (DEBUG_OUTPUT) printf("[%d] inside of queue %d we found block %d\n",rank,i,block->id);
	// we found a block
	block->queued = true;
	// we add all unqueued face neighbours from this processor that are not receiving to queue 3
	for (i = 0; i < 6; i++)
	{
	  if (block->neighbors[i] && block->neighbors[i]->queued == false && block->neighbors[i]->proc_id == rank && block->neighbors[i]->receiving == 0)
	  {
	    block->neighbors[i]->queued = true;
	    candidate_queue[3]->push_back(block->neighbors[i]);
	  }
	}
	// we put it into the field where processBlock() expects it
	selected_block = block;
	return block->id;
      }   
    }
    
    if (num_boundary_faces_recv)
    {
      receive_face(true);
      num_boundary_faces_recv--;
    }
    else
    {
      printf( "[%d] FATAL ERROR: no block to select and nothing to receive\n", rank); fflush(0);
      exit(1);
    }
  }
}
#else // ENABLE_PARALLEL_MODE
int Ghost3Dmodule::selectBlock()
{
  // return -1 if we already processed all the blocks
  if (num_processed_blocks == num_blocks)
  {
    return -1;
  }

  // return the id of a block that was selected previously
  if (selected_block != 0)
  {
    printf( "WARNING: block already selected. need to call processBlock() first.\n");
    return selected_block->id;
  }

  // loop over the queues
  int i,j;
  Ghost3Dblock* block;
  for (i = 0; i < 5; i++)
  {
    block = 0;
    j = candidate_queue[i]->size();
    while (j--)
    {
      // get the front element
      block = candidate_queue[i]->front();
      candidate_queue[i]->pop_front();
      // has this block already been processed
      if (block->loaded)
      {
	// remove it and keep looking
	block = 0;
	continue;
      }
      // does this block have an allowed adjacency to previously processed blocks
      if (!allowed_for_processing(block))
      {
	// put it at the back of the queue and keep looking
	candidate_queue[i]->push_back(block);
	block = 0;
	continue;
      }
      // found a block ... queue face neighbours
      if (block->neighbors[1]) candidate_queue[0]->push_back(block->neighbors[1]); // in i dir
      if (block->neighbors[3]) candidate_queue[1]->push_back(block->neighbors[3]); // in j dir
      if (block->neighbors[5]) candidate_queue[2]->push_back(block->neighbors[5]); // in k dir
      // put it into the field where processBlock() expects it
      selected_block = block;
      return block->id;
    }
  }
  printf( "WARNING: we should never get here.\n");
  return -1;
}
#endif // ENABLE_PARALLEL_MODE

#ifdef ENABLE_PARALLEL_MODE
unsigned char* Ghost3Dmodule::processBlock(unsigned char* data_in, int* origin_out, int* size_out)
{
  int i,size;
  Ghost3Dface* faces[6];

  if (selected_block == 0)
  {
    printf( "ERROR: no block selected. need to call selectBlock() first.\n");
    return 0;
  }

  Ghost3Dblock* block = selected_block;

  selected_block = 0;

  if (block->receiving)
  {
    printf( "WARNING: processing block whose receiving state is %d\n", block->receiving);
  }

  if (block->loaded)
  {
    printf( "WARNING: processing block that was already processed\n");
  }

  if (LOG_FILE_OUTPUT) {printf("[%d] processes block %d (%d/%d) now\n",rank, block->id, block->receiving, block->sending); fflush(NULL);}

  // in how many directions can we grow and what do we output
  int block_size[3];
  int block_origin[3];
  for (i = 0; i < 3; i++)
  {
    block_size[i] = block->size[i];
    block_origin[i] = block->origin[i];
  }
  for (i = 0; i < 6; i++)
  {
    if (block->neighbors[i] && block->neighbors[i]->faces[i^1])
    {
      faces[i] = block->neighbors[i]->faces[i^1];
      block->neighbors[i]->faces[i^1] = 0;
      block_size[i/2] += 2;
      if (i%2 == 0) block_origin[i/2] -= 2;
    }
    else
    {
      faces[i] = 0;
    }
  }

  // create the output block
  unsigned char* block_data = new unsigned char[dsize*block_size[0]*block_size[1]*block_size[2]];

  // fill the output block
  copy_block(data_in, block->origin, block->size, block_data, block_origin, block_size);
  for (i = 0; i < 6; i++)
  {
    if (faces[i])
    {
      copy_block(faces[i]->data, faces[i]->header->origin, faces[i]->header->size, block_data, block_origin, block_size);
      delete faces[i];
      faces_size--;
    }
  }

  // what do we keep
  for (i = 0; i < 6; i++)
  {
    // keep the face if the neighbour exists and has not been loaded and (is on this processor or needs to be sent)
    if (block->neighbors[i] && ((block->neighbors[i]->loaded == false && block->neighbors[i]->proc_id == rank) || (block->neighbors[i]->proc_id != rank && (i&1) == 0)))
    {
      block->faces[i] = new Ghost3Dface();
      faces_size++;
      if (faces_size > faces_maxsize) faces_maxsize = faces_size;
      if (i < 2)
      {
	block->faces[i]->alloc_and_init(dsize, 2, block_size[1], block_size[2], block_origin[0] + (i==0?0:block_size[0]-2), block_origin[1], block_origin[2]);
      }
      else if (i < 4)
      {
	block->faces[i]->alloc_and_init(dsize, block_size[0], 2, block_size[2], block_origin[0], block_origin[1] + (i==2?0:block_size[1]-2), block_origin[2]);
      }
      else
      {
	block->faces[i]->alloc_and_init(dsize, block_size[0], block_size[1], 2, block_origin[0], block_origin[1], block_origin[2] + (i==4?0:block_size[2]-2));
      }
      copy_face(block_data, block_origin, block_size, block->faces[i]->data, block->faces[i]->header->origin, block->faces[i]->header->size);
    }
    else
    {
      block->faces[i] = 0;
    }
  }
  block->loaded = true;

  if (DEBUG_OUTPUT) { printf("[%d] processed block %d kept these faces %d %d %d %d %d %d\n",rank, block->id, block->faces[0]!=0, block->faces[1]!=0, block->faces[2]!=0, block->faces[3]!=0, block->faces[4]!=0, block->faces[5]!=0); fflush(NULL); }
	
  // we may need to send something 
  i = 0;
  while (block->sending)
  {
    if (block->neighbors[i] && block->neighbors[i]->proc_id != rank)
    {
      size = dsize*block->faces[i]->header->size[0]*block->faces[i]->header->size[1]*block->faces[i]->header->size[2] + sizeof(Ghost3Dheader);
      block->faces[i]->header->block_id = block->neighbors[i]->id;
      block->faces[i]->header->face_id = i+1;
      if (DEBUG_OUTPUT) { printf("[%d] sending face %d of block %d to processor %d\n",rank,i, block->id, block->neighbors[i]->proc_id); fflush(NULL); }
      // make sure we have enough space for this non-blocking send
      if (send_request_num == send_request_alloc)
      {
	// resize array
	int j;
	MPI_Request* temp_send_requests = new MPI_Request[send_request_alloc*2];
	Ghost3Dface** temp_send_faces = new Ghost3Dface*[send_request_alloc*2];
	for (j = 0; j < send_request_alloc; j++)
	{
	  temp_send_requests[j] = send_requests[j];
	  temp_send_faces[j] = send_faces[j];
	}
	delete [] send_requests;
	delete [] send_faces;
	send_request_alloc = send_request_alloc*2;
	send_requests = temp_send_requests;
	send_faces = temp_send_faces;
      }
      MPI_Isend(block->faces[i]->header,
		size,
		MPI_BYTE,
		block->neighbors[i]->proc_id,
		FACE_TAG,
		MPI_COMM_WORLD,
		&(send_requests[send_request_num]));
      if (DEBUG_OUTPUT) { printf("[%d] sent face %d of block %d to processor %d\n",rank,i, block->id, block->neighbors[i]->proc_id); fflush(NULL); }
      send_faces[send_request_num] = block->faces[i];
      send_request_num++;
      if (send_request_num_max < send_request_num) send_request_num_max = send_request_num;
      num_boundary_faces_send--;
      block->faces[i] = 0;
      block->sending--;
      faces_size--; 
    }
    i+=2;
  }
  num_processed_blocks++;

  // copy result to output
  for (i = 0; i < 3; i++)
  {
    origin_out[i] = block_origin[i];
    size_out[i] = block_size[i];
  }

  return block_data;
}

#else // ENABLE_PARALLEL_MODE

unsigned char* Ghost3Dmodule::processBlock(unsigned char* data_in, int* origin_out, int* size_out)
{
  int i;
  Ghost3Dface* faces[6];

  if (selected_block == 0)
  {
    printf( "ERROR: no block selected. need to call selectBlock() first.\n");
    return 0;
  }

  Ghost3Dblock* block = selected_block;

  selected_block = 0;

  if (block->loaded)
  {
    printf( "WARNING: processing block that was already processed\n");
  }

  if (LOG_FILE_OUTPUT) {printf("[%d] processes block %d now\n",0, block->id); fflush(NULL);}

  // in how many directions can we grow and what do we output
  int block_size[3];
  int block_origin[3];
  for (i = 0; i < 3; i++)
  {
    block_size[i] = block->size[i];
    block_origin[i] = block->origin[i];
  }
  for (i = 0; i < 6; i++)
  {
    if (block->neighbors[i] && block->neighbors[i]->faces[i^1])
    {
      faces[i] = block->neighbors[i]->faces[i^1];
      block->neighbors[i]->faces[i^1] = 0;
      block_size[i/2] += 2;
      if (i%2 == 0) block_origin[i/2] -= 2;
    }
    else
    {
      faces[i] = 0;
    }
  }

  // create the output block
  unsigned char* block_data = new unsigned char[dsize*block_size[0]*block_size[1]*block_size[2]];

  // fill the output block
  copy_block(data_in, block->origin, block->size, block_data, block_origin, block_size);
  for (i = 0; i < 6; i++)
  {
    if (faces[i])
    {
      copy_block(faces[i]->data, faces[i]->header->origin, faces[i]->header->size, block_data, block_origin, block_size);
      delete faces[i];
      faces_size--;
    }
  }

  // what do we keep
  for (i = 0; i < 6; i++)
  {
    // keep the face if the neighbour exists and has not been loaded and (is on this processor or needs to be sent)
    if (block->neighbors[i] && block->neighbors[i]->loaded == false)
    {
      block->faces[i] = new Ghost3Dface();
      faces_size++;
      if (faces_size > faces_maxsize) faces_maxsize = faces_size;
      if (i < 2)
      {
	block->faces[i]->alloc_and_init(dsize, 2, block_size[1], block_size[2], block_origin[0] + (i==0?0:block_size[0]-2), block_origin[1], block_origin[2]);
      }
      else if (i < 4)
      {
	block->faces[i]->alloc_and_init(dsize, block_size[0], 2, block_size[2], block_origin[0], block_origin[1] + (i==2?0:block_size[1]-2), block_origin[2]);
      }
      else
      {
	block->faces[i]->alloc_and_init(dsize, block_size[0], block_size[1], 2, block_origin[0], block_origin[1], block_origin[2] + (i==4?0:block_size[2]-2));
      }
      copy_face(block_data, block_origin, block_size, block->faces[i]->data, block->faces[i]->header->origin, block->faces[i]->header->size);
    }
    else
    {
      block->faces[i] = 0;
    }
  }
  block->loaded = true;
  num_processed_blocks++;

  // copy result to output
  for (i = 0; i < 3; i++)
  {
    origin_out[i] = block_origin[i];
    size_out[i] = block_size[i];
  }

  return block_data;
}
#endif // ENABLE_PARALLEL_MODE

#ifdef ENABLE_PARALLEL_MODE
bool Ghost3Dmodule::close()
{
  // return false if we have not processed all the blocks
  if (num_processed_blocks < num_blocks)
  {
    printf( "ERROR: not all blocks processed yet.\n");
    return false;
  }

  if (DEBUG_OUTPUT) { printf( "[%d] done but waiting to send %d faces (max was %d)\n", rank, send_request_num, send_request_num_max); fflush(0); }

  // we check if we have sent something
  while (send_request_num)
  {
    int k;
    MPI_Status status;
    MPI_Waitany( send_request_num, send_requests, &k, &status );
    delete send_faces[k];
    send_request_num--;
    send_faces[k] = send_faces[send_request_num];
    send_requests[k] = send_requests[send_request_num];
  }

  if (DEBUG_OUTPUT) { printf( "[%d] faces %d/%d\n", rank, faces_size, faces_maxsize); fflush(0); }

  delete recv_hash;
  delete candidate_queue[0];
  delete candidate_queue[1];
  delete candidate_queue[2];
  delete candidate_queue[3];
  delete candidate_queue[4];

  if (send_requests) delete [] send_requests;
  if (send_faces) delete [] send_faces;

  if (from_proc_num_recv) delete [] from_proc_num_recv;
  if (from_proc_max_data) delete [] from_proc_max_data;
  if (from_proc_proc_id) delete [] from_proc_proc_id;
  if (from_proc_requests) delete [] from_proc_requests;
  if (from_proc_buffers) delete [] from_proc_buffers;

  return true;
}
#else // ENABLE_PARALLEL_MODE
bool Ghost3Dmodule::close()
{
  // return false if we have not processed all the blocks
  if (num_processed_blocks < num_blocks)
  {
    printf( "ERROR: not all blocks processed yet.\n");
    return false;
  }

  if (DEBUG_OUTPUT) { printf( "[%d] faces %d/%d\n", 0, faces_size, faces_maxsize); fflush(0); }

  delete candidate_queue[0];
  delete candidate_queue[1];
  delete candidate_queue[2];
  delete candidate_queue[3];
  delete candidate_queue[4];

  return true;
}
#endif // ENABLE_PARALLEL_MODE

int Ghost3Dmodule::copy_block(unsigned char* data, int* origin, int* size, unsigned char* data_out, int* origin_out, int* size_out)
{
  int j,k;
  int pos;
  int pos_out;

  int copied = 0;
  int diff_i = origin[0] - origin_out[0];
  int diff_j = origin[1] - origin_out[1];
  int diff_k = origin[2] - origin_out[2];
  int start_i = (origin[0] >= origin_out[0]) ? 0 : origin_out[0] - origin[0];
  int end_i = (origin[0] + size[0] <= origin_out[0] + size_out[0]) ? size[0] : origin[0] - origin_out[0] + size_out[0] - diff_i - diff_i; 
  int start_j = (origin[1] >= origin_out[1]) ? 0 : origin_out[1] - origin[1];
  int end_j = (origin[1] + size[1] <= origin_out[1] + size_out[1]) ? size[1] : origin[1] - origin_out[1] + size_out[1] - diff_j - diff_j; 
  int start_k = (origin[2] >= origin_out[2]) ? 0 : origin_out[2] - origin[2];
  int end_k = (origin[2] + size[2] <= origin_out[2] + size_out[2]) ? size[2] : origin[2] - origin_out[2] + size_out[2] - diff_k - diff_k; 

  for (k = start_k; k < end_k; k++)
  {
    // just a check that this layer is really inside the outblock
    if ( ((origin[2] + k) < origin_out[2]) || ((origin[2] + k) >= (origin_out[2]+size_out[2])) )
    {
      printf( "WARNING: outside layer %d\n", k);
    }
    pos = (k * size[1] * size[0]) + (start_j * size[0]) + start_i;
    pos_out = ((diff_k + k) * size_out[1] * size_out[0]) + ((diff_j + start_j) * size_out[0]) + (diff_i + start_i);
    for (j = start_j; j < end_j; j++)
    {
      // just a check that this row is really inside the outblock
      if ( ((origin[1] + j) < origin_out[1]) || ((origin[1] + j) >= (origin_out[1]+size_out[1])) )
      {
        printf( "WARNING: outside row %d/%d\n", k, j);
      }
      memcpy(&(data_out[dsize*pos_out]), &(data[dsize*pos]), dsize*(end_i-start_i));
      copied += (end_i-start_i);
      pos += size[0];
      pos_out += size_out[0];
    }
  }
  return copied;
}

int Ghost3Dmodule::copy_face(unsigned char* data, int* origin, int* size, unsigned char* data_out, int* origin_out, int* size_out)
{
  int j,k;
  int pos;
  int pos_out;
  int copied = 0;
  int diff_i = origin_out[0] - origin[0];
  int diff_j = origin_out[1] - origin[1];
  int diff_k = origin_out[2] - origin[2];
  for (k = 0; k < size_out[2]; k++)
  {
    pos = ((diff_k + k) * size[1] * size[0]) + (diff_j * size[0]) + diff_i;
    pos_out = k * size_out[1] * size_out[0];
    for (j = 0; j < size_out[1]; j++)
    {
      memcpy(&(data_out[dsize*pos_out]), &(data[dsize*pos]), dsize*size_out[0]);
      pos += size[0];
      pos_out += size_out[0];
      copied += size_out[0];
    }
  }
  return copied;
}

/* when non-zero the case is invalid
   note: this is only validity check from the point of
   adding the vertex number zero. if the tested scenario
   is invalid without vertex zero (which can happen in a
   parallel scenario) but making it valid will always
   result in a valid scenario (with or without zero) then
   we also consider it okay. (e.g. 17, 44, 49, 65, 81,
   97, 113) */

static const int invalid_cases_alt[128] = {
  0, 1, 2, 3, 4, 5, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  24, 25, 26, 27, 28, 29, 30, 31, 
  0, 0, 0, 0, 36, 37, 38, 39, 
  0, 0, 0, 0, 44, 45, 46, 47, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 66, 67, 0, 0, 70, 71, 
  0, 0, 74, 75, 0, 0, 78, 79, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0
};

bool Ghost3Dmodule::allowed_for_processing(Ghost3Dblock* block)
{
  int i,j,k;
  int c;

#ifdef ENABLE_PARALLEL_MODE
  if (block->dependencies)
  {
    for (i = 0; i < 3; i++)
    {
      if (block->dependencies[i] == 0) break;
      if (block->dependencies[i]->loaded == false) return false;
    } 
  }
#endif

  for (k = 4; k < 6; k++)
  {
    for (j = 2; j < 4; j++)
    {
      for (i = 0; i < 2; i++)
      {
	c = 0;
	if (block->neighbors[i])
	{
	  if (block->neighbors[i]->loaded) c |= 64;
	  if (block->neighbors[i]->neighbors[j])
	  {
	    if (block->neighbors[i]->neighbors[j]->loaded) c |= 8;
	    if (block->neighbors[i]->neighbors[j]->neighbors[k])
	    {
	      if (block->neighbors[i]->neighbors[j]->neighbors[k]->loaded) c |= 1;	      
	    }
	  }
	  if (block->neighbors[i]->neighbors[k])
	  {
	    if (block->neighbors[i]->neighbors[k]->loaded) c |= 4;
	    if (block->neighbors[i]->neighbors[k]->neighbors[j])
	    {
	      if (block->neighbors[i]->neighbors[k]->neighbors[j]->loaded) c |= 1;	      
	    }
	  }
	}
	if (block->neighbors[j])
	{
	  if (block->neighbors[j]->loaded) c |= 32;
	  if (block->neighbors[j]->neighbors[i])
	  {
	    if (block->neighbors[j]->neighbors[i]->loaded) c |= 8;
	    if (block->neighbors[j]->neighbors[i]->neighbors[k])
	    {
	      if (block->neighbors[j]->neighbors[i]->neighbors[k]->loaded) c |= 1;	      
	    }
	  }
	  if (block->neighbors[j]->neighbors[k])
	  {
	    if (block->neighbors[j]->neighbors[k]->loaded) c |= 2;
	    if (block->neighbors[j]->neighbors[k]->neighbors[i])
	    {
	      if (block->neighbors[j]->neighbors[k]->neighbors[i]->loaded) c |= 1;	      
	    }
	  }	  
	}
	if (block->neighbors[k])
	{
	  if (block->neighbors[k]->loaded) c |= 16;
	  if (block->neighbors[k]->neighbors[i])
	  {
	    if (block->neighbors[k]->neighbors[i]->loaded) c |= 4;
	    if (block->neighbors[k]->neighbors[i]->neighbors[j])
	    {
	      if (block->neighbors[k]->neighbors[i]->neighbors[j]->loaded) c |= 1;	      
	    }
	  }
	  if (block->neighbors[k]->neighbors[j])
	  {
	    if (block->neighbors[k]->neighbors[j]->loaded) c |= 2;
	    if (block->neighbors[k]->neighbors[j]->neighbors[i])
	    {
	      if (block->neighbors[k]->neighbors[j]->neighbors[i]->loaded) c |= 1;	      
	    }
	  }	  
	}
	// check case number
	if (invalid_cases_alt[c]) return false;
      }
    }
  }
  return true;
}

#ifdef ENABLE_PARALLEL_MODE
void Ghost3Dmodule::partition_blocks_simple(Ghost3Dblock* blocks, int nblocks, int nparts)
{
  int i;
  for (i = 0; i < nblocks; i++)
  {
    blocks[i].proc_id = i*nparts/nblocks;
  }
}

void Ghost3Dmodule::partition_blocks_random(Ghost3Dblock* blocks, int nblocks, int nparts)
{
  int i;
  for (i = 0; i < nblocks; i++)
  {
    blocks[i].proc_id = rand()%nparts;
  }
}

void Ghost3Dmodule::partition_blocks(Ghost3Dblock* blocks, int nblocks, int nparts, bool use_weights)
{
#ifdef HAVE_NO_METIS
  partition_blocks_simple(blocks, nblocks, nparts);
#else
  // create an adjacency graph for metis
  int i,j;
  int* xadj = new int[nblocks+1];
  int* adjcny = new int[nblocks*6];
  int* vweights = 0;
  int l = 0;
  xadj[0] = 0;

  for (i = 0; i < nblocks; i++)
  {
    for (j = 0; j < 6; j++)
    {
      if (blocks[i].neighbors[j])
      {
        adjcny[l] = blocks[i].neighbors[j]->id;
        l++;
      }
    }
    xadj[i+1] = l;
  }

  if (use_weights)
  {
    int max_weight = 0;
    vweights = new int[nblocks];
    for (i = 0; i < nblocks; i++)
    {
      vweights[i] = blocks[i].size[0]*blocks[i].size[1]*blocks[i].size[2];
      if (vweights[i] > max_weight) max_weight = vweights[i];
      if (vweights[i] == 0) printf("WARNING weight %d is zero\n",i);
    }
    // bring weights into range from 1 to 10
    float scale = 10.0f/max_weight;
    for (i = 0; i < nblocks; i++)
    {
      vweights[i] = (int)(scale*vweights[i]+0.5f);
      if (vweights[i] == 0) vweights[i] = 1;
    }
  }

  // partition with metis

  static int options[5] = { 0, 3, 1, 1, 0 };
  int edgeCut = 0;
  int *metisBoxes = new int[nblocks];
  int wgtflag = use_weights ? 2 : 0;
  int numflag = 0;

  if (nparts > 8)
  {
    METIS_PartGraphKway(&nblocks, xadj, adjcny, vweights, 0, &wgtflag, &numflag, &nparts, options, &edgeCut, metisBoxes);
  }
  else
  {
    METIS_PartGraphRecursive(&nblocks, xadj, adjcny, vweights, 0, &wgtflag, &numflag, &nparts, options, &edgeCut, metisBoxes);
  }

  delete [] xadj;
  delete [] adjcny;
  if (vweights) delete [] vweights;

  // assign processor numbers
  for (i = 0; i < nblocks; i++)
  {
    blocks[i].proc_id = metisBoxes[i];
  }

  delete [] metisBoxes;
#endif
}

void Ghost3Dmodule::add_dependency(Ghost3Dblock* block, Ghost3Dblock* dependency)
{
  if (block->dependencies == 0)
  {
    block->dependencies = new Ghost3Dblock*[3];
    block->dependencies[0] = block->dependencies[1] = block->dependencies[2] = 0;
  }
  for (int i = 0; i < 3; i++)
  {
    if (block->dependencies[i] == dependency)
    {
      return;
    }
    else if (block->dependencies[i] == 0)
    {
      block->dependencies[i] = dependency;
      return;
    }
  }
  printf("we should never get here\n");
  exit(0);
}

bool Ghost3Dmodule::receive_face(bool blocking)
{
  int k;
  int k_mult;
  MPI_Status status;
  if (blocking)
  {
    MPI_Waitany( REQUESTS_PER_PROC*from_proc_num, from_proc_requests, &k_mult, &status );
  }
  else
  {
    int flag;
    MPI_Testany( REQUESTS_PER_PROC*from_proc_num, from_proc_requests, &k_mult, &flag, &status );
    if (!flag) return false;
  }

  k = k_mult % from_proc_num;

  // process received block
  int block_id = ((int*)(from_proc_buffers[k_mult]))[0];
  int face_id = ((int*)(from_proc_buffers[k_mult]))[1];
  
  Ghost3Dblock* recv_block;
  my_hash::iterator hash_element = recv_hash->find(block_id);
  if (hash_element == recv_hash->end())
  {
    printf("[%d] ERROR: block id %d not in hash.\n",rank,block_id);
    exit(0);
  }
  else
  {
    recv_block = (*hash_element).second;
  }
  
  if (DEBUG_OUTPUT) { printf( "[%d] received face from %d for block %d (%d/%d) for neighbor %d\n", rank, from_proc_proc_id[k], block_id, recv_block->receiving, recv_block->sending, face_id); fflush(0); }

  // make sure this waiting neighbour exists

  if (recv_block->receiving == 0)
  {
    printf("[%d] ERROR block %d for which face was received is not receiving.\n",rank,block_id);
    exit(0);
  }
  // make sure we were not queued
  if (recv_block->queued)
  {
    printf("[%d] ERROR block %d for which face was received was already queued.\n",rank,block_id);
    exit(0);
  }
  if (recv_block->neighbors[face_id] == 0)
  {
    printf("[%d] ERROR neighbour %d of block %d for which face was received does not exist.\n",rank,face_id,block_id);
    exit(0);
  }
  if (recv_block->neighbors[face_id]->proc_id != status.MPI_SOURCE)
  {
    printf("[%d] ERROR neighbour we received from lives on different processor.\n",rank);
    exit(0);
  }

  // the fact that we receive from this neighbour means he 'was loaded'
  recv_block->neighbors[face_id]->loaded = true;

  // update waiting count
  recv_block->receiving--;
  
  // possibly this block will no longer have to wait
  if (recv_block->receiving == 0)
  {
    if (recv_block->sending == 3)
    {
      recv_block->queued = true;
      candidate_queue[0]->push_back(recv_block);
    }
    else if (recv_block->sending == 2)
    {
      recv_block->queued = true;
      candidate_queue[1]->push_back(recv_block);
    }
    else if (recv_block->sending == 1)
    {
      recv_block->queued = true;
      candidate_queue[2]->push_back(recv_block);
    }
    else
    {
      candidate_queue[4]->push_back(recv_block);
    }
    recv_hash->erase(hash_element);
  }
	
  // create the face that was received
  int count;
  MPI_Get_count(&status, MPI_BYTE, &count);
  Ghost3Dface* face = new Ghost3Dface;
  face->alloc_and_copy(from_proc_buffers[k_mult], count);
  faces_size++;
  if (faces_size > faces_maxsize) faces_maxsize = faces_size;
  
  //   printf("[%d] received %d %d %d %d %d %d : ", rank, face->size[0], face->size[1], face->size[2], face->origin[0], face->origin[1], face->origin[2]);
  //   printf("%d %d ... %d %d\n", face->data[0], face->data[1], face->data[count-8], face->data[count-7]); 

  recv_block->neighbors[face_id]->faces[face_id^1] = face; 

  if (status.MPI_SOURCE != from_proc_proc_id[k])
  {
    printf("[%d] ERROR proc %d is supposed to be equal to k %d.\n",rank,status.MPI_SOURCE,from_proc_proc_id[k]);
  }

  // do we receive more from this processor
  if (from_proc_num_recv[k])
  {
    MPI_Irecv(from_proc_buffers[k_mult],
	      dsize*from_proc_max_data[k]+sizeof(Ghost3Dheader),
	      MPI_BYTE,
	      from_proc_proc_id[k],
	      FACE_TAG,
	      MPI_COMM_WORLD,
	      &(from_proc_requests[k_mult]));
    if (DEBUG_OUTPUT) { printf("[%d] New Scheduling receipt from proc %d\n",rank,from_proc_proc_id[k]); fflush(NULL); }
    from_proc_num_recv[k]--;
  }
  else
  {
    delete [] from_proc_buffers[k_mult];
    from_proc_buffers[k_mult] = 0;
    from_proc_requests[k_mult] = 0;    
  }

  return true;
}
#endif // ENABLE_PARALLEL_MODE
