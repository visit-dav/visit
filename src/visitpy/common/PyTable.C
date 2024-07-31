// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             PyTable.C                                     //
// ************************************************************************* //

#include <PyTable.h>

// VisIt includes
#include <DebugStream.h>
#include <QueryAttributes.h>
#include <Py2and3Support.h>
#include <PyMapNode.h>

// std includes
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

// For shared memory access
#ifndef _WIN32
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

static const char *visit_PyTable_doc =
"PyTable\n"
"\n"
"\n"
"Description:\n"
"\n"
"Wraps a table of data (2D array) stored in a contiguous buffer of memory.\n"
"Use numpy.asarray() to interact with the data as a numpy ND array.\n"
;

// ****************************************************************************
//  Function: CopyValuesFromQueryAttributes
//
//  Purpose:
//    Helper function to copy the proper results vector from the query attributes.
//    Also resets the vector inside query attributes
//
//  Programmer: Chris Laganella
//  Creation:   Fri Feb  4 11:09:29 EST 2022
//
//  Modifications:
// ****************************************************************************
static void
TakeQueryAttributesResults(QueryAttributes &qA, float *buff)
{
    // Move the vector in so that it gets destroyed at the end of the function
    std::vector<float> results = std::move(qA.GetFloatResultsValue());
    memcpy(buff, results.data(), results.size() * sizeof(float));
}

static void
TakeQueryAttributesResults(QueryAttributes &qA, double *buff)
{
    // Move the vector in so that it gets destroyed at the end of the function
    std::vector<double> results = std::move(qA.GetResultsValue());
    memcpy(buff, results.data(), results.size() * sizeof(double));
}

// ****************************************************************************
//  Struct: PyTableDataWrapper
//
//  Purpose:
//    Handles unmapping mapped memory, or freeing malloc'd memory
//
//  Programmer: Chris Laganella
//  Creation:   Fri Jan 28 16:54:33 EST 2022
//
//  Modifications:
//
//  Chris Laganella, Wed Feb  2 11:41:05 EST 2022
//  Make this handle malloc'd and mmap'd memory
// ****************************************************************************
class PyTableDataWrapper
{
public:
    PyTableDataWrapper(void *ptr, std::size_t len, bool isMemMapped)
    {
        this->ptr = ptr;
        this->len = len;
        this->isMemMapped = isMemMapped;
    }

    ~PyTableDataWrapper()
    {
        debug5 << "About to destroy PyTableDataWrapper with"
            <<  " ptr=" << ptr
            << ", len=" << len
            << ", isMemMapped=" << isMemMapped << std::endl;
        if(isMemMapped)
        {
    #ifndef _WIN32
            if(ptr)
            {
                munmap(ptr, len);
                debug5 << "Successfully unmapped " << len
                    << " bytes at " << ptr << std::endl;
            }
    #else
            debug2 << "WARNING: Should not be memory mapping"
                << " on Windows." << std::endl;
    #endif
        }
        else
        {
            free(ptr);
        }
    }

private:
    void *ptr;
    std::size_t len;
    bool isMemMapped;
};

// ****************************************************************************
//  Struct: PyTableData
//
//  Purpose:
//    The underlying data contained in a PyTableObject.
//
//  Programmer: Chris Laganella
//  Creation:   Mon Jan 17 15:11:40 EST 2022
//
//  Modifications:
//
// ****************************************************************************
struct PyTableData
{
    std::shared_ptr<PyTableDataWrapper> dataWrap; // Owns data
    void *buff;           // Pointer to table data
    Py_ssize_t len;       // The size of the buffer
    Py_ssize_t itemsize;  // The size of each element in the buffer
    Py_ssize_t shape[2];
    Py_ssize_t strides[2];
    char format[4];
};

// ****************************************************************************
//  Struct: PyTableObject
//
//  Purpose:
//    Python object used to expose a buffer of memory from C as a
//    2D array in Python.
//
//  Programmer: Chris Laganella
//  Creation:   Mon Jan 17 15:11:40 EST 2022
//
//  Modifications:
//
// ****************************************************************************
struct PyTableObject
{
    PyObject_HEAD
    PyTableData table;
};

// ****************************************************************************
//  Method: PyTableData_dealloc
//
//  Purpose:
//      Free the underlying memory owned by a PyTableData struct.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
//  Chris Laganella, Wed Feb  2 14:59:35 EST 2022
//  Call the destructor on the shared pointer, allow the PyTableDataWrapper
//  to manage the memory in both cases.
// ****************************************************************************
static void
PyTableData_dealloc(PyTableData *table)
{
    if(!table)
    {
        return;
    }

    table->dataWrap.~shared_ptr<PyTableDataWrapper>();
}

