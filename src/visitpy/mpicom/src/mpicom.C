/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <Python.h>
#include <mpi.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "Buffer.h"

using namespace std;

// Make the initmpicom function callable from C.
extern "C" { void initmpicom(void); }

MPI_Comm mpi_group = MPI_COMM_WORLD;
void *mpi_comm_ptr = &mpi_group;

#define MPI_COMM_MAIN (*((MPI_Comm *)mpi_comm_ptr))


/*****************************************************************************
 * Function: mpicom_error.
 *
 * Purpose:
 *   Helper to set python error string.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
void
mpicom_error(const char *err_msg, int err_code = 0)
{
    int rank;
    ostringstream oss;
    MPI_Comm_rank(MPI_COMM_MAIN, &rank);
    if(err_code != 0)
        oss << "[" << rank << "]:error(" << err_code << "):" << err_msg;
    else
        oss << "[" << rank << "]:error:" << err_msg;

    PyErr_SetString(PyExc_RuntimeError, oss.str().c_str());
}


/*****************************************************************************
 * Function: mpicom_init
 *
 * Purpose:
 *   Inits mpi.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Mon Jan  5 11:44:24 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
static PyObject*
mpicom_init(PyObject *self, PyObject *args,PyObject *kwds)
{
    PyObject *args_tuple = NULL;
    const char  *addy_cstr = "<unknown>";
    static char *kwlist[] = {(char*)"argv", (char*)"caddy", NULL};

     if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!s", kwlist,
                                     &PyList_Type, &args_tuple, &addy_cstr))
        return NULL;

    int inited = 0;
    MPI_Initialized(&inited);

    string addy_str(addy_cstr);

    if(addy_str != "<unknown>")
    {
        if(!inited)
        {
            string emsg = "mpicom_init::";
            emsg += "MPI not initialized: Cannot use existing";
            emsg += "communicator @ address = ";
            emsg += addy_str;
            mpicom_error(emsg.c_str());
            return NULL;
        }
        // in this case we assume mpi is already initialized and we simply
        // want to set the communicator used by the mpicom module
        // (use handy python helpers to obtain addy value)

        istringstream iss(addy_str);
        unsigned long addy = 0;
        iss >> std::hex >> addy;
        mpi_comm_ptr = (void*) addy;
    }
    else if(args_tuple)
    {
        if(inited)
        {
            mpicom_error("mpicom_init::MPI already initialized");
            return NULL;
        }
        // in this case we assume  mpi is not initialized and we execute the
        // init.
        int argc = PyList_Size(args_tuple); 
        char **argv = new char*[argc+1];
        for(int i=0; i<argc; i++)
            argv[i] = PyString_AsString(PyList_GetItem(args_tuple, i));

        argv[argc] = NULL;

        int err = MPI_Init(&argc,&argv);
        delete [] argv;

        if(err != 0)
        {
            mpicom_error("mpicom_init:: Call to MPI_Init failed",err);
            return NULL;
        }
    }
    else
    {
        mpicom_error("mpicom_init::Invalid arguments to mpicom.init()");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*****************************************************************************
 * Function: mpicom_inited
 *
 * Purpose:
 *   Checks if mpi is initialized.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Mon Jan  5 11:44:24 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
static PyObject*
mpicom_inited(PyObject *self, PyObject *args)
{
    // no args function
    int inited = 0;
    MPI_Initialized(&inited);
    if(inited)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


/*****************************************************************************
 * Function: mpicom_serial
 *
 * Purpose:
 *   Returns if the module is in serial mode.
 *   The mpicom module always returns False.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Mon Jan  5 11:44:24 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
static PyObject*
mpicom_serial(PyObject *self, PyObject *args)
{
    // no args function
    Py_RETURN_FALSE;
}

/*****************************************************************************
 * Function: mpicom_parallel
 *
 * Purpose:
 *   Returns if the module is in parallel mode.
 *   The mpicom module always returns True.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Mon Jan  5 11:44:24 PST 2009
 *
 * Modifications:
 *   
 * ***************************************************************************/
static PyObject*
mpicom_parallel(PyObject *self, PyObject *args)
{
    // no args function
    Py_RETURN_TRUE;
}

