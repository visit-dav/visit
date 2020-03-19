// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