// ****************************************************************************
//  Method: PyTableObject_dealloc
//
//  Purpose:
//      To be called by Python when the reference count on a PyTableObject
//      reaches 0.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
PyTableObject_dealloc(PyTableObject *self)
{
    if(!self)
    {
        return;
    }
    PyTableData_dealloc(&self->table);
    PyObject_Del(self);
}

// ****************************************************************************
//  Method: PyTableObject_getbuffer
//
//  Purpose:
//      Used to return the underlying data in a numpy compatible form.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static int
PyTableObject_getbuffer(PyObject *obj, Py_buffer *view, int)
{
    if(!view)
    {
        PyErr_SetString(PyExc_BufferError, "NULL view in getbuffer");
        return -1;
    }

    if(!PyTable_Check(obj))
    {
        PyErr_SetString(PyExc_BufferError, "obj is not a table in getbuffer.");
        return -1;
    }

    PyTableObject *self = (PyTableObject*)obj;
    view->buf = (void*)self->table.buff;
    view->obj = obj;
    view->len = self->table.len;
    view->itemsize = self->table.itemsize;
    view->readonly = 0;
    view->ndim = 2;
    view->format = self->table.format;
    view->shape = self->table.shape;
    view->strides = self->table.strides;
    view->suboffsets = NULL;

    // Need to increase the reference count
    Py_INCREF(self);
    return 0;
}

// This definition is only compatible with Python 3.3 and above
static PyBufferProcs PyTableObject_as_buffer = {
  (getbufferproc)PyTableObject_getbuffer,
  (releasebufferproc)0,
};

// NOTE: Need to use the tp_as_buffer field so not using the VISIT_PY_TYPE_OBJ
//  macro.
static PyTypeObject PyTableObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "PyTable",                          /* tp_name */
    sizeof(PyTableObject),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)PyTableObject_dealloc,  /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash  */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    &PyTableObject_as_buffer,           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    visit_PyTable_doc,                  /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0                                   /* tp_init */
    PyVarObject_TAIL
};

// ****************************************************************************
//  Method: PyTable_Check
//
//  Purpose:
//      Check if the given PyObject is a PyTableObject.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
bool
PyTable_Check(const PyObject *obj)
{
    return (obj && (obj->ob_type == &PyTableObjectType));
}

// ****************************************************************************
//  Method: SetTypeFormat
//
//  Purpose:
//      Templated helper function to properly set the format string.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
// ****************************************************************************
template<typename T>
static void
SetTypeFormat(char str[4])
{
    // Default value assumed by Python (unsigned bytes)
    str[0] = 'B';
    str[1] = '\0';
}

template<>
void
SetTypeFormat<float>(char str[4])
{
    str[0] = 'f';
    str[1] = '\0';
}

template<>
void
SetTypeFormat<double>(char str[4])
{
    str[0] = 'd';
    str[1] = '\0';
}

// ****************************************************************************
//  Method: PyTable_CreateImpl
//
//  Purpose:
//      Construct a PyTableObject using the given data and shape.
//      The data is copied into a new buffer owned by the PyTableObject.
//      Shape is of size 2
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
//  Chris Laganella, Fri Jan 21 16:02:36 EST 2022
//  I turned this into a template function to so that a float version
//  could be supported
//
//  Chris Laganella, Fri Jan 28 17:12:35 EST 2022
//  I added the MemoryMappedDataWrapper class and use it when creating a PyTable
//
//  Chris Laganella, Wed Feb  2 14:58:39 EST 2022
//  Use PyTableDataWrapper for both cases, call placement new on the shared
//  pointer.
// ****************************************************************************
template<typename T>
static PyObject*
PyTable_CreateImpl(T *data,
                   const unsigned long *shape,
                   const std::shared_ptr<PyTableDataWrapper> &dataWrap)
{
    PyTableObject *obj = PyObject_New(PyTableObject, &PyTableObjectType);
    if(!obj)
    {
        return NULL;
    }

    PyTableData &table = obj->table;
    table.len = sizeof(T) * shape[0] * shape[1];
    new (&table.dataWrap) std::shared_ptr<PyTableDataWrapper>(dataWrap);
    table.buff = data;
    table.itemsize = sizeof(T);

    typedef typename std::remove_cv<T>::type ActualType;
    SetTypeFormat<ActualType>(table.format);

    table.shape[0] = shape[0];
    table.shape[1] = shape[1];
    table.strides[0] = sizeof(T) * shape[1];
    table.strides[1] = sizeof(T);
    return (PyObject*)obj;
}

