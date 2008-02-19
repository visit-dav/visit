/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                             avtIsenburgSGG.h                              //
// ************************************************************************* //

#ifndef AVT_ISENBURG_SGG_H
#define AVT_ISENBURG_SGG_H

#include <database_exports.h>

#include <avtGhostData.h>
#include <avtStreamingGhostGenerator.h>

#include <vector>
using std::vector;

class vtkDataSet;
class vtkDataArray;
class avtMixedVariable;
class avtMaterial;

class Ghost3Dmodule;
class Ghost3Dblock;

// ****************************************************************************
//  Class:  avtIsenburgSGG
//
//  Purpose:
//      A concrete type of a streaming ghost generator.  This implements the
//      streaming module written by Martin Isenburg.
//
//  Programmer:  Hank Childs
//  Creation:    February 10, 2008
//
// ****************************************************************************

class DATABASE_API avtIsenburgSGG : public avtStreamingGhostGenerator
{
  public:
                          avtIsenburgSGG();
    virtual              ~avtIsenburgSGG();

    void                  SetNumberOfDomains(int);
    void                  SetInfoForDomain(int index, int *origin, int *size,
                                           int *neighbors);
    void                  FinalizeDomainInformation(void);

    virtual int           GetNextDomain(void);
    virtual vtkDataSet   *StreamDataset(vtkDataSet *);

  protected:
    int                   numDomains;
    int                   curBlock;
    Ghost3Dmodule        *ghost3dmodule;
    Ghost3Dblock         *blocks;
};

#endif


/*
===============================================================================

  FILE:  ghost3dblock.h
  
  CONTENTS:
  
    Defines the blocks that the structured grid domain is partitioned into.
    The private variables are used by the ghost3dmodule to store the current
    state and to decide which block to process next.
  
  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    copyright (C) 2007 martin isenburg@cs.unc.edu
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:

  18 December 2007 - created a day after Henna's small group christmas party

===============================================================================
*/
#ifndef GHOST_3D_BLOCK_H
#define GHOST_3D_BLOCK_H

#ifdef PARALLEL
#define ENABLE_PARALLEL_MODE
#include <mpi.h>
#endif

#define ENABLE_DEBUG_CHECK

class Ghost3Dface;

class Ghost3Dblock
{
public:
  int id;
  int proc_id;
  Ghost3Dblock* neighbors[6];
  int size[3];
  int origin[3];

#ifdef ENABLE_DEBUG_CHECK
  unsigned char* data;
  int size_out[3];
  int origin_out[3];
  unsigned char* data_out;
#endif // ENABLE_DEBUG_CHECK

  Ghost3Dblock()
  {
    // does not have an assigned id
    id = -1;
    // does not have an assigned processor
    proc_id = -1;
    // has no neighbors yet
    neighbors[0] = neighbors[1] = neighbors[2] = neighbors[3] = neighbors[4] = neighbors[5] = 0;
    // has not been loaded
    loaded = false;
#ifdef ENABLE_PARALLEL_MODE
    // has not been [0] [1] [2] [3] queued
    queued = false;
    // does not wait to receive neighbors
    receiving = 0;
    // does not send to neighbors
    sending = 0;
    // does not have dependencies
    dependencies = 0;
    // has no faces yet
    faces[0] = faces[1] = faces[2] = faces[3] = faces[4] = faces[5] = 0;
#endif // ENABLE_PARALLEL_MODE
#ifdef ENABLE_DEBUG_CHECK
    data = 0;
    data_out = 0;
#endif
  };
  ~Ghost3Dblock()
  {
#ifdef ENABLE_PARALLEL_MODE
    if (dependencies) delete [] dependencies;
#endif // ENABLE_PARALLEL_MODE
#ifdef ENABLE_DEBUG_CHECK
    if (data) delete [] data;
    if (data_out) delete [] data_out;
#endif //  ENABLE_DEBUG_CHECK
  };

private:
  friend class Ghost3Dmodule;
  // used internally
  bool loaded;
#ifdef ENABLE_PARALLEL_MODE
  bool queued;
  short receiving;
  short sending;
  Ghost3Dblock** dependencies;
#endif // ENABLE_PARALLEL_MODE
  Ghost3Dface* faces[6];
};

#endif
/*
===============================================================================

  FILE:  ghost3dface.h
  
  CONTENTS:
  
    Defines the faces that are the units in which data travels from one
    block to the next in order to append ghost data (and unsend original
    data) to other blocks. The header is used for extra information for
    all those faces that are sent to another processor.
  
  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    copyright (C) 2007 martin isenburg@cs.unc.edu
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:

  22 January 2007 - created the day that Henna gave her 30 days notice

===============================================================================
*/
#ifndef GHOST_3D_FACE_H
#define GHOST_3D_FACE_H

