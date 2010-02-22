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

#ifndef MPICOM_BUFFER
#define MPICOM_BUFFER

#include <Python.h>
#include <mpi.h>

/*****************************************************************************
 * Class: Buffer
 *
 * Purpose:
 *   Helps with MPI Serialization / Deserialization
 *
 * Programmer: Cyrus Harrison
 * Creation:   Wed Jan  7 10:05:02 PST 2009
 *
 * Modifications:
 *   
 * ***************************************************************************/

class Buffer
{
public:
    enum 
    {
        EMPTY,
        INTEGER,
        DOUBLE,
        STRING,
        OBJECT
    };
                   Buffer();
                   Buffer(void *buff_ptr);
                   Buffer(int buff_size);
                   Buffer(int type_id, int data_size);
                   Buffer(PyObject *py_obj);
    virtual       ~Buffer();

    void           Init(void *buff_ptr);
    void           Init(int buff_size);
    void           Init(int type_id,int data_size);
    void           Init(PyObject *py_object);
    void           Reset();

    inline int     TypeId() { return  buffer ? HeaderPtr()[0] : EMPTY;}
    MPI_Datatype   MPIType();

    inline int     BufferSize() { return size;}
    inline int     DataSize()   { return  buffer ? HeaderPtr()[1] : 0;}

    inline void   *BufferPtr() { return buffer;}
    inline void   *DataPtr()   { return data;}

    inline int    *HeaderPtr() { return ((int*)buffer);}

    inline int    *DataAsIntPtr()    { return (int*)    data;}
    inline double *DataAsDoublePtr() { return (double*) data;}
    inline char   *DataAsCharPtr()   { return (char*)   data;}
    PyObject      *ToPyObject();

    static void    PickleInit();
    static bool    PickleReady();
    static void    PickleCleanup();

private:
    int            TotalBufferSize(int type_id, int data_size);
    bool           alloced;
    int            size;
    void          *buffer;
    void          *data;


    static PyObject *pickleModule;
    static PyObject *pickleDict;
    static PyObject *pickleDumps;
    static PyObject *pickleLoads;

};


#endif
