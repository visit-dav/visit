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

#include "Buffer.h"

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

PyObject *Buffer::pickleModule = NULL;
PyObject *Buffer::pickleDict  = NULL;
PyObject *Buffer::pickleDumps = NULL;
PyObject *Buffer::pickleLoads = NULL;

/*****************************************************************************
 * Function: Buffer Constructor
 *
 * Purpose: 
 *   Creates an empty mpi buffer.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

Buffer::Buffer()
: alloced(false), size(0), buffer(NULL), data(NULL)
{
    Reset();
}

/*****************************************************************************
 * Function: Buffer Constructor
 *
 * Purpose:
 *   Wraps a pointer to an existing buffer.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

Buffer::Buffer(void *buff_ptr)
: alloced(false), size(0), buffer(NULL), data(NULL)
{
    Init(buff_ptr);
}

/*****************************************************************************
 * Function: Buffer Constructor
 *
 * Purpose:
 *   Creates an mpi buffer of given total size.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

Buffer::Buffer(int buffer_size)
: alloced(false), size(0), buffer(NULL), data(NULL)
{
    Init(buffer_size);
}

/*****************************************************************************
 * Function: Buffer Constructor
 *
 * Purpose: 
 *   Creates an mpi buffer of given type and size.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

Buffer::Buffer(int type_id, int data_size)
: alloced(false), size(0), buffer(NULL), data(NULL)
{
    Init(type_id,data_size);
}

/*****************************************************************************
 * Function: Buffer Constructor
 *
 * Purpose:
 *   Creates an mpi buffer from a python object.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

Buffer::Buffer(PyObject *py_obj)
: alloced(false), size(0), buffer(NULL), data(NULL)
{
    Init(py_obj);
}


/*****************************************************************************
 * Function: Buffer Destructor
 *
 * Purpose: 
 *   Destroys an mpi buffer.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

Buffer::~Buffer()
{
    Reset();
}

/*****************************************************************************
 * Function: Buffer::Init
 *
 * Purpose:
 *   Inits buffer to given size.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

void
Buffer::Init(void *buff_ptr)
{
    if(BufferSize() > 0)
        Reset();

    alloced = false;

    buffer = buff_ptr;
    data = (void*)(((char*)buffer) + 2 * sizeof(int));

    int *header_ptr = HeaderPtr();
    // get type & data size to calc the full buffer size.
    int type_id   = header_ptr[0];
    int data_size = header_ptr[1];

    this->size = TotalBufferSize(type_id,data_size);
}


/*****************************************************************************
 * Function: Buffer::Init
 *
 * Purpose:
 *   Inits buffer to given size.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

void
Buffer::Init(int buffer_size)
{
    if(BufferSize() > 0)
        Reset();

    this->size = buffer_size;
    buffer = (void*) new char[this->size];
    int *header_ptr = HeaderPtr();
    header_ptr[0] = EMPTY;
    header_ptr[1] = 0;
    data = (void*)(((char*)buffer) + 2 * sizeof(int));
    alloced = true;
}

/*****************************************************************************
 * Function: Buffer::Init
 *
 * Purpose: 
 *   Inits buffer to given type and size.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

void
Buffer::Init(int type_id, int data_size)
{
    if(BufferSize() > 0)
        Reset();

    if(data_size == 0)
        return;

    int header_size = 2 * sizeof(int);
    this->size = TotalBufferSize(type_id,data_size);

    buffer = (void*) new char[this->size];
    int *header_ptr = HeaderPtr();
    header_ptr[0] = type_id;
    header_ptr[1] = data_size;
    data = (void*)(((char*)buffer) + header_size);
    alloced = true;
}


/*****************************************************************************
 * Function: Buffer::Init
 *
 * Purpose: 
 *   Inits buffer from a python object.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
void
Buffer::Init(PyObject *py_obj)
{
    bool pickle = false;

    if(py_obj == NULL)
        Reset();
    else if(PyInt_Check(py_obj))
    {
        Init(INTEGER,1);
        DataAsIntPtr()[0] = PyInt_AS_LONG(py_obj);
    }
    else if(PyFloat_Check(py_obj))
    {
        Init(DOUBLE,1);
        DataAsDoublePtr()[0] = PyFloat_AS_DOUBLE(py_obj);
    }
    else if(PyString_Check(py_obj))
    {
        char *py_data = PyString_AS_STRING(py_obj);
        int slen = strlen(py_data) +1;
        Init(STRING,slen);
        char *ptr = DataAsCharPtr();
        memset(ptr,0,slen*sizeof(char));
        memcpy(ptr,py_data,(slen-1)*sizeof(char));
    }
    else if(PySequence_Check(py_obj))
    {
        PyObject *py_seq  = PySequence_Fast(py_obj,"Expected Sequence");
        int length = PySequence_Size(py_seq);

        // check for list type all numeric entries 
        for(int i=0; i < length && !pickle; i++)
        {
            PyObject *py_itm = PySequence_Fast_GET_ITEM(py_seq,i); // borrowed
            if(!PyNumber_Check(py_itm))
                pickle = true;
        }

        if(!pickle)
        {
            Init(DOUBLE,length);
            double *ptr = DataAsDoublePtr();
            for(int i=0; i < length; i++)
            {
                // TODO
                PyObject *py_itm = PySequence_Fast_GET_ITEM(py_seq,i); // borrowed
                PyObject *py_val = PyNumber_Float(py_itm);
                ptr[i] = PyFloat_AS_DOUBLE(py_val);
                Py_DECREF(py_val);
            }
        }
        Py_DECREF(py_seq);
    }
    else
        pickle =true;

    if(pickle)
    {
        PyObject *res=PyObject_CallFunction(pickleDumps, (char*)"O", py_obj);
        // TODO
        Init(res);
        // Init will set the type to string - override it to identify as
        // and object.
        HeaderPtr()[0] = OBJECT;
        Py_DECREF(res);
    }

    if(this->size > 0 && HeaderPtr()[0] != EMPTY)
        alloced = true;
}


/*****************************************************************************
 * Function: Buffer::Reset
 *
 * Purpose:
 *   Cleans up buffer data.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
void
Buffer::Reset()
{
    if(buffer != NULL)
    {
        if(alloced)
        {
            char *ptr = (char*) buffer;
            delete [] ptr;
        }
        this->buffer = NULL;
        this->data   = NULL;
    }

    this->size = 0;
    this->alloced = false;
}


/*****************************************************************************
 * Function: Buffer::MPIType
 *
 * Purpose:
 *   Returns the mpi type of the buffer.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
MPI_Datatype
Buffer::MPIType()
{
    int type_id = TypeId();
    if(type_id == INTEGER)
        return MPI_INT;
    else if(type_id == DOUBLE)
        return MPI_DOUBLE;
    else if(type_id  == STRING || type_id == OBJECT)
        return MPI_CHAR;
    return NULL;
}


/*****************************************************************************
 * Function: Buffer::ToPyObject
 *
 * Purpose:
 *   Constructs a python object from the buffer data.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/
PyObject *
Buffer::ToPyObject()
{
    int type_id   = TypeId();
    int data_size = DataSize();

    if(type_id == INTEGER)
    {
        int *ptr = (int*)data;
        if(data_size == 1)
            return PyInt_FromLong(ptr[0]);
        else // return a list containing the int vals
        {
            PyObject *res = PyList_New(data_size);
            for(int i=0; i < data_size; i++)
                PyList_SET_ITEM(res,i,PyInt_FromLong(ptr[i]));
            return res;
        }
    }
    else if(type_id == DOUBLE)
    {
        double *ptr = (double*)data;
        if(data_size == 1)
            return PyFloat_FromDouble(ptr[0]);
        else // return a list containing the double vals
        {
            PyObject *res = PyList_New(data_size);
            for(int i=0; i < data_size; i++)
                PyList_SET_ITEM(res,i,PyFloat_FromDouble(ptr[i]));
            return res;
        }
    }
    else if(type_id == STRING || type_id == OBJECT)
    {
        char *ptr = (char*)data;
        PyObject *py_str = PyString_FromStringAndSize(ptr,data_size-1);
        if(type_id == STRING)
            return py_str;
        PyObject *res=PyObject_CallFunction(pickleLoads, (char*)"O", py_str);
        Py_DECREF(py_str);
        return res;
    }

    return NULL;
}

/*****************************************************************************
 * Function: PickleInit
 *
 * Purpose:
 *   Gets pointers to python's pickle functions
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Mar  4 15:33:39 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

void
Buffer::PickleInit()
{
    if(PickleReady())
        return;

    pickleModule = PyImport_ImportModule("pickle");  // new ref
    pickleDict   = PyModule_GetDict(pickleModule); // borrowed ref
    pickleDumps  = PyDict_GetItemString(pickleDict, "dumps"); // borrowed ref
    pickleLoads  = PyDict_GetItemString(pickleDict, "loads"); // borrowed ref

}

/*****************************************************************************
 * Function: PickleReady
 *
 * Purpose:
 *   Returns true if pickle functions are setup.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Thu Mar  5 08:29:22 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

bool
Buffer::PickleReady()
{
    return pickleModule != NULL;
}


/*****************************************************************************
 * Function: PickleCleanup
 *
 * Purpose:
 *   Decrements the ref count of the pickle module ref and cleans up
 *   borrowed references. 
 *
 * Note: This was created to clean up static members if necessary, in most
 *       cases these member will persist.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Thu Mar  5 08:29:22 PST 2009
 *
 * Modifications:
 *
 * ***************************************************************************/

void
Buffer::PickleCleanup()
{
    if(!PickleReady())
        return;

    Py_DECREF(pickleModule); // was true ref
    pickleModule = NULL;
    pickleDict   = NULL; // was borrowed ref
    pickleDumps  = NULL; // was borrowed ref
    pickleLoads  = NULL; // was borrowed ref

}

/*****************************************************************************
 * Function: Buffer::TotalBufferSize
 *
 * Purpose:
 *   Calculates buffer size, including header, for given data type and 
 *   data length.
 *
 * Programmer: Cyrus Harrison
 * Creation:   Fri Feb 19 08:12:50 PST 2010
 *
 * Modifications:
 *
 * ***************************************************************************/

int
Buffer::TotalBufferSize(int type_id, int data_size)
{
    int res = 0;
    int header_size = 2 * sizeof(int);

    if(type_id == INTEGER)
    {
        res = header_size +  sizeof(int) * data_size;
    }
    else if(type_id == DOUBLE)
    {
        res = header_size + sizeof(double) * data_size;
    }
    else if(type_id == STRING || type_id == OBJECT)
    {
        res = header_size + data_size;
    }

    return res;
}