class Ghost3Dheader
{
public:
  int block_id;
  int face_id;
  int size[3];
  int origin[3];
};

class Ghost3Dface
{
public:
  Ghost3Dheader* header;
  unsigned char* data;
  void alloc_and_copy(unsigned char* buffer, int total_size)
  {
    // alloc
    unsigned char* d = new unsigned char[total_size];
    header = (Ghost3Dheader*)d;
    data = d + sizeof(Ghost3Dheader);
    // copy
    for (int c = 0; c < total_size; c++) d[c] = buffer[c];
  };
  void alloc_and_init(int dsize, int size_i, int size_j, int size_k, int origin_i, int origin_j, int origin_k)
  {
    // alloc
    int total_size = dsize*(size_i*size_j*size_k) + sizeof(Ghost3Dheader);
    unsigned char* d = new unsigned char[total_size];
    header = (Ghost3Dheader*)d;
    data = d + sizeof(Ghost3Dheader);
    // init
    header->block_id = -1;
    header->face_id = -1;
    header->size[0] = size_i;
    header->size[1] = size_j;
    header->size[2] = size_k;
    header->origin[0] = origin_i;
    header->origin[1] = origin_j;
    header->origin[2] = origin_k;
  };
  Ghost3Dface()
  {
    header = 0;
    data = 0;
  };
  ~Ghost3Dface()
  {
    if (header)
    {
      data = (unsigned char*)header;
      delete [] data;
    }
  };
};

#endif
/*
===============================================================================

  FILE:  ghost3dmodule.h
  
  CONTENTS:
  
    A module for generating ghost data for structured grids in a streaming and
    parallel fashion.
  
  PROGRAMMERS:
  
    martin isenburg@cs.unc.edu
  
  COPYRIGHT:
  
    copyright (C) 2007 martin isenburg@cs.unc.edu
    
    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
  18 November 2007 - created after spotting oil-dead birds along the marina
  
===============================================================================
*/
#ifndef GHOST_3D_MODULE_H
#define GHOST_3D_MODULE_H

// #include "ghost3dblock.h"

#ifdef ENABLE_PARALLEL_MODE

extern "C" {
//#include <mpi.h>
}
#include <map>
typedef std::map<int, Ghost3Dblock*> my_hash;

#endif // ENABLE_PARALLEL_MODE

#include <deque>
typedef std::deque<Ghost3Dblock*> my_queue;

class Ghost3Dmodule
{
public:
#ifdef ENABLE_PARALLEL_MODE
  bool init(MPI_Comm comm, Ghost3Dblock* blocks, int nblocks, int dsize=1, int partition_method=1);
#else
  bool init(Ghost3Dblock* blocks, int nblocks, int dsize=1);
#endif // ENABLE_PARALLEL_MODE

  int selectBlock();
  unsigned char* processBlock(unsigned char* data_in, int* origin_out, int* size_out);

  bool close();
  
  Ghost3Dmodule(){};
  ~Ghost3Dmodule(){};

private:
  int dsize;                             // number of bytes per field entry
  int num_blocks;                        // number of blocks for this processor
  int num_processed_blocks;              // number of blocks already processed

  Ghost3Dblock* selected_block;          // the next block to be processed

  my_queue* candidate_queue[5];          // queues for block selection

  int faces_size;                        // statistics only
  int faces_maxsize;
#ifdef ENABLE_PARALLEL_MODE
  int num_boundary_faces_recv;           // number of faces we recv or send
  int num_boundary_faces_send;
  int from_proc_num;                     // processors we receive from
  int* from_proc_num_recv;
  int* from_proc_max_data;
  int* from_proc_proc_id;
  MPI_Request* from_proc_requests;       // space for receive requests
  unsigned char** from_proc_buffers;
  int send_request_alloc;                // space for send requests
  int send_request_num;
  int send_request_num_max;
  MPI_Request* send_requests;
  Ghost3Dface** send_faces;
  int rank;                              // communication stuff
  int nprocs;
  MPI_Comm communicator;

  my_hash* recv_hash;                    // hash for received faces

  void partition_blocks(Ghost3Dblock* blocks, int nblocks, int nparts, bool use_weights = false);
  void partition_blocks_simple(Ghost3Dblock* blocks, int nblocks, int nparts);
  void partition_blocks_random(Ghost3Dblock* blocks, int nblocks, int nparts);

  void add_dependency(Ghost3Dblock* block, Ghost3Dblock* dependency);

  bool receive_face(bool blocking);
#endif // ENABLE_PARALLEL_MODE

  bool allowed_for_processing(Ghost3Dblock* block);


  int copy_block(unsigned char* data, int* origin, int* size, unsigned char* data_out, int* origin_out, int* size_out);
  int copy_face(unsigned char* data, int* origin, int* size, unsigned char* data_out, int* origin_out, int* size_out);
};

#endif