// ****************************************************************************
// Function: LoadFromSharedMemory
//
// Purpose:
//   Memory maps the node and zone tables from the given shared memory object.
//   The node table pointer is returned through outPtrs[0] and zone table
//   through outPtrs[1].
//
// Programmer: Chris Laganella
// Creation:   Tue Jan 25 16:25:44 EST 2022
//
// ****************************************************************************
static std::shared_ptr<PyTableDataWrapper>
LoadFromSharedMemory(const std::string &shmName,
                     const std::array<bool, 2> &haveData,
                     const std::array<unsigned long, 2> &offsets,
                     const unsigned int elemSize,
                     const unsigned long buffSize,
                     std::array<void*, 2> &outPtrs)
{
    outPtrs[0] = outPtrs[1] = nullptr;
#ifndef _WIN32
    debug5 << "Loading flatten data from shared memory." << std::endl;
    int fd = shm_open(shmName.c_str(), O_RDWR, 0);
    if(fd == -1)
    {
        debug1 << "Error opening shared memory block with the name "
            << shmName << std::endl;
        return std::shared_ptr<PyTableDataWrapper>();
    }

    // mmap the shared memory block and check for errors
    void *addr = mmap(NULL, buffSize, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE, fd, 0);
    if(addr == NULL || addr == MAP_FAILED)
    {
        debug1 << "Failed to mmap the shared memory block.\n"
            << "Error string: " << strerror(errno)
            << "\nmmap(NULL, " << buffSize << ", PROT_READ | PROT_WRITE"
            << ", MAP_PRIVATE, " << fd << ", 0);" << std::endl;
        shm_unlink(shmName.c_str());
        close(fd);
        return std::shared_ptr<PyTableDataWrapper>();
    }

    // Determine the proper offsets into the shared memory block
    unsigned char *temp = (unsigned char *)addr;
    for(int i = 0; i < 2; i++)
    {
        if(!haveData[i])
        {
            continue;
        }

        outPtrs[i] = temp + (offsets[i] * elemSize);
    }

    // Always unlink / close the shared memory block
    shm_unlink(shmName.c_str());
    close(fd);
    return std::make_shared<PyTableDataWrapper>(addr, buffSize, true);
#else
    return nullptr;
#endif
}