/*****************************************************************************
 * Function: mpicom_rank
 *
 * Purpose: 
 *   Gets this processors rank.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_rank(PyObject *self, PyObject *args)
{
    // no args function
    int rank;
    int err = MPI_Comm_rank(MPI_COMM_MAIN,&rank);
    if(err != 0)
    {
        mpicom_error("mpicom_rank::Call to MPI_Comm_rank failed!",err);
        return NULL;
    }

    return PyInt_FromLong(rank);
}

/*****************************************************************************
 * Function: mpicom_size
 *
 * Purpose:
 *   Gets the mpi communicator's size.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_size(PyObject *self, PyObject *args)
{
    // no args function
    int com_size;
    int err = MPI_Comm_size(MPI_COMM_MAIN,&com_size);
    if(err != 0)
    {
        mpicom_error("mpicom_size::Call to MPI_Comm_size failed!",err);
        return NULL;
    }

    return PyInt_FromLong(com_size);
}

/*****************************************************************************
 * Function: mpicom_reduce
 *
 * Purpose:
 *   Min reduce.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_reduce(PyObject *self, PyObject *args, MPI_Op mpi_op)
{
    // args:
    //  value:long, double, long array, double array 
    //  des(optional): -1 = all, else specific proc rank

    PyObject *py_obj;
    int des = -1;

    if (!PyArg_ParseTuple(args, "O|i", &py_obj, &des))
        return NULL;

    int rank = 0;
    int err = MPI_Comm_rank(MPI_COMM_MAIN,&rank);
    if(err != 0)
    {
        mpicom_error("mpicom_reduce::Call to MPI_Comm_rank failed",err);
        return NULL;
    }
    // obtain type and length of array

    Buffer snd_buff;
    snd_buff.Init(py_obj);

    if(snd_buff.TypeId() != Buffer::INTEGER &&
       snd_buff.TypeId() != Buffer::DOUBLE  )
    {
        mpicom_error("mpicom_reduce::Unsupported Reduce Data Type"
                     " (expected: integer or double)");
        return NULL;
    }

    Buffer rcv_buff;

    rcv_buff.Init(snd_buff.TypeId(),snd_buff.DataSize());
    if(des != -1)
    {
        err = MPI_Reduce(snd_buff.DataPtr(),rcv_buff.DataPtr(),
                         snd_buff.DataSize(),snd_buff.MPIType(),
                         mpi_op,des,MPI_COMM_MAIN);
    }
    else
    {
        err = MPI_Allreduce(snd_buff.DataPtr(),rcv_buff.DataPtr(),
                            snd_buff.DataSize(),snd_buff.MPIType(),
                            mpi_op,MPI_COMM_MAIN);
    }

    if(err != 0)
    {
        mpicom_error("mpicom_reduce::MPI Reduce operation failed",err);
        return NULL;
    }

    // only return value on all reduce, or des == rank
    if(des != -1 && des != rank )
    {
        Py_RETURN_NONE;
    }

    return rcv_buff.ToPyObject();
}


/*****************************************************************************
 * Function: mpicom_min
 *
 * Purpose:
 *   Min reduce.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_min(PyObject *self, PyObject *args)
{
    return mpicom_reduce(self,args,MPI_MIN);
}

/*****************************************************************************
 * Function: mpicom_max
 *
 * Purpose:
 *   Max reduce.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_max(PyObject *self, PyObject *args)
{
    return mpicom_reduce(self,args,MPI_MAX);
}

/*****************************************************************************
 * Function: mpicom_sum
 *
 * Purpose:
 *   Max reduce.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_sum(PyObject *self, PyObject *args)
{
    return mpicom_reduce(self,args,MPI_SUM);
}

/*****************************************************************************
 * Function: mpicom_mult
 *
 * Purpose:
 *   Multiply reduce.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 09:50:40 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_mult(PyObject *self, PyObject *args)
{
    return mpicom_reduce(self,args,MPI_PROD);
}


/*****************************************************************************
 * Function: mpicom_barrier
 *
 * Purpose:
 *   Execute a barrier.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 14:14:15 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_barrier(PyObject *self, PyObject *args)
{
    int err = MPI_Barrier(MPI_COMM_MAIN);
    if(err != 0)
    {
        mpicom_error("mpicom_barrier::Call to MPI_Barrier failed",err);
        return NULL;
    }

    Py_RETURN_NONE;
}

/*****************************************************************************
 * Function: mpicom_broadcast
 *
 * Purpose:
 *   Broadcast a value to all other procs.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 14:14:15 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_broadcast(PyObject *self, PyObject *args)
{
    PyObject *py_obj = NULL;
    if (!PyArg_ParseTuple(args, "|O", &py_obj))
        return NULL;

    int rank = 0;
    int err = MPI_Comm_rank(MPI_COMM_MAIN,&rank);
    if(err != 0)
    {
        mpicom_error("mpicom_broadcast::Call to MPI_Comm_rank failed!",err);
        return NULL;
    }

    // if wehave the value - assume we are the source.
    int root = 0;
    if(py_obj != NULL)
        root = 1;

    Buffer snd_obj;
    Buffer rcv_obj;

    int snd_params[3] = {0,0,0};
    int rcv_params[3] = {0,0,0};

    if(root)
    {
        snd_obj.Init(py_obj);
        snd_params[0] = 1;
        snd_params[1] = rank;
        snd_params[2] = snd_obj.BufferSize();
    }

    err = MPI_Allreduce(&snd_params,&rcv_params,3,MPI_INT,
                        MPI_MAX,MPI_COMM_MAIN);
    if(err != 0)
    {
        mpicom_error("mpicom_broadcast::Error unifying broadcast params",err);
        return NULL;
    }

    int root_count = rcv_params[0];
    int root_id    = rcv_params[1];
    int msg_size   = rcv_params[2];


    if(root_count != 1)
    {
        mpicom_error("mpicom_broadcast::Invalid broadcast root.");
        return NULL;
    }

    if(root)
    {
        err = MPI_Bcast(snd_obj.BufferPtr(),snd_obj.BufferSize(),
                        MPI_CHAR,rank, MPI_COMM_MAIN);
    }
    else
    {
        rcv_obj.Init(msg_size);
        err = MPI_Bcast(rcv_obj.BufferPtr(),rcv_obj.BufferSize(),
                        MPI_CHAR,root_id, MPI_COMM_MAIN);
    }

    if(err != 0)
    {
        mpicom_error("mpicom_broadcast::MPI_Bcast Failed",err);
        return NULL;
    }

    if(root)
        Py_RETURN_NONE;
    else
        return rcv_obj.ToPyObject();
}

/*****************************************************************************
 * Function: mpicom_send
 *
 * Purpose:
 *   Sends a value to another proc.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Mar  4 16:39:49 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_send(PyObject *self, PyObject *args)
{
    PyObject *py_obj = NULL;
    int des=0;
    if (!PyArg_ParseTuple(args, "|Oi", &py_obj,&des))
        return NULL;

    Buffer snd_obj(py_obj);

    int err = MPI_Send(snd_obj.BufferPtr(),snd_obj.BufferSize(),
                       MPI_CHAR, des, 0,MPI_COMM_MAIN);
    if(err != 0)
    {
        mpicom_error("mpicom_recv::MPI_Send message failed",err);
        return NULL;
    }

    Py_RETURN_NONE;
}

/*****************************************************************************
 * Function: mpicom_recv
 *
 * Purpose:
 *   Receives a value to another proc.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Mar  4 16:39:49 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_recv(PyObject *self, PyObject *args)
{
    MPI_Status status;
    int src=0;
    if (!PyArg_ParseTuple(args, "|i", &src))
        return NULL;

    // use MPI_Probe to determine the size of the incoming buffer
    int err = MPI_Probe(src,MPI_ANY_TAG,MPI_COMM_MAIN,&status);

    if(err != 0)
    {
        mpicom_error("mpicom_recv::Probe for message failed",err);
        return NULL;
    }

    int bsize = 0;
    err = MPI_Get_count(&status,MPI_CHAR,&bsize);

    if(err != 0)
    {
        mpicom_error("mpicom_recv::Fetch of message size failed",err);
        return NULL;
    }

    Buffer rcv_obj(bsize);

    err = MPI_Recv(rcv_obj.BufferPtr(),rcv_obj.BufferSize(),
                   MPI_CHAR,src, MPI_ANY_TAG,MPI_COMM_MAIN,&status);
    if(err != 0)
    {
        mpicom_error("mpicom_recv::MPI_Recv message failed",err);
        return NULL;
    }

    return rcv_obj.ToPyObject();
}

/*****************************************************************************
 * Function: mpicom_recv
 *
 * Purpose:
 *   Receives a value to another proc.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Mar  4 16:39:49 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/

static PyObject*
mpicom_sendrecv(PyObject *self, PyObject *args)
{
    MPI_Request request;
    MPI_Status  status;
    PyObject *py_obj = NULL;
    int des=0;
    int src=0;
    if (!PyArg_ParseTuple(args, "|Oii", &py_obj,&des,&src))
        return NULL;

    // use async send to send out the message.
    Buffer snd_obj(py_obj);
    int err = MPI_Isend(snd_obj.BufferPtr(),snd_obj.BufferSize(),
                       MPI_CHAR, des, 0,MPI_COMM_MAIN,&request);
    if(err != 0)
    {
        mpicom_error("mpicom_sendrecv::MPI_ISend message failed",err);
        return NULL;
    }

    // use MPI_Probe to determine the size of the incoming buffer

    err = MPI_Probe(src,MPI_ANY_TAG,MPI_COMM_MAIN,&status);

    if(err != 0)
    {
        mpicom_error("mpicom_sendrecv::Probe for message failed",err);
        return NULL;
    }

    int bsize = 0;
    err = MPI_Get_count(&status,MPI_CHAR,&bsize);

    if(err != 0)
    {
        mpicom_error("mpicom_sendrecv::Fetch of message size failed",err);
        return NULL;
    }

    Buffer rcv_obj(bsize);

    err = MPI_Recv(rcv_obj.BufferPtr(),rcv_obj.BufferSize(),
                   MPI_CHAR,src, MPI_ANY_TAG,MPI_COMM_MAIN,&status);
    if(err != 0)
    {
        mpicom_error("mpicom_sendrecv::MPI_Recv message failed",err);
        return NULL;
    }

    return rcv_obj.ToPyObject();
}

/*****************************************************************************
 * Function: mpicom_gather
 *
 * Purpose:
 *   Implements an mpi allgather.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Thu Feb 18 19:54:12 PST 2010
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_gather(PyObject *self, PyObject *args)
{
    PyObject *py_obj = NULL;
    // msg="" & root="val"
    if (!PyArg_ParseTuple(args, "|O", &py_obj))
        return NULL;

    int nprocs;
    int err = MPI_Comm_size(MPI_COMM_MAIN,&nprocs);
    if(err != 0)
    {
        mpicom_error("mpicom_gather::Call to MPI_Comm_size failed",err);
        return NULL;
    }

    Buffer snd_buff(py_obj);
    int total_size = 0;
    int snd_count = snd_buff.BufferSize();
    vector<int> rcv_counts(nprocs);

    err = MPI_Allgather(&snd_count, 1, MPI_INT,&rcv_counts[0], 1, MPI_INT,
                        MPI_COMM_MAIN);
    if(err != 0)
    {
        mpicom_error("mpicom_gather::Error collection message sizes"
                    " (MPI_Allgather failed)",err);
        return NULL;
    }

    vector<int> rcv_displacements(nprocs);
    rcv_displacements[0] = 0;
    for(int i=0;i<nprocs;i++)
    {
        if(i > 0)
            rcv_displacements[i] = rcv_displacements[i-1] + rcv_counts[i-1];
        total_size += rcv_counts[i];
    }

    vector<char> rcv_buffer(total_size);
    err = MPI_Allgatherv(snd_buff.BufferPtr(),snd_buff.BufferSize(), MPI_CHAR,
                         &rcv_buffer[0], &rcv_counts[0],&rcv_displacements[0], MPI_CHAR,
                         MPI_COMM_MAIN);

    if(err != 0)
    {
        mpicom_error("mpicom_gather::MPI_Allgatherv Failed",err);
        return NULL;
    }

    // create result list
    PyObject *res_lst = PyList_New(nprocs);
    if(res_lst == NULL)
    {
        mpicom_error("mpicom_gather::Failed to create result list");
        return NULL;
    }

    int rcv_idx = 0;
    for(int i=0;i<nprocs;i++)
    {
        Buffer obj_buff((void*)&rcv_buffer[rcv_displacements[i]]);
        PyObject *py_obj = obj_buff.ToPyObject();
        if(py_obj  == NULL)
        {
            mpicom_error("mpicom_gather::Failed to create item for result list");
            return NULL;
        }
        if(PyList_SetItem(res_lst,i,py_obj) == -1)
        {
            mpicom_error("mpicom_gather::Failed to set item in result list");
            return NULL;
        }
    }
    return res_lst;
}

/*****************************************************************************
 * Function: mpicom_scatter
 *
 * Purpose:
 *   Scatter sequence from a root process to all other procs.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Thu Feb 18 19:54:12 PST 2010
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_scatter(PyObject *self, PyObject *args)
{
    PyObject *py_obj = NULL;
    if (!PyArg_ParseTuple(args, "|O", &py_obj))
        return NULL;

    int rank = 0;
    int err = MPI_Comm_rank(MPI_COMM_MAIN,&rank);
    if(err != 0)
    {
        mpicom_error("mpicom_scatter::Call to MPI_Comm_rank failed!",err);
        return NULL;
    }


    int nprocs;
    err = MPI_Comm_size(MPI_COMM_MAIN,&nprocs);
    if(err != 0)
    {
        mpicom_error("mpicom_scatter::Call to MPI_Comm_size failed",err);
        return NULL;
    }

    int snd_params[2] = {0,0};
    int rcv_params[2] = {0,0};

    // if we have the value - assume we are the scatter.
    if(py_obj != NULL)
    {
        snd_params[0] = 1;
        snd_params[1] = rank;
    }

    // just to be sure: unify root params with all reduce sum
    err = MPI_Allreduce(snd_params,rcv_params,2,MPI_INT,
                        MPI_SUM,MPI_COMM_MAIN);

    if(err != 0)
    {
        mpicom_error("mpicom_scatter::Error unifying scatter root",err);
        return NULL;
    }

    if(rcv_params[0] != 1)
    {
        mpicom_error("mpicom_scatter::Invalid (or multpile) scatter root(s).");
        return NULL;
    }

    int root_id = rcv_params[1];

    PyObject *res= NULL;

    if(rank == root_id)
    {
        if(PySequence_Check(py_obj) == 0)
        {
            mpicom_error("mpicom_scatter:: Scatter root must scatter a sequence");
            return NULL;
        }

        PyObject *py_seq = PySequence_Fast(py_obj,"Expected Sequence");
        if(py_seq  == NULL)
        {
            mpicom_error("mpicom_scatter:: Error obtaining sequence");
            return NULL;
        }

        vector<Buffer> buffers(nprocs);
        vector<int>    snd_counts(nprocs);
        vector<int>    snd_displacements(nprocs);
        snd_displacements[0] = 0;
        int            total_size = 0;

        int nitems = PySequence_Size(py_seq);
        for(int i = 0; i < nprocs; i++)
        {
            if(i >= nitems)
                buffers[i].Init(Py_None);
            else
            {
                PyObject *py_itm = PySequence_Fast_GET_ITEM(py_seq,i);  // borrowed
                if(py_itm == NULL)
                {
                    mpicom_error("mpicom_scatter::Error fetching scatter "
                                 "object from sequence");
                    return NULL;
                }
                buffers[i].Init(py_itm);
            }
            snd_counts[i]= buffers[i].BufferSize();
            if(i > 0)
                snd_displacements[i] = snd_displacements[i-1] + snd_counts[i-1];
            total_size +=snd_counts[i];
        }

        // send out message sizes with a scatter
        int local_count = 0;
        err = MPI_Scatter(&snd_counts[0],1,MPI_INT,
                          &local_count,1,MPI_INT,
                          root_id,MPI_COMM_MAIN);
        if(err != 0)
        {
            mpicom_error("mpicom_scatter::Error scattering message sizes",err);
            return NULL;
        }

        // prepare messages for scatterv

        vector<char> snd_buff(total_size);
        char *snd_ptr = &snd_buff[0];
        for(int i = 0; i < nprocs; i++)
        {
            memcpy(snd_ptr,buffers[i].BufferPtr(),buffers[i].BufferSize());
            snd_ptr += buffers[i].BufferSize();
        }
        buffers.clear();
        Buffer rcv_obj(local_count);
        err = MPI_Scatterv(&snd_buff[0],&snd_counts[0],&snd_displacements[0],MPI_CHAR,
                           rcv_obj.BufferPtr(),rcv_obj.BufferSize(),MPI_CHAR,
                           root_id,MPI_COMM_MAIN);
        if(err != 0)
        {
            mpicom_error("mpicom_scatter::Call to MPI_Scatterv failed",err);
            return NULL;
        }
        res = rcv_obj.ToPyObject();
    }
    else
    {
        // get local message size with a scatter
        int local_count = 0;
        err = MPI_Scatter(NULL,1,MPI_INT,
                          &local_count,1,MPI_INT,
                          root_id,MPI_COMM_MAIN);
        if(err != 0)
        {
            mpicom_error("mpicom_scatter::Error scattering message sizes",err);
            return NULL;
        }


        Buffer rcv_obj(local_count);
        err = MPI_Scatterv(NULL,NULL,NULL,MPI_CHAR,
                           rcv_obj.BufferPtr(),rcv_obj.BufferSize(),MPI_CHAR,
                           root_id,MPI_COMM_MAIN);
        if(err != 0)
        {
            mpicom_error("mpicom_scatter::Call to MPI_Scatterv failed",err);
            return NULL;
        }

        res = rcv_obj.ToPyObject();
    }

    return res;
}

/*****************************************************************************
 * Function: mpicom_alltoall
 *
 * Purpose:
 *   Send message from / to all processes.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Fri Feb 19 18:22:23 PST 2010
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_alltoall(PyObject *self, PyObject *args)
{
    PyObject *py_obj = NULL;
    if (!PyArg_ParseTuple(args, "|O", &py_obj))
        return NULL;

    int rank = 0;
    int err = MPI_Comm_rank(MPI_COMM_MAIN,&rank);
    if(err != 0)
    {
        mpicom_error("mpicom_alltoall::Call to MPI_Comm_rank failed!",err);
        return NULL;
    }

    int nprocs;
    err = MPI_Comm_size(MPI_COMM_MAIN,&nprocs);
    if(err != 0)
    {
        mpicom_error("mpicom_alltoall::Call to MPI_Comm_size failed",err);
        return NULL;
    }


    PyObject *res= NULL;

    if(PySequence_Check(py_obj) == 0)
    {
        mpicom_error("mpicom_alltoall:: Alltoall requires a sequence input.");
        return NULL;
    }

    PyObject *py_seq = PySequence_Fast(py_obj,"Expected Sequence");
    if(py_seq  == NULL)
    {
        mpicom_error("mpicom_alltoall:: Error obtaining sequence");
        return NULL;
    }

    vector<Buffer> buffers(nprocs);
    vector<int>    snd_counts(nprocs);
    vector<int>    snd_displacements(nprocs);
    snd_displacements[0] = 0;
    int            snd_total_size = 0;

    int nitems = PySequence_Size(py_seq);
    for(int i = 0; i < nprocs; i++)
    {
        if(i >= nitems)
            buffers[i].Init(Py_None);
        else
        {
            PyObject *py_itm = PySequence_Fast_GET_ITEM(py_seq,i);  // borrowed
            if(py_itm == NULL)
            {
                mpicom_error("mpicom_alltoall::Error fetching "
                             "object from sequence");
                return NULL;
            }
            buffers[i].Init(py_itm);
        }
        snd_counts[i]= buffers[i].BufferSize();
        if(i > 0)
            snd_displacements[i] = snd_displacements[i-1] + snd_counts[i-1];
        snd_total_size +=snd_counts[i];
    }

    vector<int>    rcv_counts(nprocs);

    // send out message sizes with an alltoall.
    err = MPI_Alltoall(&snd_counts[0],1,MPI_INT,
                       &rcv_counts[0],1,MPI_INT,
                       MPI_COMM_MAIN);

    if(err != 0)
    {
        mpicom_error("mpicom_alltoall::Error obtaining message sizes",err);
        return NULL;
    }

    vector<int> rcv_displacements(nprocs);
    rcv_displacements[0] = 0;
    int rcv_total_size = 0;
    for(int i=0;i<nprocs;i++)
    {
        if(i > 0)
            rcv_displacements[i] = rcv_displacements[i-1] + rcv_counts[i-1];
        rcv_total_size += rcv_counts[i];
    }

    // prepare messages for alltoallv

    vector<char> snd_buff(snd_total_size);
    char *snd_ptr = &snd_buff[0];
    for(int i = 0; i < nprocs; i++)
    {
        memcpy(snd_ptr,buffers[i].BufferPtr(),buffers[i].BufferSize());
        snd_ptr += buffers[i].BufferSize();
    }
    buffers.clear();
    
    vector<char> rcv_buff(rcv_total_size);

    err = MPI_Alltoallv(&snd_buff[0],&snd_counts[0],&snd_displacements[0],MPI_CHAR,
                       &rcv_buff[0],&rcv_counts[0],&rcv_displacements[0],MPI_CHAR,
                       MPI_COMM_MAIN);
    if(err != 0)
    {
        mpicom_error("mpicom_alltoall::Call to MPI_Alltoallv failed",err);
        return NULL;
    }
    
    // create result list
    PyObject *res_lst = PyList_New(nprocs);
    if(res_lst == NULL)
    {
        mpicom_error("mpicom_alltoall::Failed to create result list");
        return NULL;
    }

    for(int i=0;i<nprocs;i++)
    {
        Buffer obj_buff((void*)&rcv_buff[rcv_displacements[i]]);
        PyObject *py_obj = obj_buff.ToPyObject();
        if(py_obj  == NULL)
        {
            mpicom_error("mpicom_alltoall::Failed to create item for result list");
            return NULL;
        }
        if(PyList_SetItem(res_lst,i,py_obj) == -1)
        {
            mpicom_error("mpicom_alltoall::Failed to set item in result list");
            return NULL;
        }
    }

    return res_lst;
}


/*****************************************************************************
 * Function: mpicom_finalize
 *
 * Purpose:
 *   Shuts down mpi.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Mon Jan  5 11:44:24 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
static PyObject*
mpicom_finalize(PyObject *self, PyObject *args)
{
    int err = MPI_Finalize();
    if(err != 0)
    {
        mpicom_error("mpicom_finalize::MPI_Finalize failed",err);
        return NULL;
    }

    Py_RETURN_NONE;
}



/****************************************************************************
 * Method Defs
 ****************************************************************************/

static PyMethodDef mpicom_funcs[] =
{
 {"init",(PyCFunction)mpicom_init,METH_VARARGS|METH_KEYWORDS,NULL},
 {"inited",mpicom_inited,METH_NOARGS,NULL},
 {"serial",mpicom_serial,METH_NOARGS,NULL},
 {"parallel",mpicom_parallel,METH_NOARGS,NULL},
 {"rank",mpicom_rank,METH_NOARGS,NULL},
 {"size",mpicom_size,METH_NOARGS,NULL},
 {"min",mpicom_min,METH_VARARGS,NULL},
 {"max",mpicom_max,METH_VARARGS,NULL},
 {"sum",mpicom_sum,METH_VARARGS,NULL},
 {"mult",mpicom_mult,METH_VARARGS,NULL},
 {"send",mpicom_send,METH_VARARGS,NULL},
 {"recv",mpicom_recv,METH_VARARGS,NULL},
 {"sendrecv",mpicom_sendrecv,METH_VARARGS,NULL},
 {"barrier",mpicom_barrier,METH_VARARGS,NULL},
 {"broadcast",mpicom_broadcast,METH_VARARGS,NULL},
 {"gather",mpicom_gather,METH_VARARGS,NULL},
 {"scatter",mpicom_scatter,METH_VARARGS,NULL},
 {"alltoall",mpicom_alltoall,METH_VARARGS,NULL},
 {"finalize",mpicom_finalize,METH_VARARGS,NULL},
 {NULL, NULL, METH_VARARGS, NULL}
};

/*****************************************************************************
 * Function: initmpicom
 *
 * Purpose:
 *   This is the entry point to the module.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Mon Jan  5 11:44:24 PST 2009
 *
 * Modifications:
 *
 ****************************************************************************/
void
#if __GNUC__ >= 4
/* Ensure this function is visible even if -fvisibility=hidden was passed */
__attribute__ ((visibility("default")))
#endif
initmpicom(void)
{
    Buffer::PickleInit();
    Py_InitModule("mpicom", mpicom_funcs);
}