// ****************************************************************************
// Function: LoadFlattenOutputData
//
// Purpose:
//   Helper function for creating return value for GetFlattenOutput.
//   Supports float or double results from Flatten.
//
//
// Programmer: Chris Laganella
// Creation:   Mon Jan 24 18:06:10 EST 2022
//
// Modifications
// Chris Laganella, Wed Feb  2 14:57:42 EST 2022
// Update to use PyTableDataWrapper for mmap case and non mmap case.
// ****************************************************************************
template<typename FloatType>
static PyObject *
LoadFlattenOutputData(QueryAttributes &qa,
                            const MapNode &node)
{
    // Check if this is the output of Flatten
    bool haveNodeData = node.HasEntry("nodeColumnNames");
    bool haveZoneData = node.HasEntry("zoneColumnNames");

    // Get the nodeData info
    unsigned long nodeTableShape[2] = {0, 0};
    if(haveNodeData)
    {
        if(node.HasEntry("nodeTableShape"))
        {
            auto &temp = node.GetEntry("nodeTableShape")->AsLongVector();
            nodeTableShape[0] = (temp[0] < 0) ? 0 : temp[0];
            nodeTableShape[1] = (temp[1] < 0) ? 0 : temp[1];
        }
        else
        {
            haveNodeData = false;
        }
    }

    // Get the zoneData info
    unsigned long zoneTableShape[2] = {0, 0};
    unsigned long zoneTableOffset = 0;
    if(haveZoneData)
    {
        if(node.HasEntry("zoneTableShape"))
        {
            auto &temp = node.GetEntry("zoneTableShape")->AsLongVector();
            zoneTableShape[0] = (temp[0] < 0) ? 0 : temp[0];
            zoneTableShape[1] = (temp[1] < 0) ? 0 : temp[1];
        }
        else
        {
            haveZoneData = false;
        }

        if(node.HasEntry("zoneTableOffset"))
        {
            long temp = node.GetEntry("zoneTableOffset")->AsLong();
            zoneTableOffset = (temp < 0) ? 0 : temp;
        }
    }

    unsigned long totalSize = 0;
    if(node.HasEntry("totalSize"))
    {
        long temp = node.GetEntry("totalSize")->AsLong();
        if(temp < 0)
        {
            temp = 0;
        }
        totalSize = static_cast<unsigned long>(temp);
    }

    bool useShm = false;
    std::string shmName("");
    if(node.HasEntry("useSharedMemory"))
    {
        useShm = (bool)node.GetEntry("useSharedMemory")->AsInt();
        if(useShm)
        {
            if(node.HasEntry("sharedMemoryName"))
            {
                shmName = node.GetEntry("sharedMemoryName")->AsString();
            }
            else
            {
                debug1 << "useSharedMemory set to true but no entry for"
                    << "sharedMemoryName exist. Cannot build flatten output."
                    << std::endl;
                useShm = false;
                haveNodeData = false;
                haveZoneData = false;
            }
        }
    }

    const unsigned long buffSize = totalSize * sizeof(FloatType);
    std::shared_ptr<PyTableDataWrapper> dataWrap;
    std::array<void*, 2> dataPtrs = {nullptr, nullptr};
    if(useShm)
    {
        const std::array<bool, 2> haveData = {haveNodeData, haveZoneData};
        const std::array<unsigned long, 2> offsets = {0, zoneTableOffset};
        dataWrap = LoadFromSharedMemory(shmName, haveData, offsets,
                                sizeof(FloatType), buffSize, dataPtrs);
        haveNodeData = (dataPtrs[0] != nullptr);
        haveZoneData = (dataPtrs[1] != nullptr);
    }
    else
    {
        unsigned char *buff = (unsigned char *)malloc(buffSize);
        if(!buff)
        {
            debug1 << "Could not malloc(" << buffSize << ");"
                << std::endl;
            haveNodeData = false;
            haveZoneData = false;
            dataPtrs[0] = dataPtrs[1] = nullptr;
        }
        else
        {
            dataWrap = std::make_shared<PyTableDataWrapper>(buff, buffSize, false);
            TRY
                TakeQueryAttributesResults(qa, ((FloatType*)(void*)buff));
                dataPtrs[0] = (haveNodeData) ? buff : nullptr;
                dataPtrs[1] = (haveZoneData) ? buff + (zoneTableOffset * sizeof(FloatType))
                                            : nullptr;
            CATCHALL
                debug1 << "Could not load results value from queryattributes."
                    << std::endl;
                haveNodeData = false;
                haveZoneData = false;
                dataPtrs[0] = dataPtrs[1] = nullptr;
            ENDTRY
        }
    }

    PyObject *retval = PyDict_New();
    if(haveNodeData)
    {
        FloatType *data = (FloatType*)dataPtrs[0];
        PyObject *table = PyTable_CreateImpl<FloatType>(data, nodeTableShape, dataWrap);
        PyObject *wrappedNode = PyMapNode_Wrap(*node.GetEntry("nodeColumnNames"));
        PyDict_SetItemString(retval, "nodeTable", table);
        PyDict_SetItemString(retval, "nodeColumnNames", wrappedNode);
        // Remove the references held by this function.
        Py_DecRef(table);
        Py_DecRef(wrappedNode);
    }

    if(haveZoneData)
    {
        FloatType *data = (FloatType*)dataPtrs[1];
        PyObject *table = PyTable_CreateImpl<FloatType>(data, zoneTableShape, dataWrap);
        PyObject *wrappedNode = PyMapNode_Wrap(*node.GetEntry("zoneColumnNames"));
        PyDict_SetItemString(retval, "zoneTable", table);
        PyDict_SetItemString(retval, "zoneColumnNames", wrappedNode);
        // Remove the references held by this function.
        Py_DecRef(table);
        Py_DecRef(wrappedNode);
    }

    return retval;
}

// ****************************************************************************
//  Method: PyTable_CreateFromFlattenOutput
//
//  Purpose:
//      Create the return value of visit_GetFlattenOutput.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 25 16:04:30 EST 2022
//
// ****************************************************************************
PyObject*
PyTable_CreateFromFlattenOutput(QueryAttributes &qa)
{
    const MapNode node(XMLNode(qa.GetXmlResult()));
    std::string dataType("");
    if(node.HasEntry("dataType"))
    {
        const MapNode *dt = node.GetEntry("dataType");
        if(dt->Type() == MapNode::STRING_TYPE)
        {
            dataType = dt->AsString();
        }
    }

    PyObject *retval = Py_None;
    if(dataType == "float")
    {
        retval = LoadFlattenOutputData<float>(qa, node);
    }
    else if(dataType == "double")
    {
        retval = LoadFlattenOutputData<double>(qa, node);
    }
    else
    {
        retval = PyDict_New();
    }

    return retval;
}
